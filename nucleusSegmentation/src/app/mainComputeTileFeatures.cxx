#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <omp.h>


// openslide
#include "openslide.h"

// local
#include "itkTypedefs.h"

#include "utilityIO.h"
#include "MultipleObjectFeatureAnalysisFilter.h"



int main(int argc, char **argv)
{
  if (argc < 4)
    {
      std::cerr<<"Parameters: tileImageName tileSegmentationName outputFeatureName\n";
      exit(-1);
    }

  std::string tileImageName(argv[1]);
  std::string tileSegmentationName(argv[2]);
  std::string outputFeatureName(argv[3]);

  //--------------------------------------------------------------------------------
  // Init output feature file
  std::ofstream outputFeatureFile(outputFeatureName.c_str());
  //================================================================================

  itkRGBImageType::Pointer tile = ImagenomicAnalytics::IO::readImage<itkRGBImageType>(tileImageName.c_str());
  itkUCharImageType::Pointer maskOfTile = ImagenomicAnalytics::IO::readImage<itkUCharImageType>(tileSegmentationName.c_str());


  ImagenomicAnalytics::MultipleObjectFeatureAnalysisFilter featureAnalyzer;
  featureAnalyzer.setInputRGBImage(tile);
  featureAnalyzer.setObjectBinaryMask(maskOfTile);
  featureAnalyzer.update();

  std::vector< std::vector<FeatureValueType> > features = featureAnalyzer.getFeatures();

  for (std::size_t iObject = 0; iObject < features.size(); ++iObject)
    {
      for (std::size_t iFeature = 0; iFeature < features[iObject].size(); ++iFeature)
        {
          outputFeatureFile<<features[iObject][iFeature]<<",";
        }
      outputFeatureFile<<std::endl<<std::flush;
    }


  //--------------------------------------------------------------------------------
  // Close everything
  outputFeatureFile.close();
  //================================================================================

  return 0;
}
