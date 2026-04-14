#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkSimilarity2DTransform.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkImageRegistrationMethodv4.h"
#include "itkCenteredTransformInitializer.h"
#include "itkPNGImageIOFactory.h"
#include "itkResampleImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include <iostream>

int main(int argc, char *argv[])
{
    // REGISTER THE PNG FACTORY
    itk::PNGImageIOFactory::RegisterOneFactory();

    constexpr unsigned int Dimension = 2;
    using PixelType = float; 
    using ImageType = itk::Image<PixelType, Dimension>;
    
    using ReaderType = itk::ImageFileReader<ImageType>;
    using TransformType = itk::Similarity2DTransform<double>;
    using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
    using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
    using RegistrationType = itk::ImageRegistrationMethodv4<ImageType, ImageType>;

    // READ IMAGES
    auto fixedReader = ReaderType::New();
    auto movingReader = ReaderType::New();
    
    fixedReader->SetFileName("img2.png"); // 60mm circle
    movingReader->SetFileName("img1.png"); // 30mm circle

    try {
        fixedReader->Update();
        movingReader->Update();
    } catch (itk::ExceptionObject & err) {
        std::cerr << "Error reading images: " << err << std::endl;
        return EXIT_FAILURE;
    }

    auto fixedImage = fixedReader->GetOutput();
    auto movingImage = movingReader->GetOutput();

    // ENFORCE PHYSICAL SPACING
    ImageType::SpacingType spacing;
    spacing.Fill(1.0);
    fixedImage->SetSpacing(spacing);
    movingImage->SetSpacing(spacing);

    ImageType::PointType origin;
    origin.Fill(0.0);
    fixedImage->SetOrigin(origin);
    movingImage->SetOrigin(origin);

    // INSTANTIATE PIPELINE
    auto transform = TransformType::New();
    auto metric = MetricType::New();
    auto optimizer = OptimizerType::New();
    auto registration = RegistrationType::New();

    // INITIALIZATION (Aligns Centers of Mass)
    using InitializerType = itk::CenteredTransformInitializer<TransformType, ImageType, ImageType>;
    auto initializer = InitializerType::New();
    
    initializer->SetTransform(transform);
    initializer->SetFixedImage(fixedImage);
    initializer->SetMovingImage(movingImage);
    initializer->MomentsOn(); 
    initializer->InitializeTransform();

    // CONFIGURE OPTIMIZER
    optimizer->SetLearningRate(1.0); 
    optimizer->SetMinimumStepLength(1e-4);
    optimizer->SetNumberOfIterations(200);

    // --- THE FIX: EXPLICIT MANUAL SCALES ---
    // Similarity2D Params: [0: Scale, 1: Angle, 2: TransX, 3: TransY]
    // We scale the angle and scale parameters highly so they don't "blow up" 
    // compared to translation which operates in larger millimeter units.
    using OptimizerScalesType = OptimizerType::ScalesType;
    OptimizerScalesType optimizerScales(transform->GetNumberOfParameters());
    optimizerScales[0] = 1000.0; // Scale
    optimizerScales[1] = 1000.0; // Angle
    optimizerScales[2] = 1.0;    // Translation X
    optimizerScales[3] = 1.0;    // Translation Y
    optimizer->SetScales(optimizerScales);
    // ---------------------------------------

    registration->SetOptimizer(optimizer);
    registration->SetMetric(metric);
    registration->SetFixedImage(fixedImage);
    registration->SetMovingImage(movingImage);
    registration->SetInitialTransform(transform);
    registration->InPlaceOn(); // Modifies our transform directly

    // Multi-resolution framework (Blurs the binary edges to create a gradient)
    const unsigned int numberOfLevels = 3;
    RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel(numberOfLevels);
    shrinkFactorsPerLevel[0] = 4;
    shrinkFactorsPerLevel[1] = 2;
    shrinkFactorsPerLevel[2] = 1;

    RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel(numberOfLevels);
    smoothingSigmasPerLevel[0] = 2;
    smoothingSigmasPerLevel[1] = 1;
    smoothingSigmasPerLevel[2] = 0;

    registration->SetNumberOfLevels(numberOfLevels);
    registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);
    registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);
    registration->SmoothingSigmasAreSpecifiedInPhysicalUnitsOn();

    // EXECUTE
    try {
        std::cout << "Starting Registration..." << std::endl;
        registration->Update();
    } catch (itk::ExceptionObject & err) {
        std::cerr << "Exception caught during registration:" << std::endl;
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    // OUTPUT RESULTS
    std::cout << "Registration Complete!" << std::endl;
    std::cout << "Scale:         " << transform->GetScale() << std::endl;
    std::cout << "Translation X: " << transform->GetTranslation()[0] << std::endl;
    std::cout << "Translation Y: " << transform->GetTranslation()[1] << std::endl;
    std::cout << "Stop Condition: " << optimizer->GetStopConditionDescription() << std::endl;

    // RESAMPLE AND SAVE THE REGISTERED IMAGE
    using ResampleFilterType = itk::ResampleImageFilter<ImageType, ImageType>;
    auto resampler = ResampleFilterType::New();
    
    resampler->SetInput(movingImage);       
    resampler->SetTransform(transform);     
    resampler->UseReferenceImageOn();
    resampler->SetReferenceImage(fixedImage);
    resampler->SetDefaultPixelValue(0);     

    // B. Cast the float image to unsigned char (8-bit) for PNG saving
    using OutputPixelType = unsigned char;
    using OutputImageType = itk::Image<OutputPixelType, Dimension>;
    using CastFilterType = itk::CastImageFilter<ImageType, OutputImageType>;
    
    auto caster = CastFilterType::New();
    caster->SetInput(resampler->GetOutput());

    // C. Write the 8-bit image to disk
    using WriterType = itk::ImageFileWriter<OutputImageType>;
    auto writer = WriterType::New();
    writer->SetFileName("registered_output.png");
    writer->SetInput(caster->GetOutput());

    try {
        std::cout << "Saving registered image..." << std::endl;
        writer->Update();
        std::cout << "Successfully saved to: registered_output.png" << std::endl;
    } catch (itk::ExceptionObject & err) {
        std::cerr << "Error writing registered image: " << err << std::endl;
        return EXIT_FAILURE;
    }
}