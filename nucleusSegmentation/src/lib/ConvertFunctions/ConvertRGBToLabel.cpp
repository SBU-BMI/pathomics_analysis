/*=========================================================================
 *
 *  Purpose: Image pre-processing.
 *  Reads an RGB image file and returns a itkLabelImageType.
 *
 *=========================================================================*/

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "ConvertRGBToLabel.h"

itkLabelImageType::Pointer rgb2label(itkRGBImageType::Pointer maskOfTile) {

    itkLabelImageType::Pointer m_objectLabelImage = itkLabelImageType::New();
    m_objectLabelImage->SetRegions(maskOfTile->GetLargestPossibleRegion());
    m_objectLabelImage->Allocate();
    m_objectLabelImage->FillBuffer(0);

    itkLabelImageType::PixelType *labelBuffer = m_objectLabelImage->GetBufferPointer();

    itk2DIndexType idx;
    long nx = maskOfTile->GetLargestPossibleRegion().GetSize(0);
    long ny = maskOfTile->GetLargestPossibleRegion().GetSize(1);

    std::size_t ii = 0;

    // Convert RGB values into single integer pixel
    for (long iy = 0; iy < ny; ++iy) {
        idx[1] = iy;
        for (long ix = 0; ix < nx; ++ix) {
            idx[0] = ix;
            itkRGBImageType::PixelType a = maskOfTile->GetPixel(idx);
            int red = (int) a.GetRed();
            int green = (int) a.GetGreen();
            int blue = (int) a.GetBlue();

            int rgb = blue;
            rgb = (rgb << 8) + green;
            rgb = (rgb << 8) + red;

            /*
            // DEBUG
            if (red > 0) {
                std::cout << "before: " << a << std::endl;
            }
            */

            //labelBuffer[ii] = (unsigned int) (blue * 256 * 256 + green * 256 + red);
            labelBuffer[ii] = (unsigned int) rgb;

            /*
            // DEBUG
            if (red > 0) {
                std::cout << "after: " << labelBuffer[ii] << std::endl;
            }
            */

            ii++;
        }
    }

    return m_objectLabelImage;
}
