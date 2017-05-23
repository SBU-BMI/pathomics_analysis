/*=========================================================================
 *
 *  Purpose: Image pre-processing.
 *  Reads an image file and determines whether or not it's binary.
 *
 *  Uses itkStatisticsImageFilter to determine the max label.
 *  If max not equal to 255 (or 1), image is not binary.
 *  If max is equal to 255 (or 1), then check each pixel to make sure
 *  the file only contains zeros and ones.
 *
 *  ITK Example: Max/Min: itkStatisticsImageFilter
 *  https://itk.org/Wiki/ITK/Examples/Statistics/StatisticsImageFilter
 *
 *=========================================================================*/

// LOCAL
#include "IsImageBinary.h"

// ITK
#include "itkStatisticsImageFilter.h"

bool isBinary(std::string filename) {
    ImageType::Pointer image = ImageType::New();

    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    image = reader->GetOutput();

    typedef itk::StatisticsImageFilter<ImageType> StatisticsImageFilterType;
    StatisticsImageFilterType::Pointer statisticsImageFilter
            = StatisticsImageFilterType::New();
    statisticsImageFilter->SetInput(image);
    statisticsImageFilter->Update();

    std::cout << "Mean: " << statisticsImageFilter->GetMean() << std::endl;
    std::cout << "Std.: " << statisticsImageFilter->GetSigma() << std::endl;

    std::cout << "Min: " << static_cast<int>(statisticsImageFilter->GetMinimum()) << std::endl;
    int max_int = static_cast<int>(statisticsImageFilter->GetMaximum());
    std::cout << "Max: " << max_int << std::endl;

    //if (statisticsImageFilter->GetMaximum() == 255) {
    if (max_int == 255 || max_int == 1) {

        itk2DIndexType idx;
        long nx = image->GetLargestPossibleRegion().GetSize(0);
        long ny = image->GetLargestPossibleRegion().GetSize(1);

        for (long iy = 0; iy < ny; ++iy) {
            idx[1] = iy;
            for (long ix = 0; ix < nx; ++ix) {
                idx[0] = ix;
                ImageType::PixelType a = image->GetPixel(idx);
                int pix = (int) a;

                if (pix != 255 && pix != 1 && pix != 0) {
                    // Abort! Image not binary.
                    std::cout << "Caught! Image is not binary." << std::endl;
                    return 0;
                }

            }
        }

        // Image is binary.
        std::cout << "Image is binary." << std::endl;
        return 1;

    } else {
        // Image is not binary.
        std::cout << "Image is not binary." << std::endl;
        return 0;
    }
}
