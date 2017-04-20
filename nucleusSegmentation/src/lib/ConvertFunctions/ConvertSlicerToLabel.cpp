/*=========================================================================
 *
 *  Purpose: Image pre-processing.
 *  Reads a 16-bit binary inputImage file and returns a itkLabelImageType.
 *
 *=========================================================================*/

// LOCAL
#include "itkImage.h"
#include "ConvertSlicerToLabel.h"

#define MYTEST
#include "utilityTileAnalysis.h"

// ITK
#include "itkTypedefs.h"
#include "BinaryMaskAnalysisFilter.h"

using namespace std;

itkLabelImageType::Pointer slicer2label(string filename) {

    typedef itk::Image<short, 2> ShortImageType;

    typedef itk::ImageFileReader<ShortImageType> ReaderType;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    reader->Update();

    ShortImageType::Pointer inputImage = reader->GetOutput();

    ImageType::Pointer outputImage = ImageType::New();
    outputImage->SetRegions(inputImage->GetLargestPossibleRegion());
    outputImage->Allocate();
    outputImage->FillBuffer(0);

    int cols = inputImage->GetLargestPossibleRegion().GetSize()[0]; // width
    int rows = inputImage->GetLargestPossibleRegion().GetSize()[1]; // height

    const int nx = cols;
    const int ny = rows;
    try {
        ImageType::IndexType index;
        for (int x = 0; x < nx; x++) // for all Columns
        {
            index[0] = x;
            for (int y = 0; y < ny; y++) // for all Rows
            {
                index[1] = y;

                unsigned char result = (unsigned char) inputImage->GetPixel(index);

                outputImage->SetPixel(index, result);

                //if (result > 0) {
                //cout << index[0] << " " << index[1] << " " << (int) result << endl;
                //cout << "pixel value " << (int) outputImage->GetPixel(index) << endl;
                //}
            }
        }
    }
    catch (int e) {
        cout << "An exception occurred. Exception Nr. " << e << '\n';
    }

    // Creating new image, from the outputImage.
    itkLabelImageType::Pointer labelImage = ImagenomicAnalytics::ScalarImage::binaryImageToConnectedComponentLabelImage<char>(
            outputImage);
    size_t np = labelImage->GetLargestPossibleRegion().GetNumberOfPixels();
    //cout << "np " << np << endl;

    const itkUIntImageType::PixelType *labelImageBufferPointer = labelImage->GetBufferPointer();

    itkUIntImageType::PixelType numberOfUniqueNonZeroLabels = 0;

    for (size_t ip = 0; ip < np; ++ip) {
        numberOfUniqueNonZeroLabels =
                numberOfUniqueNonZeroLabels > labelImageBufferPointer[ip] ? numberOfUniqueNonZeroLabels
                                                                          : labelImageBufferPointer[ip];
    }
    cout << "numberOfUniqueNonZeroLabels " << numberOfUniqueNonZeroLabels << endl;

#if 0

    itkLabelImageType::Pointer newLabelImage = itkLabelImageType::New();
    newLabelImage->SetRegions(outputImage->GetLargestPossibleRegion());
    newLabelImage->Allocate();
    newLabelImage->FillBuffer(0);

    typedef itk::BinaryImageToLabelMapFilter<ImageType> BinaryImageToLabelMapFilterType;
    BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
    binaryImageToLabelMapFilter->SetInput(outputImage);
    binaryImageToLabelMapFilter->Update();

    // The output of this filter is an itk::LabelMap, which contains itk::LabelObject's
    cout << "There are " << binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << " objects."
              << endl;

    // Loop over each region
    ImageType::IndexType pixelIndex;
    for (unsigned int i = 0; i < binaryImageToLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++) {
        // Get the ith region
        BinaryImageToLabelMapFilterType::OutputImageType::LabelObjectType *labelObject = binaryImageToLabelMapFilter->GetOutput()->GetNthLabelObject(
                i);

        // Output the pixels composing the region
        for (unsigned int pixelId = 0; pixelId < labelObject->Size(); pixelId++) {

            //cout << "Object " << i << " contains pixel " << labelObject->GetIndex(pixelId) << endl;
            pixelIndex = labelObject->GetIndex(pixelId);
            newLabelImage->SetPixel(pixelIndex, i);

            //cout << "m_objectLabelImage " << newLabelImage->GetPixel(pixelIndex) << endl;

        }
    }
#endif

    // return newLabelImage;
    return labelImage;
}
