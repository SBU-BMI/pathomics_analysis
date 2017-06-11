#ifndef UTILITY_FEATURES_IO
#define UTILITY_FEATURES_IO

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

// openslide
#include "openslide.h"

// local
#include "itkTypedefs.h"

#include "utilityIO.h"
#include "SingleObjectFeatureAnalysisFilter.h"
// #include "MultipleObjectFeatureAnalysisFilter.h"

void printFeatureNames(std::vector <std::string>, std::ofstream &);

void printFeatures(std::vector <std::vector<FeatureValueType> >, std::ofstream &, int);

#endif

