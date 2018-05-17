#ifndef SingleObjectFeatureAnalysisFilter_h_
#define SingleObjectFeatureAnalysisFilter_h_

#include <vector>
#include <sstream>

// itk
#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkTypedefs.h"

namespace ImagenomicAnalytics
{
  class SingleObjectFeatureAnalysisFilter
  {
    //--------------------------------------------------------------------------------
    // In this class, i'm taking a binary or label image, and ***TREAT
    // EVERYTHING NON-ZERO AS THE SINGLE OBJECT, and computes its
    // features.***
    //
    // Therefore it can also compute cytoplasm features as long as the
    // correct mask is given.
    //
    // This is because we have to have several features computed. So
    // dealing with one object is already much work todo. We are not
    // able to expect the magic of
    // itk::LabelImageToShapeLabelMapFilter to do the work for all
    // labels.
    //================================================================================

  public:
    typedef SingleObjectFeatureAnalysisFilter Self;

    ////////////////////////////////////////////////////////////////////////////////
    /// ctor
    SingleObjectFeatureAnalysisFilter();
    SingleObjectFeatureAnalysisFilter(std::ostream& outputStream);
    ~SingleObjectFeatureAnalysisFilter() {}
    /// ctor, end
    ////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////
    /// public fn

    ///--------------------------------------------------------------------------------
    /// Input: Note that we Assume the spacing of both m_RGBImage and
    /// m_singleObjectMask are correctly set to mpp, *BEFORE* calling
    /// this class.
    void setInputRGBImage(itkRGBImageType::Pointer inputRGBImage) {m_RGBImage = inputRGBImage;}
    void setInputMask(itkBinaryMaskImageType::Pointer inputMask);
    void setInputMask(itkLabelImageType::Pointer inputMask);

    void setTopLeft(int64_t x, int64_t y) {m_TopLeftX = x; m_TopLeftY = y;}
	void setMpp(double mpp_val) { m_mpp = mpp_val; }

    void update();
    ///================================================================================

    ///--------------------------------------------------------------------------------
    /// Output
    std::vector<FeatureValueType> getFeatures();
    std::vector<std::string> getFeatureNames();
    void outputFeaturesToFile(std::string outputFeaturesFileName);
    ///================================================================================

    /// public fn, end
    ////////////////////////////////////////////////////////////////////////////////


  private:
    std::ostream& m_outputStream;

    ////////////////////////////////////////////////////////////////////////////////
    /// private data

    ///--------------------------------------------------------------------------------
    /// Input data
    itkRGBImageType::Pointer m_RGBImage;
    int64_t m_TopLeftX; // the x of the top left cornor of this tile in the WSI, in index (pixel) space
    int64_t m_TopLeftY;
    double  m_mpp;

    int m_numberOfFeatures;
    ///================================================================================


    ///--------------------------------------------------------------------------------
    /// Computed data
    itkBinaryMaskImageType::Pointer m_singleObjectMask;
    std::vector<std::string> m_featureNames;
    std::vector<FeatureValueType> m_objectFeatures;
    ///================================================================================

    bool m_allDone;
    /// private data, end
    ////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////
    /// private fn
    void _init();
    void _computeFeatures();

    std::vector<FeatureValueType> _computeMorphologyFeatures();
    std::vector<FeatureValueType> _computeBoundaryPolygon();
    std::vector<FeatureValueType> _computeColorFeatures();


    void _someTest();
    /// private fn
    ////////////////////////////////////////////////////////////////////////////////
  };

}// namespace gth818n


#endif
