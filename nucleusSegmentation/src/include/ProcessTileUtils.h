#if 0

#ifndef ProcessTileUtils_h_
#define ProcessTileUtils_h_

// itk
#include "itkOpenCVImageBridge.h"
#include "itkTypedefs.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLiThresholdImageFilter.h"
#include "itkHuangThresholdImageFilter.h"
#include "itkIsoDataThresholdImageFilter.h"
#include "itkMaximumEntropyThresholdImageFilter.h"
#include "itkMomentsThresholdImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkRenyiEntropyThresholdImageFilter.h"
#include "itkShanbhagThresholdImageFilter.h"
#include "itkTriangleThresholdImageFilter.h"
#include "itkYenThresholdImageFilter.h"

#include "itkConnectedComponentImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include "itkRelabelComponentImageFilter.h"

#include "Normalization.h"
#include "HistologicalEntities.h"
#include "BinaryMaskAnalysisFilter.h"
#include "SFLSLocalChanVeseSegmentor2D.h"

#include "itkTypedefs.h"

#include "utilityScalarImage.h"
#include "utilityIO.h"
#include "utilityTileAnalysis.h"
#include "ProcessTileUtils.h"


/**
 * Process Tile
 * Instead of passing in parameter doDeclump true/false,
 * passing in an integer.
 * 0 = no declumping
 * 1 = Yi's Mean Shift
 * 2 = Jun's Watershed
 */
