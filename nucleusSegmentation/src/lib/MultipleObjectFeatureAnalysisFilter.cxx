#include <vector>
#include <fstream>

// itk
#include "itkImage.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"

#include "utilityIO.h"
#include "utilityScalarImage.h"

// local
#include "itkTypedefs.h"
#include "SingleObjectFeatureAnalysisFilter.h"

#include "MultipleObjectFeatureAnalysisFilter.h"


namespace ImagenomicAnalytics {
    void MultipleObjectFeatureAnalysisFilter::update() {
        //m_outputStream << "Computing features....\n" << std::flush;

        if (m_labelComputed == 0) {
            m_objectLabelImage = ScalarImage::binaryImageToConnectedComponentLabelImage<char>(m_binaryObjectMask);
        }

        _computeFeaturesForAllObjects();

        //m_outputStream << "Computing features....done\n" << std::flush;

        m_allDone = true;

        return;
    }


    void MultipleObjectFeatureAnalysisFilter::_computeFeaturesForAllObjects() {
        //--------------------------------------------------------------------------------
        // compute bounding box for each object
        std::vector<int64_t> allBoundingBoxes;

        ScalarImage::computeBoundingBoxesForAllConnectedComponents<char>(m_objectLabelImage, allBoundingBoxes);
        //================================================================================

        m_totalNumberOfConnectedComponents = allBoundingBoxes.size() / 4; // each bbox has 4 elements

        int64_t nx = static_cast<int64_t>(m_objectLabelImage->GetLargestPossibleRegion().GetSize()[0]);
        int64_t ny = static_cast<int64_t>(m_objectLabelImage->GetLargestPossibleRegion().GetSize()[1]);

        //--------------------------------------------------------------------------------
        // Expend regions by 10 pixels
        int extend = 10;
        for (itkUIntImageType::PixelType it = 0; it < m_totalNumberOfConnectedComponents; ++it) {
            int64_t x0 = allBoundingBoxes[4 * it];
            int64_t y0 = allBoundingBoxes[4 * it + 1];
            int64_t x1 = allBoundingBoxes[4 * it + 2];
            int64_t y1 = allBoundingBoxes[4 * it + 3];

            x0 = (x0 - extend) >= 0 ? (x0 - extend) : 0;
            y0 = (y0 - extend) >= 0 ? (y0 - extend) : 0;
            x1 = (x1 + extend) <= nx - 1 ? (x1 + extend) : (nx - 1);
            y1 = (y1 + extend) <= ny - 1 ? (y1 + extend) : (ny - 1);

            allBoundingBoxes[4 * it] = x0;
            allBoundingBoxes[4 * it + 1] = y0;
            allBoundingBoxes[4 * it + 2] = x1;
            allBoundingBoxes[4 * it + 3] = y1;
        }

        //std::cout << "after extending\n" << allBoundingBoxes[0] << ", " << allBoundingBoxes[1] << ", " << allBoundingBoxes[2] << ", " << allBoundingBoxes[3] << "\n";
        //================================================================================


        //--------------------------------------------------------------------------------
        // crop each object and compute features
        m_featuresOfAllObjects.resize(m_totalNumberOfConnectedComponents);

        itkUIntImageType::IndexType start;
        start.Fill(0);

        itkUIntImageType::SizeType ROISize;

        itkUIntImageType::RegionType region;
        region.SetIndex(start);

        itkUIntImageType::IndexType idx;
        itkUIntImageType::IndexType idxSmall;


        for (int64_t it = 0; it < m_totalNumberOfConnectedComponents; ++it)
            //for (int64_t it = 0; it < 1; ++it)
        {
            itkLabelImageType::PixelType label =
                    it + 1;//The assumption is that in the label image we have consecutive labels from 1 to n_of_objects

            //----------------------------------------------------------------------
            // crop each object
            int64_t x0 = allBoundingBoxes[4 * it];
            int64_t y0 = allBoundingBoxes[4 * it + 1];
            int64_t x1 = allBoundingBoxes[4 * it + 2];
            int64_t y1 = allBoundingBoxes[4 * it + 3];

            //std::cout << it << "   (" << x0 << ", " << y0 << "), (" << x1 << ", " << y1 << ")\n";

            ROISize[0] = static_cast<itkUIntImageType::SizeValueType>(x1 - x0) + 1;
            ROISize[1] = static_cast<itkUIntImageType::SizeValueType>(y1 - y0) + 1;

            region.SetSize(ROISize);

            //std::cout << region << std::endl;

            itkBinaryMaskImageType::Pointer thisObjectMask = itkBinaryMaskImageType::New();
            thisObjectMask->SetRegions(region);
            thisObjectMask->Allocate();
            thisObjectMask->FillBuffer(0);

            itkRGBImageType::Pointer thisObjectRGB = itkRGBImageType::New();
            thisObjectRGB->SetRegions(region);
            thisObjectRGB->Allocate();

            for (int64_t iy = y0; iy <= y1; ++iy) {
                idx[1] = iy;
                idxSmall[1] = iy - y0;

                for (int64_t ix = x0; ix <= x1; ++ix) {
                    idx[0] = ix;
                    idxSmall[0] = ix - x0;

                    if (m_objectLabelImage->GetPixel(idx) == it + 1) {
                        thisObjectMask->SetPixel(idxSmall, 1);
                    }

                    thisObjectRGB->SetPixel(idxSmall, m_RGBImage->GetPixel(idx));
                }
            }

            // dbg
            // writeImage<itkBinaryMaskImageType>(thisObjectMask, "thisObjectMask.png");
            // writeImage<itkRGBImageType>(thisObjectRGB, "thisObjectRGB.png");
            // dbg, end

            //----------------------------------------------------------------------
            // Compute feature of this object
            SingleObjectFeatureAnalysisFilter featureAnalyzer;
            featureAnalyzer.setInputRGBImage(thisObjectRGB);
            featureAnalyzer.setInputMask(thisObjectMask);
            featureAnalyzer.setTopLeft(m_TopLeftX + x0, m_TopLeftY + y0);
            featureAnalyzer.update();

            m_featuresOfAllObjects[it] = featureAnalyzer.getFeatures();
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        // std::cout << m_featuresOfAllObjects.size() << std::endl;
        // std::cout << m_featuresOfAllObjects[0].size() << std::endl;

        m_allDone = true;

        return;
    }

    std::vector<std::vector<FeatureValueType> >
    MultipleObjectFeatureAnalysisFilter::getFeatures() {
        if (m_allDone == false) {
            m_outputStream << "Error: feature not yet computed.\n" << std::flush;
            abort();
        }

        return m_featuresOfAllObjects;
    }

    void MultipleObjectFeatureAnalysisFilter::outputFeaturesToConsole() {
        if (m_allDone == false) {
            m_outputStream << "Error: feature not yet computed.\n" << std::flush;
            abort();
        }

        for (int64_t iObject = 0; iObject < m_totalNumberOfConnectedComponents; ++iObject) {
            for (std::size_t it = 0; it < m_featuresOfAllObjects[iObject].size(); ++it) {
                std::cout << m_featuresOfAllObjects[iObject][it] << ",";
            }
            std::cout << std::endl << std::flush;
        }

        return;
    }

    void MultipleObjectFeatureAnalysisFilter::outputFeaturesToFile(std::string outputFeaturesFileName) {
        if (m_allDone == false) {
            m_outputStream << "Error: feature not yet computed.\n" << std::flush;
            abort();
        }

        SingleObjectFeatureAnalysisFilter featureAnalyzer;
        m_featureNames = featureAnalyzer.getFeatureNames();

        std::ofstream f(outputFeaturesFileName.c_str());
        int nf = m_featureNames.size();
        for (int it = 0; it < nf; ++it) {
            f << m_featureNames[it] << ",";
        }
        f << std::endl << std::flush;

        for (int64_t iObject = 0; iObject < m_totalNumberOfConnectedComponents; ++iObject) {
            for (std::size_t it = 0; it < m_featuresOfAllObjects[iObject].size(); ++it) {
                f << m_featuresOfAllObjects[iObject][it] << ",";
            }
            f << std::endl << std::flush;
        }

        f.close();

        return;
    }


    void MultipleObjectFeatureAnalysisFilter::_init() {
        m_RGBImage = 0;
        m_binaryObjectMask = 0;
        m_objectLabelImage = 0;
        m_labelComputed = 0;

        m_TopLeftX = 0;
        m_TopLeftY = 0;

        m_allDone = false;

        return;
    }

    MultipleObjectFeatureAnalysisFilter::MultipleObjectFeatureAnalysisFilter() : m_outputStream(std::cout) {
        /// ctor
        _init();

        return;
    }

    MultipleObjectFeatureAnalysisFilter::MultipleObjectFeatureAnalysisFilter(std::ostream &outputStream)
            : m_outputStream(outputStream) {
        /// ctor
        _init();

        return;
    }


}// namespace gth818n
