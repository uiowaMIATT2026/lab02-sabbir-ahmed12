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