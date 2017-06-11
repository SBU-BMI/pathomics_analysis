#ifndef utilityWholeSlideProcessing_h_
#define utilityWholeSlideProcessing_h_

// itk
#include "itkTypedefs.h"

// openslide
#include "openslide.h"


namespace ImagenomicAnalytics
{
  namespace WholeSlideProcessing
  {
    //--------------------------------------------------------------------------------
    // extract magnification
    template<typename TNull>
    int extractMagnification(openslide_t* osr)
    {
      const char magnificationStringName[] = "aperio.AppMag";
      int magnification = 0;
      const char * const *property_names = openslide_get_property_names(osr);
      while (*property_names)
        {
          const char *name = *property_names;
          const char *value = openslide_get_property_value(osr, name);

          if (!strcmp(magnificationStringName, name))
            {
              magnification = atof(value);
              printf("%s: '%d'\n", name, magnification);

              break;
            }

          property_names++;
        }

      return magnification;
    }


    //--------------------------------------------------------------------------------
    // extract MPP
    template<typename TNull>
    float extractMPP(openslide_t* osr)
    {
      float mpp = 0.25; ///< The MPP of 40x, as default, in case there is no such info.

      std::string mppStringName("MPP");
      bool mppObtained = false;
      const char * const *property_names = openslide_get_property_names(osr);
      while (*property_names)
        {
          const char *name = *property_names;
          std::string nameString(name);
          std::transform(nameString.begin(), nameString.end(), nameString.begin(), ::toupper);

          const char *value = openslide_get_property_value(osr, name);

          if (nameString.find(mppStringName) != std::string::npos)
            {
              mpp = atof(value);
              mppObtained = true;

              break;
            }

          property_names++;
        }

      if (!mppObtained)
        {
          std::cerr<<"Warning: mpp is not obtained. Assuming to be 0.25.\n"<<std::flush;
        }

      return mpp;
    }


    template<typename TNull>
    void generateTileRegions(int64_t largestW, int64_t largestH, int64_t tileSize, \
                             std::vector<int64_t>& tileTopleftX, std::vector<int64_t>& tileTopleftY, std::vector<int64_t>& tileSizeX, std::vector<int64_t>& tileSizeY)
    {
      tileTopleftX.clear();
      tileTopleftY.clear();
      tileSizeX.clear();
      tileSizeY.clear();

      int64_t nTileW = largestW/tileSize;
      int64_t nTileH = largestH/tileSize;

      //--------------------------------------------------------------------------------
      // 1. top-left "inner region"
      for (int64_t iTileW = 0; iTileW < nTileW; ++iTileW)
        {
          for (int64_t iTileH = 0; iTileH < nTileH; ++iTileH)
            {
              int64_t topLeftX = iTileW*tileSize;
              int64_t topLeftY = iTileH*tileSize;

              tileTopleftX.push_back(topLeftX);
              tileTopleftY.push_back(topLeftY);
              tileSizeX.push_back(tileSize);
              tileSizeY.push_back(tileSize);
            }
        }
      //================================================================================

      //--------------------------------------------------------------------------------
      // 2. right-most colum, without the right-bottom corner
      {
        int64_t iTileW = nTileW;

        int64_t rightMostTileWidth = largestW % tileSize;
        int64_t rightMostTileHeight = tileSize;

        for (int64_t iTileH = 0; iTileH < nTileH; ++iTileH)
          {
            int64_t topLeftX = iTileW*tileSize;
            int64_t topLeftY = iTileH*tileSize;

            tileTopleftX.push_back(topLeftX);
            tileTopleftY.push_back(topLeftY);
            tileSizeX.push_back(rightMostTileWidth);
            tileSizeY.push_back(rightMostTileHeight);
          }
      }
      //================================================================================

      //--------------------------------------------------------------------------------
      // 3. bottom-most colum, without the right-bottom corner
      {
        int64_t iTileH = nTileH;

        int64_t bottomMostTileWidth = tileSize;
        int64_t bottomMostTileHeight = largestH % tileSize;

        for (int64_t iTileW = 0; iTileW < nTileW; ++iTileW)
          {
            int64_t topLeftX = iTileW*tileSize;
            int64_t topLeftY = iTileH*tileSize;

            tileTopleftX.push_back(topLeftX);
            tileTopleftY.push_back(topLeftY);
            tileSizeX.push_back(bottomMostTileWidth);
            tileSizeY.push_back(bottomMostTileHeight);
          }
      }
      //================================================================================

      //--------------------------------------------------------------------------------
      // 4. right-bottom corner
      {
        int64_t iTileH = nTileH;
        int64_t iTileW = nTileW;

        int64_t bottomRighCornerTileWidth = largestW % tileSize;
        int64_t bottomRighCornerTileHeight = largestH % tileSize;

        int64_t topLeftX = iTileW*tileSize;
        int64_t topLeftY = iTileH*tileSize;

        tileTopleftX.push_back(topLeftX);
        tileTopleftY.push_back(topLeftY);
        tileSizeX.push_back(bottomRighCornerTileWidth);
        tileSizeY.push_back(bottomRighCornerTileHeight);
      }
      //================================================================================
    }

    template<typename TNull>
    cv::Mat extractTileFromWSI(openslide_t* osr,                        \
                               int32_t level,                           \
                               int64_t topLeftX, int64_t topLeftY, int64_t sizeX, int64_t sizeY)
    {
      int64_t numOfPixelPerTile = sizeY*sizeX;

      uint32_t* dest = new uint32_t[numOfPixelPerTile];
      openslide_read_region(osr, dest, topLeftX, topLeftY, level, sizeX, sizeY);

      // Check if any errors when extracting the region
      uint32_t nonZero = 0;
      for (int64_t i=0;i<numOfPixelPerTile;i++)
    	  nonZero += dest[i];
      if (nonZero==0) { // Error extracting the region from the image
    	  cv::Mat emptyTile;
    	  delete[] dest;
    	  return emptyTile;
      }

      cv::Mat thisTile(sizeY, sizeX, CV_8UC3, cv::Scalar(0, 0, 0)); // for cv mat is (height, width), so (y, x)

      for (int64_t it = 0; it < numOfPixelPerTile; ++it)
        {
          uint32_t p = dest[it];

          uint8_t a = (p >> 24) & 0xFF;
          uint8_t r = (p >> 16) & 0xFF;
          uint8_t g = (p >> 8) & 0xFF;
          uint8_t b = p & 0xFF;

          switch (a) {
          case 0:
            r = 0;
            b = 0;
            g = 0;
            break;

          case 255:
            // no action needed
            break;

          default:
            r = (r * 255 + a / 2) / a;
            g = (g * 255 + a / 2) / a;
            b = (b * 255 + a / 2) / a;
            break;
          }

          // write back
          thisTile.at<cv::Vec3b>(it)[0] = b;
          thisTile.at<cv::Vec3b>(it)[1] = g;
          thisTile.at<cv::Vec3b>(it)[2] = r;
        }

      delete[] dest;

      return thisTile;
    }


  }
}// namespace


#endif