template<typename TNull>
itkUCharImageType::Pointer processTileDeclumpType(cv::Mat thisTileCV, \
                                           itkUShortImageType::Pointer &outputLabelImageUShort, \
                                           float otsuRatio = 1.0, \
                                           double curvatureWeight = 0.8, \
                                           float sizeThld = 3, \
                                           float sizeUpperThld = 200, \
                                           double mpp = 0.25, \
                                           float msKernel = 20.0, \
                                           int levelsetNumberOfIteration = 100, \
                                           int declumpingType = 0) {

    std::cout << "ImagenomicAnalytics::TileAnalysis::normalizeImageColor.....\n" << std::flush;
    cv::Mat newImgCV = ImagenomicAnalytics::TileAnalysis::normalizeImageColor<char>(thisTileCV);

    std::cout << "ImagenomicAnalytics::TileAnalysis::extractTissueMask.....\n" << std::flush;
    itkUCharImageType::Pointer foregroundMask = ImagenomicAnalytics::TileAnalysis::extractTissueMask<char>(
            newImgCV);

    itkRGBImageType::Pointer thisTile = itk::OpenCVImageBridge::CVMatToITKImage<itkRGBImageType>(thisTileCV);

    //IO::writeImage<itkRGBImageType>(thisTile, "thisTile.png", 0);

    std::cout << "ImagenomicAnalytics::TileAnalysis::ExtractHematoxylinChannel.....\n" << std::flush;
    itkUCharImageType::Pointer hematoxylinImage = ImagenomicAnalytics::TileAnalysis::ExtractHematoxylinChannel<char>(
            thisTile);

    short maskValue = 1;

    itkFloatImageType::Pointer hemaFloat = ImagenomicAnalytics::ScalarImage::castItkImage<itkUCharImageType, itkFloatImageType>(
            hematoxylinImage);

    std::cout << "ThresholdImage.....\n" << std::flush;

    itkUCharImageType::Pointer nucleusBinaryMask = ImagenomicAnalytics::ScalarImage::otsuThresholdImage<char>(hemaFloat,
                                                                                                              maskValue,
                                                                                                              otsuRatio);

    long numPixels = nucleusBinaryMask->GetLargestPossibleRegion().GetNumberOfPixels();

    //std::cout<<"output otsuThresholdImage.....\n"<<std::flush;
    //ImagenomicAnalytics::IO::writeImage<itkUCharImageType>(nucleusBinaryMask, "nucleusBinaryMask.png", 0);

    if (foregroundMask) {
        const itkUCharImageType::PixelType *fgMaskBufferPointer = foregroundMask->GetBufferPointer();
        itkBinaryMaskImageType::PixelType *nucleusBinaryMaskBufferPointer = nucleusBinaryMask->GetBufferPointer();

        for (long it = 0; it < numPixels; ++it) {
            if (0 == fgMaskBufferPointer[it]) {
                // for sure glass region
                nucleusBinaryMaskBufferPointer[it] = 0;
            }
        }
    }

    // SEGMENT: ChanVese
    if (!ImagenomicAnalytics::ScalarImage::isImageAllZero<itkBinaryMaskImageType>(nucleusBinaryMask)) {
        std::cout << "before CV\n" << std::flush;
        // int numiter = 100;

        CSFLSLocalChanVeseSegmentor2D<itkFloatImageType::PixelType> cv;
        cv.setImage(hemaFloat);
        cv.setMask(nucleusBinaryMask);
        cv.setNumIter(levelsetNumberOfIteration);
        cv.setCurvatureWeight(curvatureWeight);
        cv.doSegmentation();

        std::cout << "after CV\n" << std::flush;

        CSFLSLocalChanVeseSegmentor2D<itkFloatImageType::PixelType>::LSImageType::Pointer phi = cv.mp_phi;

        itkUCharImageType::PixelType *nucleusBinaryMaskBufferPointer = nucleusBinaryMask->GetBufferPointer();
        CSFLSLocalChanVeseSegmentor2D<itkFloatImageType::PixelType>::LSImageType::PixelType *phiBufferPointer = phi->GetBufferPointer();

        for (long it = 0; it < numPixels; ++it) {
            nucleusBinaryMaskBufferPointer[it] = phiBufferPointer[it] <= 1.0 ? 1 : 0;
        }
    }

    typedef itk::BinaryFillholeImageFilter<itkBinaryMaskImageType> fhFilterType;
    fhFilterType::Pointer fhfilter = fhFilterType::New();
    fhfilter->SetInput(nucleusBinaryMask);
    fhfilter->SetForegroundValue(1);
    fhfilter->Update();

    typedef itk::ConnectedComponentImageFilter<itkBinaryMaskImageType, itkLabelImageType> ConnectedComponentImageFilterType;
    ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
    connected->SetInput(nucleusBinaryMask);
    connected->Update();

    typedef itk::RelabelComponentImageFilter<itkLabelImageType, itkLabelImageType> FilterType;
    FilterType::Pointer relabelFilter = FilterType::New();
    relabelFilter->SetInput(connected->GetOutput());
    relabelFilter->SetMinimumObjectSize(static_cast<FilterType::ObjectSizeType>(sizeThld / mpp / mpp));
    relabelFilter->Update();

    itkLabelImageType::Pointer tmpImg = relabelFilter->GetOutput();

    itkUCharImageType::PixelType *nucleusBinaryMaskBufferPointer = nucleusBinaryMask->GetBufferPointer();
    itkLabelImageType::PixelType *tmpImgBufferPointer = tmpImg->GetBufferPointer();

    for (long it = 0; it < nucleusBinaryMask->GetLargestPossibleRegion().GetNumberOfPixels(); ++it) {
        nucleusBinaryMaskBufferPointer[it] = tmpImgBufferPointer[it] > 0 ? 1 : 0;
    }


    // SEGMENT: Declumping
    // Here, instead of testing boolean, we are testing an int value.
    if (declumpingType > 0) {
        if (!ImagenomicAnalytics::ScalarImage::isImageAllZero<itkBinaryMaskImageType>(nucleusBinaryMask)) {

            // WATERSHED
            if (declumpingType == 2) {

                cv::Mat watershedMask;
                cv::Mat seg = itk::OpenCVImageBridge::ITKImageToCVMat<itkUCharImageType>(nucleusBinaryMask);

                // (img, seg, mask, int minSizePl=30, int watershedConnectivity=8,
                // ::cciutils::SimpleCSVLogger *logger = NULL, ::cciutils::cv::IntermediateResultHandler *iresHandler = NULL);
                nscale::HistologicalEntities::plSeparateNuclei(newImgCV, seg, watershedMask, 30, 8, NULL, NULL);

                nucleusBinaryMask = itk::OpenCVImageBridge::CVMatToITKImage<itkUCharImageType>(watershedMask);

            }

            // MEAN SHIFT
            if (declumpingType == 1) {
                gth818n::BinaryMaskAnalysisFilter binaryMaskAnalyzer;
                binaryMaskAnalyzer.setMaskImage(nucleusBinaryMask);
                binaryMaskAnalyzer.setObjectSizeThreshold(sizeThld);
                binaryMaskAnalyzer.setObjectSizeUpperThreshold(sizeUpperThld);
                binaryMaskAnalyzer.setMeanshiftSigma(msKernel);
                binaryMaskAnalyzer.setMPP(mpp);
                // Assumes declumpingType==0
                binaryMaskAnalyzer.update();

                std::cout << "after declumping\n" << std::flush;

                itkUIntImageType::Pointer outputLabelImage = binaryMaskAnalyzer.getConnectedComponentLabelImage();
                itkUCharImageType::Pointer edgeBetweenLabelsMask = ImagenomicAnalytics::ScalarImage::edgesOfDifferentLabelRegion<char>(
                        ImagenomicAnalytics::ScalarImage::castItkImage<itkUIntImageType, itkUIntImageType>(
                                binaryMaskAnalyzer.getConnectedComponentLabelImage()));
                itkUCharImageType::PixelType *edgeBetweenLabelsMaskBufferPointer = edgeBetweenLabelsMask->GetBufferPointer();

                const itkUIntImageType::PixelType *outputLabelImageBufferPointer = outputLabelImage->GetBufferPointer();

                itkUCharImageType::PixelType *nucleusBinaryMaskBufferPointer = nucleusBinaryMask->GetBufferPointer();

                for (long it = 0; it < numPixels; ++it) {
                    nucleusBinaryMaskBufferPointer[it] = outputLabelImageBufferPointer[it] >= 1 ? 1 : 0;
                    nucleusBinaryMaskBufferPointer[it] *= (1 - edgeBetweenLabelsMaskBufferPointer[it]);
                }
            }

        }
    }


    // SEGMENT: ChanVese again, with numiter = 50.
    if (!ImagenomicAnalytics::ScalarImage::isImageAllZero<itkBinaryMaskImageType>(nucleusBinaryMask)) {
        int numiter = 50;
        CSFLSLocalChanVeseSegmentor2D<itkFloatImageType::PixelType> cv;
        cv.setImage(hemaFloat);
        cv.setMask(nucleusBinaryMask);
        cv.setNumIter(numiter);
        cv.setCurvatureWeight(curvatureWeight);
        cv.doSegmentation();

        CSFLSLocalChanVeseSegmentor2D<itkFloatImageType::PixelType>::LSImageType::Pointer phi = cv.mp_phi;

        itkUCharImageType::PixelType *nucleusBinaryMaskBufferPointer = nucleusBinaryMask->GetBufferPointer();
        CSFLSLocalChanVeseSegmentor2D<itkFloatImageType::PixelType>::LSImageType::PixelType *phiBufferPointer = phi->GetBufferPointer();

        for (long it = 0; it < numPixels; ++it) {
            nucleusBinaryMaskBufferPointer[it] = phiBufferPointer[it] <= 1.0 ? 1 : 0;
        }
    }

    // FIX hole in object
    fhFilterType::Pointer fhfilter1 = fhFilterType::New();
    fhfilter1->SetInput(nucleusBinaryMask);
    fhfilter1->SetForegroundValue(1);
    fhfilter1->Update();

    return fhfilter1->GetOutput();
}

