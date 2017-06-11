/*=========================================================================
 *
 *  Type definitions.
 *
 *=========================================================================*/

// unsigned char
//typedef itk::Image<unsigned char, 2> UnsignedCharImageType;
typedef itk::Image<unsigned char, 2> ImageType;
typedef itk::RGBPixel<unsigned char> RGBPixelType;
typedef itk::Image<RGBPixelType, 2> itkRGBImageType;

// unsigned int
typedef itk::Image<unsigned int, 2> itkUIntImageType;
typedef itkUIntImageType itkLabelImageType;
//typedef itk::Image<unsigned int, 2> UnsignedIntImageType;

typedef itk::Index<2> itk2DIndexType;
