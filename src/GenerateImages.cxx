#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"

#include <iostream>
#include <string>
#include <cmath>

// Define our standard 2D image type. 
// Using unsigned char (8-bit) as it maps perfectly to standard PNGs.
constexpr unsigned int Dimension = 2;
using PixelType = unsigned char;
using ImageType = itk::Image<PixelType, Dimension>;

void CreateCircleImage(const std::string& filename, double centerX, double centerY, double diameter)
{
    auto image = ImageType::New();

    // 1. DEFINE IMAGE GEOMETRY
    ImageType::SizeType size;
    size[0] = 300; // 300 pixels wide
    size[1] = 300; // 300 pixels high

    ImageType::IndexType start;
    start.Fill(0);

    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    image->SetRegions(region);
    image->Allocate();
    image->FillBuffer(0); // Fill with black background

    // 2. DEFINE SPACING AND ORIGIN (Crucial for registration)
    ImageType::SpacingType spacing;
    spacing.Fill(1.0); // 1 pixel = 1 mm
    image->SetSpacing(spacing);

    ImageType::PointType origin;
    origin.Fill(0.0);
    image->SetOrigin(origin);

    // 3. DRAW THE CIRCLE USING PHYSICAL POINTS
    using IteratorType = itk::ImageRegionIteratorWithIndex<ImageType>;
    IteratorType it(image, image->GetRequestedRegion());

    double radius = diameter / 2.0;
    ImageType::PointType physicalPoint;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
        // Convert the current pixel index (e.g., [150, 150]) to physical space
        image->TransformIndexToPhysicalPoint(it.GetIndex(), physicalPoint);

        // Calculate distance squared from the requested center
        double dx = physicalPoint[0] - centerX;
        double dy = physicalPoint[1] - centerY;
        double distanceSquared = (dx * dx) + (dy * dy);

        // If inside the radius, set to white (255)
        if (distanceSquared <= (radius * radius))
        {
            it.Set(255);
        }
    }

    // 4. WRITE TO PNG
    using WriterType = itk::ImageFileWriter<ImageType>;
    auto writer = WriterType::New();
    writer->SetFileName(filename);
    writer->SetInput(image);

    try
    {
        writer->Update();
        std::cout << "Successfully generated: " << filename << std::endl;
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "Error writing " << filename << ": " << err << std::endl;
    }
}

int main()
{
    std::cout << "Generating synthetic ITK images..." << std::endl;
    
    // Image 1: 30mm diameter centered at 50, 50
    CreateCircleImage("img1.png", 50.0, 50.0, 30.0);

    // Image 2: 60mm diameter centered at 200, 200
    CreateCircleImage("img2.png", 200.0, 200.0, 60.0);

    return EXIT_SUCCESS;
}