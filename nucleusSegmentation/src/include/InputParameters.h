#ifndef _INPUT_PARAMETERS_H_
#define _INPUT_PARAMETERS_H_

#include <cstdio>
#include <iostream>
#include <string>
#include <omp.h>

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#define WSI     0
#define TILES   1
#define ONETILE 2
#define IMG     3
#define DEFAULT_WSI_TILE   4096
#define DEFAULT_SMALL_TILE 512
#define MASK_ONLY        1
#define MASK_IMG         2
#define MASK_IMG_OVERLAY 3

typedef struct _InputParameters {
    int inpType;  // wsi (0)|tile|img
    float otsuRatio;
    double curvatureWeight;
    float sizeLowerThld;
    float sizeUpperThld;
    float msKernel;
    int declumpingType;
    int64_t levelsetNumberOfIteration;
    int64_t topLeftX, topLeftY;
    int64_t sizeX, sizeY;
    float mpp;
    int64_t tileSizeX, tileSizeY;
    int outputLevel;
    std::string inpFile;
    std::string outFolder;
    std::string analysisId;
    std::string analysisDesc;
    std::string subjectId;
    std::string caseId;

    // compress the output files into a zip package
    std::string zipFile;
    int isZipped;
} InputParameters;

void printParseError(char *argv[]);

void printInputParameters(InputParameters *inpParams);

int parseInputParameters(int argc, char **argv, InputParameters *inpParams);

#endif // _INPUT_PARAMETERS_H_
