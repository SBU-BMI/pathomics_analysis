/*=========================================================================
 *
 *  Purpose: Image pre-processing.
 *  Reads a binary image file and returns a itkLabelImageType.
 *
 *=========================================================================*/

#include "BinaryMaskAnalysisFilter.h"
#include "ConvertBinaryToLabel.h"
#include "itkBinaryImageToLabelMapFilter.h"
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkTypedefs.h"
#include "SFLSLocalChanVeseSegmentor2D.h"
#include "utilityIO.h"
#include "utilityScalarImage.h"
#include "utilityTileAnalysis.h"

using namespace std;

itkLabelImageType::Pointer bin2label(string filename) {

    // Read image
    typedef itk::ImageFileReader<ImageType> ReaderType;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    reader->Update();
    ImageType::Pointer image = reader->GetOutput();

    // Convert image
    itkLabelImageType::Pointer labelImage = ImagenomicAnalytics::ScalarImage::binaryImageToConnectedComponentLabelImage<char>(image);
    size_t np = labelImage->GetLargestPossibleRegion().GetNumberOfPixels();
    cout << "np " << np << endl;

    const itkUIntImageType::PixelType *labelImageBufferPointer = labelImage->GetBufferPointer();

    itkUIntImageType::PixelType numberOfUniqueNonZeroLabels = 0;

    for (size_t ip = 0; ip < np; ++ip) {
        numberOfUniqueNonZeroLabels =
                numberOfUniqueNonZeroLabels > labelImageBufferPointer[ip] ? numberOfUniqueNonZeroLabels
                                                                          : labelImageBufferPointer[ip];
    }
    cout << "numberOfUniqueNonZeroLabels " << numberOfUniqueNonZeroLabels << endl;


#if 0

    // Creating new image.
    itkLabelImageType::Pointer newLabelImage = itkLabelImageType::New();
    newLabelImage->SetRegions(image->GetLargestPossibleRegion());
    newLabelImage->Allocate();
    newLabelImage->FillBuffer(0);

    typedef itk::BinaryImageToLabelMapFilter<ImageType> BinaryImageToLabelMapFilterType;
    BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
    binaryImageToLabelMapFilter->SetInput(image);
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

    //return newLabelImage;
    return labelImage;

}
