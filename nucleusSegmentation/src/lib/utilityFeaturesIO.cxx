#include "utilityFeaturesIO.h"

void printFeatureNames(std::vector<std::string> featureNames, std::ofstream &outputFeatureFile) {
    for (std::size_t iFeature = 0; iFeature < featureNames.size() - 1; ++iFeature) {
        outputFeatureFile << featureNames[iFeature] << ",";
    }
    outputFeatureFile << featureNames[featureNames.size() - 1];
    outputFeatureFile << std::endl << std::flush;
}

void printFeatures(std::vector <std::vector<FeatureValueType> > features, std::ofstream &outputFeatureFile, int nf) {
    // Format for each nucleus's feature is: 1,2,1.1,[polygonx1:polygony1:polygonx2:polygony2]
    for (std::size_t iObject = 0; iObject < features.size(); ++iObject) {
        for (int it = 0; it < (nf - 1); ++it) {
            outputFeatureFile << features[iObject][it] << ",";
        }
        outputFeatureFile << "[";

        for (std::size_t it = nf - 1; it < features[iObject].size() - 1; ++it) {
            outputFeatureFile << features[iObject][it] << ":";
        }
        outputFeatureFile << features[iObject][features[iObject].size() - 1];
        outputFeatureFile << "]" << std::endl;
    }
}
