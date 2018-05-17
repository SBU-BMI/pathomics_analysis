/*=========================================================================
 *
 *  Purpose: Image pre-processing.
 *  Reads an image file, retrieves information about it,
 *  and returns a struct containing:
 *  filename
 *  binary (true/false)
 *  componentSize (1 = 8-bit)
 *  pixelType (1 = binary or labeled; 2 = RGB)
 *  valid (was file-read successful)
 *
 *  ITK Example: IO ReadUnknownImageType
 *  https://itk.org/Wiki/ITK/Examples/IO/ReadUnknownImageType
 *
 *=========================================================================*/

// LOCAL
#include "ReadUnknownImageType.h"

// ITK
#include "itkBinaryImageToLabelMapFilter.h"

imageInfo getImageInfo(std::string inputFilename) {

    imageInfo newImageInfo;

    newImageInfo.filename = inputFilename;

    typedef itk::ImageIOBase::IOComponentType ScalarPixelType;

    itk::ImageIOBase::Pointer imageIO =
            itk::ImageIOFactory::CreateImageIO(
                    inputFilename.c_str(), itk::ImageIOFactory::ReadMode);
    if (!imageIO) {
        newImageInfo.valid = false;
        std::cerr << "Could not CreateImageIO for: " << inputFilename << std::endl;
        return newImageInfo;
    }

    newImageInfo.valid = true;
    imageIO->SetFileName(inputFilename);
    imageIO->ReadImageInformation();

    newImageInfo.pixelType = imageIO->GetPixelType();
    newImageInfo.componentSize = imageIO->GetComponentSize();

    const ScalarPixelType componentType = imageIO->GetComponentType();
    std::cout << "Pixel Type is " << imageIO->GetComponentTypeAsString(componentType) // 'unsigned_char'
              << std::endl;
    const size_t numDimensions = imageIO->GetNumberOfDimensions();
    std::cout << "numDimensions: " << numDimensions << std::endl; // '2'

    std::cout << "component size: " << imageIO->GetComponentSize() << std::endl; // '1'
    std::cout << "pixel type (string): " << imageIO->GetPixelTypeAsString(imageIO->GetPixelType())
              << std::endl; // 'rgb'
    std::cout << "pixel type: " << imageIO->GetPixelType() << std::endl; // '2'

    return newImageInfo;
}
