#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"

int main( int argc, char *argv[])
{
  const unsigned int Dimension = 2;
  typedef unsigned char                                 PixelType;
  typedef unsigned short                                LabelType;
  typedef itk::Image<PixelType, Dimension>              InputImageType;
  typedef itk::Image< LabelType, Dimension >            OutputImageType;
  typedef itk::ShapeLabelObject< LabelType, Dimension > ShapeLabelObjectType;
  typedef itk::LabelMap< ShapeLabelObjectType >         LabelMapType;

  std::string fileName = argv[1];;
  InputImageType::Pointer image;
  typedef itk::ImageFileReader<InputImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  image = reader->GetOutput();


  typedef itk::ConnectedComponentImageFilter <InputImageType, OutputImageType > ConnectedComponentImageFilterType;
  typedef itk::LabelImageToShapeLabelMapFilter< OutputImageType, LabelMapType> I2LType;

  ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New ();
  connected->SetInput(image);
  connected->Update();

  typedef itk::LabelImageToShapeLabelMapFilter< OutputImageType, LabelMapType> I2LType;
  I2LType::Pointer i2l = I2LType::New();
  i2l->SetInput( connected->GetOutput() );
  i2l->SetComputePerimeter(true);
  i2l->Update();

  LabelMapType *labelMap = i2l->GetOutput();
  std::cout << "File " << "\"" << fileName << "\""
            << " has " << labelMap->GetNumberOfLabelObjects() << " labels." << std::endl;

  // Retrieve all attributes
  for (unsigned int n = 0; n < labelMap->GetNumberOfLabelObjects(); ++n)
    {
      ShapeLabelObjectType *labelObject = labelMap->GetNthLabelObject(n);
      std::cout << "Label: "
                << itk::NumericTraits<LabelMapType::LabelType>::PrintType(labelObject->GetLabel()) << std::endl;
      std::cout << "    BoundingBox: "
                << labelObject->GetBoundingBox() << std::endl;
      std::cout << "    NumberOfPixels: "
                << labelObject->GetNumberOfPixels() << std::endl;
      std::cout << "    PhysicalSize: "
                << labelObject->GetPhysicalSize() << std::endl;
      std::cout << "    Centroid: "
                << labelObject->GetCentroid() << std::endl;
      std::cout << "    NumberOfPixelsOnBorder: "
                << labelObject->GetNumberOfPixelsOnBorder() << std::endl;
      std::cout << "    PerimeterOnBorder: "
                << labelObject->GetPerimeterOnBorder() << std::endl;
      std::cout << "    FeretDiameter: "
                << labelObject->GetFeretDiameter() << std::endl;
      std::cout << "    PrincipalMoments: "
                << labelObject->GetPrincipalMoments() << std::endl;
      std::cout << "    PrincipalAxes: "
                << labelObject->GetPrincipalAxes() << std::endl;
      std::cout << "    Elongation: "
                << labelObject->GetElongation() << std::endl;
      std::cout << "    Perimeter: "
                << labelObject->GetPerimeter() << std::endl;
      std::cout << "    Roundness: "
                << labelObject->GetRoundness() << std::endl;
      std::cout << "    EquivalentSphericalRadius: "
                << labelObject->GetEquivalentSphericalRadius() << std::endl;
      std::cout << "    EquivalentSphericalPerimeter: "
                << labelObject->GetEquivalentSphericalPerimeter() << std::endl;
      std::cout << "    EquivalentEllipsoidDiameter: "
                << labelObject->GetEquivalentEllipsoidDiameter() << std::endl;
      std::cout << "    Flatness: "
                << labelObject->GetFlatness() << std::endl;
      std::cout << "    PerimeterOnBorderRatio: "
                << labelObject->GetPerimeterOnBorderRatio() << std::endl;
    }

  return EXIT_SUCCESS;
}
