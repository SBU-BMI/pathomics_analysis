#include <cstdio>
#include <iostream>
#include <string>
#include <omp.h>


//itk
#include "itkImage.h"
#include "itkRGBPixel.h"
#include "itkOpenCVImageBridge.h"

// openslide
#include "openslide.h"

// openCV
#include <opencv2/opencv.hpp>


//#include "Normalization.h"

#include "itkTypedefs.h"

#include "BinaryMaskAnalysisFilter.h"

#include "SFLSLocalChanVeseSegmentor2D.h"


// u24
//#include "MaskToPoly.hpp"

#include "utilityIO.h"
#include "utilityTileAnalysis.h"
#include "utilityScalarImage.h"
#include "utilityWholeSlideProcessing.h"



int main(int argc, char **argv)
{
  if (argc < 4)
    {
      std::cerr<<"Parameters: imageName outputPrefix tileSize [otsuRatio = 1.0] [curvatureWeight = 0.8] [sizeThld = 3] [sizeUpperThld = 200] [msKernel = 20.0] [declumpingType = 0]\n";
      exit(-1);
    }

  const int ImageDimension = 2;

  const char* fileName = argv[1];
  std::string outputPrefix(argv[2]);
  int64_t tileSize = strtoll(argv[3], NULL, 10);

  /*--------------------------------------------------------------------------------
    When thresholding the hematoxylin channel, an optimization process
    identify an "optimal" threshold. However, we can adjust it by
    multiplying with this ratio. So, if we want to get fewer nuclei,
    we lower this otsuRatio; and vice versa.

    range: > 0
    --------------------------------------------------------------------------------*/
  float otsuRatio = 1.0;
  if (argc > 4)
    {
      otsuRatio = atof(argv[4]);
    }

  /*--------------------------------------------------------------------------------
    Higher value will cause smoother nuclear boundary.

    range: [0, 1]
    --------------------------------------------------------------------------------*/
  double curvatureWeight = 0.8;
  if (argc > 5)
    {
      curvatureWeight = atof(argv[5]);
    }

  /*--------------------------------------------------------------------------------
    nucleus smaller than this value will be regarded as noise and
    removed.

    range > 0
    --------------------------------------------------------------------------------*/
  float sizeThld = 3;
  if (argc > 6)
    {
      sizeThld = atof(argv[6]);
    }

  /*--------------------------------------------------------------------------------
    region larger than this value will be regarded as clump and
    de-clumped.

    range > 0
    --------------------------------------------------------------------------------*/
  float sizeUpperThld = 200;
  if (argc > 7)
    {
      sizeUpperThld = atof(argv[7]);
    }

  /*--------------------------------------------------------------------------------
    Declumping parameter. Smaller value results in smaller islands
    during de-clumping
    --------------------------------------------------------------------------------*/
  float msKernel = 20.0;
  if (argc > 8)
    {
      msKernel = atof(argv[8]);
    }

  int declumpingType;
  if (argc > 9)
    {
      int tmp = atoi(argv[9]);
      declumpingType = tmp;
    }




  //--------------------------------------------------------------------------------
  // Init output feature file
  std::string outputFeatureName = outputPrefix + ".feature";
  std::ofstream outputFeatureFile(outputFeatureName.c_str());
  outputFeatureFile << "PolygonNo\tX\tY\tArea\tBoundaries" << endl;
  //================================================================================

  openslide_t *osr = openslide_open(fileName);

  //int magnification = ImagenomicAnalytics::WholeSlideProcessing::extractMagnification<char>(osr);
  float mpp = ImagenomicAnalytics::WholeSlideProcessing::extractMPP<char>(osr);

  int32_t levelOfLargestSize = 0; // 0-th level is the largest

  int64_t largestW;
  int64_t largestH;

  {
    int64_t w[1];
    int64_t h[1];

    openslide_get_level_dimensions(osr, levelOfLargestSize, w, h);

    largestW = w[0];
    largestH = h[0];
  }

  std::vector<int64_t> tileTopleftX;
  std::vector<int64_t> tileTopleftY;
  std::vector<int64_t> tileSizeX;
  std::vector<int64_t> tileSizeY;

  ImagenomicAnalytics::WholeSlideProcessing::generateTileRegions<char>(largestW, largestH, tileSize, \
                                                                       tileTopleftX, tileTopleftY, tileSizeX, tileSizeY);
  std::size_t numberOfTiles = tileSizeY.size();

  std::cout<<"numberOfTiles = "<<numberOfTiles<<std::endl<<std::flush;


#pragma omp parallel for
  for (std::size_t iTile = 0; iTile < numberOfTiles; ++iTile)
    {
      int64_t topLeftX = tileTopleftX[iTile];
      int64_t topLeftY = tileTopleftY[iTile];
      int64_t sizeX = tileSizeX[iTile];
      int64_t sizeY = tileSizeY[iTile];

#pragma omp critical
      {
        std::cout<<"top = ("<<topLeftX<<", "<<topLeftY<<")    size = ("<<sizeX<<", "<<sizeY<<")\n"<<std::flush;
      }

      cv::Mat thisTile;
#pragma omp critical
      {
        thisTile = ImagenomicAnalytics::WholeSlideProcessing::extractTileFromWSI<char>(osr, levelOfLargestSize, topLeftX, topLeftY, sizeX, sizeY);
      }

      itkRGBImageType::Pointer thisTileItk =  itk::OpenCVImageBridge::CVMatToITKImage< itkRGBImageType >( thisTile );

#pragma omp critical
      {
        char outputTileName[1000];
        sprintf(outputTileName, "%s_mpp_%g_x%ld_y%ld-tile.jpg", outputPrefix.c_str(), mpp, topLeftX, topLeftY);
        ImagenomicAnalytics::IO::writeImage<itkRGBImageType>(thisTileItk, outputTileName, 0);
      }

      itkUShortImageType::Pointer outputLabelImageUShort = itkUShortImageType::New();
      itkUCharImageType::Pointer nucleusBinaryMask = ImagenomicAnalytics::TileAnalysis::processTile<char>(thisTile, outputLabelImageUShort, \
                                                                                                          otsuRatio, curvatureWeight, sizeThld, sizeUpperThld, \
                                                                                                          mpp, msKernel, declumpingType);

      // cv::Mat binary = itk::OpenCVImageBridge::ITKImageToCVMat< itkUCharImageType >( nucleusBinaryMask  );
      // cv::Mat outputLabelImageMat = itk::OpenCVImageBridge::ITKImageToCVMat< itkUShortImageType >( outputLabelImageUShort  );

      // #pragma omp critical
      //       {
      //         std::cout<<"done with processing\n"<<std::flush;
      //       }

#pragma omp critical
      {
        char outputLabelName[1000];
        sprintf(outputLabelName, "%s_mpp_%g_x%ld_y%ld-seg.png", outputPrefix.c_str(), mpp, topLeftX, topLeftY);

        ImagenomicAnalytics::IO::writeImage<itkUCharImageType>(nucleusBinaryMask, outputLabelName, 0);
      }


#pragma omp critical
      {
        char outputOverlayName[1000];
        sprintf(outputOverlayName, "%s_mpp_%g_x%ld_y%ld-overlay.jpg", outputPrefix.c_str(), mpp, topLeftX, topLeftY);

        itkRGBImageType::Pointer overlay = ImagenomicAnalytics::ScalarImage::generateSegmentationOverlay<char>(thisTileItk, nucleusBinaryMask);

        ImagenomicAnalytics::IO::writeImage<itkRGBImageType>(overlay, outputOverlayName, 0);
      }

    }

  outputFeatureFile.close();

  return 0;
}
