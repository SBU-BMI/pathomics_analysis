#include <vector>
#include <fstream>

// itk
#include "itkImage.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"

#include "utilityIO.h"
#include "utilityScalarImage.h"

// local
#include "SingleObjectFeatureAnalysisFilter.h"
#include "itkTypedefs.h"

namespace ImagenomicAnalytics
{
  void SingleObjectFeatureAnalysisFilter::update()
  {
    if (!ScalarImage::isImageAllZero<itkBinaryMaskImageType>(m_singleObjectMask))
      {
        //m_outputStream<<"Computing features....\n"<<std::flush;
        _computeFeatures();
        //m_outputStream<<"Computing features....done\n"<<std::flush;
      }

    m_allDone = true;

    return;
  }

  void SingleObjectFeatureAnalysisFilter::setInputMask(itkBinaryMaskImageType::Pointer inputMask)
  {
    m_singleObjectMask = itkBinaryMaskImageType::New();
    m_singleObjectMask->SetRegions(inputMask->GetLargestPossibleRegion());
    m_singleObjectMask->Allocate();

    m_singleObjectMask->CopyInformation(inputMask);

    const itkBinaryMaskImageType::PixelType* inputMaskBufferPointer = inputMask->GetBufferPointer();
    itkBinaryMaskImageType::PixelType* singleObjectMaskBufferPointer = m_singleObjectMask->GetBufferPointer();

    std::size_t np = inputMask->GetLargestPossibleRegion().GetNumberOfPixels();

    for (std::size_t it = 0; it < np; ++it)
      {
        singleObjectMaskBufferPointer[it] = inputMaskBufferPointer[it]>0?1:0;
      }

    return;
  }

  void SingleObjectFeatureAnalysisFilter::setInputMask(itkLabelImageType::Pointer inputMask)
  {
    m_singleObjectMask = itkBinaryMaskImageType::New();
    m_singleObjectMask->SetRegions(inputMask->GetLargestPossibleRegion());
    m_singleObjectMask->Allocate();

    m_singleObjectMask->CopyInformation(inputMask);

    const itkLabelImageType::PixelType* inputMaskBufferPointer = inputMask->GetBufferPointer();
    itkBinaryMaskImageType::PixelType* singleObjectMaskBufferPointer = m_singleObjectMask->GetBufferPointer();

    std::size_t np = inputMask->GetLargestPossibleRegion().GetNumberOfPixels();

    for (std::size_t it = 0; it < np; ++it)
      {
        singleObjectMaskBufferPointer[it] = inputMaskBufferPointer[it]>0?1:0;
      }

    return;
  }


  std::vector<FeatureValueType> SingleObjectFeatureAnalysisFilter::getFeatures()
  {
    if (!m_allDone)
      {
        m_outputStream<<"Error: in getFeatures, but compute not done.\n"<<std::flush;
        abort();
      }

    return m_objectFeatures;
  }


  std::vector<std::string> SingleObjectFeatureAnalysisFilter::getFeatureNames()
  {
    m_featureNames.clear();

    //--------------------------------------------------------------------------------
    // These are computed by ITK Label Shape Filter
    //m_featureNames.push_back("BoundingBoxTopLeftX");
    //m_featureNames.push_back("BoundingBoxTopLeftY");
    //m_featureNames.push_back("BoundingBoxBottomRightX");
    //m_featureNames.push_back("BoundingBoxBottomRightY");
    m_featureNames.push_back("NumberOfPixels");
    m_featureNames.push_back("PhysicalSize");
    m_featureNames.push_back("NumberOfPixelsOnBorder");
    m_featureNames.push_back("FeretDiameter");
    m_featureNames.push_back("PrincipalMoments0");
    m_featureNames.push_back("PrincipalMoments1");
    m_featureNames.push_back("Elongation");
    m_featureNames.push_back("Perimeter");
    m_featureNames.push_back("Roundness");
    m_featureNames.push_back("EquivalentSphericalRadius");
    m_featureNames.push_back("EquivalentSphericalPerimeter");
    m_featureNames.push_back("EquivalentEllipsoidDiameter0");
    m_featureNames.push_back("EquivalentEllipsoidDiameter1");
    m_featureNames.push_back("Flatness");
    //================================================================================

    m_featureNames.push_back("MeanR");
    m_featureNames.push_back("MeanG");
    m_featureNames.push_back("MeanB");
    m_featureNames.push_back("StdR");
    m_featureNames.push_back("StdG");
    m_featureNames.push_back("StdB");

    // TODO add names here if new features are added


    //--------------------------------------------------------------------------------
    // Computed by ITK iso contour, it has varying length for
    // different nucleus. So this has to be at last.
    m_featureNames.push_back("Polygon");
    //================================================================================

    return m_featureNames;
  }


