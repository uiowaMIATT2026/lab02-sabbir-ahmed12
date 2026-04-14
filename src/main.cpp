#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkSimilarity2DTransform.h"
#include "itkRegularStepGradientDescentOptimizerv4.h"
#include "itkMeanSquaresImageToImageMetricv4.h"
#include "itkImageRegistrationMethodv4.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"
#include "itkImageFileWriter.h"

#include <iostream>
#include <cmath>

constexpr unsigned int Dimension = 2;
using PixelType = float;
using ImageType = itk::Image<PixelType, Dimension>;


// Create a circle 
ImageType::Pointer CreateCicleImage(double cx, double cy, double radius) 
{
    ImageType::Pointer image = ImageType::New();

    ImageType:SizeType size;
    size[0] = 256;
    size[1] = 256;

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->Allocate();
    image->FillBuffer(0.0);

    ImageType::SpacingType spacing;
    spacing.Fill(1.0);
    image->SetSpacing(spacing);

    ImageType::PointType origin;
    origin.Fill(0.0);
    image->SetOrigin(origin);

    itk::ImageRegionIterator<ImageType> it(image, region);

    while (!it.IsAtEnd())
    {
        auto idx = it.GetIndex();

        double x = idx[0] * spacing[0];
        double y = idx[1] * spacing[1];

        double dist2 = (x - cx)*(x - cx) + (y - cy)*(y - cy);

        if (dist2 <= radius * radius)
        {
            it.Set(1.0);
        }

        ++it;
    }

    return image;
}

int main()
{
    // Create images
    auto fixedImage  = CreateCircleImage(50.0, 50.0, 15.0);
    auto movingImage = CreateCircleImage(200.0, 200.0, 30.0);

    // Transform 
    using TransformType = itk::Similarity2DTransform<double>;
    auto transform = TransformType::New();
    transform->SetIdentity();  // <-- key difference

    // Metric
    using MetricType = itk::MeanSquaresImageToImageMetricv4<ImageType, ImageType>;
    auto metric = MetricType::New();

    // Optimizer
    using OptimizerType = itk::RegularStepGradientDescentOptimizerv4<double>;
    auto optimizer = OptimizerType::New();

    optimizer->SetLearningRate(4.0);          
    optimizer->SetMinimumStepLength(1e-4);
    optimizer->SetNumberOfIterations(300);   

    // Interpolator
    using InterpolatorType = itk::LinearInterpolateImageFunction<ImageType, double>;
    auto interpolator = InterpolatorType::New();

    // Registration
    using RegistrationType = itk::ImageRegistrationMethodv4<ImageType, ImageType>;
    auto registration = RegistrationType::New();

    registration->SetFixedImage(fixedImage);
    registration->SetMovingImage(movingImage);
    registration->SetMetric(metric);
    registration->SetOptimizer(optimizer);
    registration->SetInitialTransform(transform);
    registration->InPlaceOn();

    // Multi-resolution setup
    RegistrationType::ShrinkFactorsArrayType shrinkFactorsPerLevel;
    shrinkFactorsPerLevel.SetSize(3);
    shrinkFactorsPerLevel[0] = 4;
    shrinkFactorsPerLevel[1] = 2;
    shrinkFactorsPerLevel[2] = 1;

    RegistrationType::SmoothingSigmasArrayType smoothingSigmasPerLevel;
    smoothingSigmasPerLevel.SetSize(3);
    smoothingSigmasPerLevel[0] = 2;
    smoothingSigmasPerLevel[1] = 1;
    smoothingSigmasPerLevel[2] = 0;

    registration->SetNumberOfLevels(3);
    registration->SetShrinkFactorsPerLevel(shrinkFactorsPerLevel);
    registration->SetSmoothingSigmasPerLevel(smoothingSigmasPerLevel);

    try
    {
        registration->Update();
    }
    catch (itk::ExceptionObject & err)
    {
        std::cerr << "Error: " << err << std::endl;
        return EXIT_FAILURE;
    }

    auto finalTransform = registration->GetTransform();

    std::cout << "Final parameters: "
              << finalTransform->GetParameters() << std::endl;

    // Resample
    using ResampleType = itk::ResampleImageFilter<ImageType, ImageType>;
    auto resampler = ResampleType::New();

    resampler->SetInput(movingImage);
    resampler->SetTransform(finalTransform);
    resampler->SetReferenceImage(fixedImage);
    resampler->UseReferenceImageOn();
    resampler->SetInterpolator(interpolator);

    resampler->Update();


    // Write images for viewing
    using WriterType = itk::ImageFileWriter<ImageType>;

    auto w1 = WriterType::New();
    w1->SetFileName("fixed.mha");
    w1->SetInput(fixedImage);
    w1->Update();

    auto w2 = WriterType::New();
    w2->SetFileName("moving.mha");
    w2->SetInput(movingImage);
    w2->Update();

    auto w3 = WriterType::New();
    w3->SetFileName("registered.mha");
    w3->SetInput(resampler->GetOutput());
    w3->Update();

    std::cout << "Done!" << std::endl;

    return EXIT_SUCCESS;
}