/**
 * Process Tile
 * Passing in declumpingType parameter as integer
 * 0 = no declumping
 * 1 = Yi's Mean Shift
 * 2 = Jun's Watershed
 */
cv::Mat processTileCVDeclumpType(cv::Mat thisTileCV, \
                          float otsuRatio = 1.0, \
                          double curvatureWeight = 0.8, \
                          float sizeThld = 3, \
                          float sizeUpperThld = 200, \
                          double mpp = 0.25, \
                          float msKernel = 20.0, \
                          int levelsetNumberOfIteration = 100,
                          int declumpingType = 0) {

    itkUShortImageType::Pointer outputLabelImage;

    // call regular segmentation function
    itkUCharImageType::Pointer nucleusBinaryMask = processTileDeclumpType<char>(thisTileCV, \
                                                                       outputLabelImage, \
                                                                       otsuRatio, \
                                                                       curvatureWeight, \
                                                                       sizeThld, \
                                                                       sizeUpperThld, \
                                                                       mpp, \
                                                                       msKernel, \
                                                                       levelsetNumberOfIteration,
                                                                       declumpingType);

    // Transform from 1 to 255 does not work in our Slicer use-case.

    cv::Mat binary = itk::OpenCVImageBridge::ITKImageToCVMat<itkUCharImageType>(nucleusBinaryMask);
    return binary;

}

#endif

#endif