  void SingleObjectFeatureAnalysisFilter::_computeFeatures()
  {
    m_objectFeatures.clear();

    //--------------------------------------------------------------------------------
    // Morphology features
    std::vector<FeatureValueType> morphologyFeatures = _computeMorphologyFeatures();
    m_objectFeatures.insert(m_objectFeatures.end(), morphologyFeatures.begin(), morphologyFeatures.end());
    //================================================================================

    //--------------------------------------------------------------------------------
    // Color features
    std::vector<FeatureValueType> colorFeatures = _computeColorFeatures();
    m_objectFeatures.insert(m_objectFeatures.end(), colorFeatures.begin(), colorFeatures.end());
    //================================================================================

    //--------------------------------------------------------------------------------
    // NOTE: add new features here, before the polygon.
    //
    // When new features are added, search for "TODO add names here if
    // new features are added" and then add names there.
    //================================================================================


    //--------------------------------------------------------------------------------
    // Finally, the polygon coordinates.
    //
    // The number of points in the polygon is varying, so this must be
    // at last. Otherwise don't know which is which.
    std::vector<double> contourPoints = _computeBoundaryPolygon();

    m_objectFeatures.insert(m_objectFeatures.end(), contourPoints.begin(), contourPoints.end());
    //================================================================================

    m_allDone = true;

    return;
  }

  std::vector<FeatureValueType>
  SingleObjectFeatureAnalysisFilter::_computeMorphologyFeatures()
  {
    std::vector<FeatureValueType> morphologyFeatures;

    typedef itkBinaryMaskImageType::PixelType LabelType;
    typedef itk::ShapeLabelObject< LabelType, ImageDimension > ShapeLabelObjectType;
    typedef itk::LabelMap< ShapeLabelObjectType > LabelMapType;

    typedef itkBinaryMaskImageType::RegionType RegionType;
    typedef itkBinaryMaskImageType::IndexType IndexType;
    typedef IndexType::IndexValueType IndexValueType;

    typedef itk::LabelImageToShapeLabelMapFilter< itkBinaryMaskImageType, LabelMapType> I2LType;
    I2LType::Pointer i2l = I2LType::New();
    i2l->SetInput( m_singleObjectMask );
    i2l->SetComputePerimeter(true);
    i2l->Update();

    LabelMapType *labelMap = i2l->GetOutput();

    if (labelMap->GetNumberOfLabelObjects() != 1)
      {
        m_outputStream<<"Error: labelMap->GetNumberOfLabelObjects() != 1\n"<<std::flush;
        abort();
      }

    ///--------------------------------------------------------------------------------
    /// morphology features

    for (unsigned int n = 0; n < labelMap->GetNumberOfLabelObjects(); ++n)
      {
        ShapeLabelObjectType *labelObject = labelMap->GetNthLabelObject(n);

        RegionType boundingBox = labelObject->GetBoundingBox();

        //std::cout<<boundingBox<<std::endl;

        IndexValueType topLeftX = boundingBox.GetIndex()[0] + m_TopLeftX; ///< wrt WSI
        IndexValueType topLeftY = boundingBox.GetIndex()[1] + m_TopLeftY; ///< wrt WSI

        unsigned long w = static_cast<unsigned long>(boundingBox.GetSize(0));
        unsigned long h = static_cast<unsigned long>(boundingBox.GetSize(1));

        IndexValueType bottomRightX = topLeftX + w;
        IndexValueType bottomRightY = topLeftY + h;

        //morphologyFeatures[featureIndex++] = labelObject->GetLabel();

        //morphologyFeatures.push_back(topLeftX);
        //morphologyFeatures.push_back(topLeftY);
        //morphologyFeatures.push_back(bottomRightX);
        //morphologyFeatures.push_back(bottomRightY);
        morphologyFeatures.push_back(labelObject->GetNumberOfPixels());
        morphologyFeatures.push_back(labelObject->GetPhysicalSize());
        morphologyFeatures.push_back(labelObject->GetNumberOfPixelsOnBorder());
        morphologyFeatures.push_back(labelObject->GetFeretDiameter());
        morphologyFeatures.push_back(labelObject->GetPrincipalMoments()[0]);
        morphologyFeatures.push_back(labelObject->GetPrincipalMoments()[1]);
        morphologyFeatures.push_back(labelObject->GetElongation());
        morphologyFeatures.push_back(labelObject->GetPerimeter());
        morphologyFeatures.push_back(labelObject->GetRoundness());
        morphologyFeatures.push_back(labelObject->GetEquivalentSphericalRadius());
        morphologyFeatures.push_back(labelObject->GetEquivalentSphericalPerimeter());
        morphologyFeatures.push_back(labelObject->GetEquivalentEllipsoidDiameter()[0]);
        morphologyFeatures.push_back(labelObject->GetEquivalentEllipsoidDiameter()[1]);
        morphologyFeatures.push_back(labelObject->GetFlatness());
      }
    //================================================================================

    return morphologyFeatures;
  }


  std::vector<FeatureValueType>
  SingleObjectFeatureAnalysisFilter::_computeBoundaryPolygon()
  {
    std::vector<double> contourPoints;

    //std::cout<<m_TopLeftX<<"\t"<<m_TopLeftY<<std::endl;

    ScalarImage::extractContour<itkBinaryMaskImageType>(m_singleObjectMask, m_TopLeftX, m_TopLeftY, contourPoints);

    return contourPoints;
  }

  std::vector<FeatureValueType>
  SingleObjectFeatureAnalysisFilter::_computeColorFeatures()
  {
    std::vector<FeatureValueType> colorFeatures;

    std::size_t nx = m_singleObjectMask->GetLargestPossibleRegion().GetSize()[0];
    std::size_t ny = m_singleObjectMask->GetLargestPossibleRegion().GetSize()[1];

    double meanR = 0;
    double meanG = 0;
    double meanB = 0;

    double stdR = 0;
    double stdG = 0;
    double stdB = 0;

    double n = 0;

    itk2DIndexType idx;
    RGBPixelType rgb;
    for (std::size_t iy = 0; iy < ny; ++iy)
      {
        idx[1] = iy;
        for (std::size_t ix = 0; ix < nx; ++ix)
          {
            idx[0] = ix;

            if (m_singleObjectMask->GetPixel(idx) != 0)
              {
                ++n;

                rgb = m_RGBImage->GetPixel(idx);

                meanR += rgb[0];
                meanG += rgb[1];
                meanB += rgb[2];

                stdR += (rgb[0]*rgb[0]);
                stdG += (rgb[1]*rgb[1]);
                stdB += (rgb[2]*rgb[2]);
              }
          }
      }

    meanR /= n;
    meanG /= n;
    meanB /= n;

    stdR /= n;
    stdG /= n;
    stdB /= n;

    stdR = sqrt(stdR - meanR*meanR);
    stdG = sqrt(stdG - meanG*meanG);
    stdB = sqrt(stdB - meanB*meanB);


    //std::vector<double> meanAndVar(6);
    colorFeatures.resize(6);
    colorFeatures[0] = meanR;
    colorFeatures[1] = meanG;
    colorFeatures[2] = meanB;
    colorFeatures[3] = stdR;
    colorFeatures[4] = stdG;
    colorFeatures[5] = stdB;

    return colorFeatures;
  }


  void SingleObjectFeatureAnalysisFilter::outputFeaturesToFile(std::string outputFeaturesFileName)
  {
    if (m_allDone == false)
      {
        m_outputStream<<"Error: feature not yet computed.\n"<<std::flush;
        abort();
      }

    std::ofstream f(outputFeaturesFileName.c_str());

    for (std::size_t it = 0; it < m_objectFeatures.size(); ++it)
      {
        f<<m_objectFeatures[it]<<",";
      }
    f<<std::endl<<std::flush;

    f.close();


    // debug
    //_someTest();
    // debug

    return;
  }


  void SingleObjectFeatureAnalysisFilter::_someTest()
  {
    //--------------------------------------------------------------------------------
    // This function is to test if the object number obtained from the ITK
    // shape filter IS that of the label image.
    //
    // They are.
    //
    // So I can compute other features and the correspondence is easy to follow.
    //================================================================================

    if (m_allDone == false)
      {
        m_outputStream<<"Error: feature not yet computed.\n"<<std::flush;
        abort();
      }

    const itkBinaryMaskImageType::PixelType* nucleiLabelImagePointer = m_singleObjectMask->GetBufferPointer();
    size_t np = m_singleObjectMask->GetLargestPossibleRegion().GetNumberOfPixels();

    std::vector<long> sizes(1+1, 0);
    for (size_t ip = 0; ip < np; ++ip)
      {
        sizes[nucleiLabelImagePointer[ip]] += 1;
      }


    std::ofstream f("sizes.txt");

    for (int64_t iNucleus = 0; iNucleus <= 1; ++iNucleus)
      {
        f<<iNucleus<<"\t"<<sizes[iNucleus]<<std::endl;
      }

    f.close();


    return;
  }



  void SingleObjectFeatureAnalysisFilter::_init()
  {
    m_RGBImage = 0;
    m_singleObjectMask = 0;

    m_TopLeftX = 0;
    m_TopLeftY = 0;

    m_numberOfFeatures = 19;

    m_allDone = false;

    return;
  }

  SingleObjectFeatureAnalysisFilter::SingleObjectFeatureAnalysisFilter() : m_outputStream(std::cout)
  {
    /// ctor
    _init();

    return;
  }

  SingleObjectFeatureAnalysisFilter::SingleObjectFeatureAnalysisFilter(std::ostream& outputStream) : m_outputStream(outputStream)
  {
    /// ctor
    _init();

    return;
  }


}// namespace gth818n
