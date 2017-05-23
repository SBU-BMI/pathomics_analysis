#include <cstring>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "InputParameters.h"

void printParseError(char *argv[]) {

    std::cerr << "Usage: " << argv[0] << " -h|[parameters]" << std::endl
              << "  Required arguments: " << std::endl
              << "   -t [onetile|tiles] " << std::endl
              << "   -i <input file>" << std::endl
              << "   -o <output folder>" << std::endl
              << "   -s <tile_minx,tile_miny>" << std::endl
              << "   -b <tile_width,tile_height>" << std::endl
              << "   -d <patch_width,patch_height>" << std::endl
              << "   -a <analysis_id: string>" << std::endl
              << "   -c <case_id: string>" << std::endl
              << "   -p <subject_id: string>" << std::endl
              << "  Optional arguments: " << std::endl
              << "   -r <otsuRatio> " << std::endl
              << "   -w <curvatureWeight>" << std::endl
              << "   -l <sizeLowerThld>" << std::endl
              << "   -u <sizeUpperThld>" << std::endl
              << "   -k <msKernel>" << std::endl
              << "   -j <declumpingType: 0=None | 1=MeanShift | 2=Watershed>" << std::endl
              << "   -n <levelsetNumberOfIterations>" << std::endl
              << "   -m <mpp>" << std::endl
              << "   -e <analysis desc: string>" << std::endl
              << "   -z <zipFile - compression works with onetile only.>" << std::endl
              << "   -v <output level: mask|mask:img|mask:img:overlay>" << std::endl;

    // alphabet letters remaining: f g h q x y
}

void printInputParameters(InputParameters *inpParams) {
    std::cout << "INPUT PARAMETERS: " << std::endl;
    switch (inpParams->inpType) {
        case WSI:
            std::cout << "input_type: WSI" << std::endl;
            break;
        case TILES:
            std::cout << "input_type: TILES" << std::endl;
            break;
        case ONETILE:
            std::cout << "input_type: ONETILE" << std::endl;
            break;
        case IMG:
            std::cout << "input_type: IMG" << std::endl;
            break;
        default:
            std::cerr << "Error: Undefined input type." << std::endl;
            break;
    }
    std::cout << "otsu_ratio: " << inpParams->otsuRatio << std::endl;
    std::cout << "curvature_weight: " << inpParams->curvatureWeight << std::endl;
    std::cout << "min_size: " << inpParams->sizeLowerThld << std::endl;
    std::cout << "max_size: " << inpParams->sizeUpperThld << std::endl;
    std::cout << "ms_kernel: " << inpParams->msKernel << std::endl;
    std::cout << "declump_type: " << inpParams->declumpingType << std::endl;
    std::cout << "levelset_num_iters: " << inpParams->levelsetNumberOfIteration << std::endl;
    std::cout << "tile_minx: " << inpParams->topLeftX << " tile_miny: " << inpParams->topLeftY << std::endl;
    std::cout << "tile_width: " << inpParams->sizeX << " tile_height: " << inpParams->sizeY << std::endl;
    std::cout << "mpp: " << inpParams->mpp << std::endl;
    std::cout << "patch_width: " << inpParams->tileSizeX << " patch_height: " << inpParams->tileSizeY << std::endl;
    std::cout << "input_file: " << inpParams->inpFile << std::endl;
    std::cout << "subject_id: " << inpParams->subjectId << std::endl;
    std::cout << "case_id: " << inpParams->caseId << std::endl;
    std::cout << "output_folder: " << inpParams->outFolder << std::endl;
    std::cout << "analysis_id: " << inpParams->analysisId << std::endl;
    std::cout << "analysis_desc: " << inpParams->analysisDesc << std::endl;
    if (inpParams->isZipped) std::cout << "zipFile: " << inpParams->zipFile << std::endl;
    switch (inpParams->outputLevel) {
        case MASK_ONLY:
            std::cout << "output_level: MASK" << std::endl;
            break;
        case MASK_IMG:
            std::cout << "output_level: MASK:IMG" << std::endl;
            break;
        case MASK_IMG_OVERLAY:
            std::cout << "output_level: MASK:IMG:OVERLAY" << std::endl;
            break;
        default:
            std::cerr << "ERROR: Undefined output level." << std::endl;
            break;
    }
}

int parseInputParameters(int argc, char **argv, InputParameters *inpParams) {
    int c;

    if (argc < 4) return 1;

    inpParams->inpType = WSI;
    inpParams->otsuRatio = 1.0;
    inpParams->curvatureWeight = 0.8;
    inpParams->sizeLowerThld = 3.0;
    inpParams->sizeUpperThld = 200.0;
    inpParams->msKernel = 20.0;
    inpParams->declumpingType = 0;
    inpParams->levelsetNumberOfIteration = 100;
    inpParams->topLeftX = 0;
    inpParams->topLeftY = 0;
    inpParams->sizeX = DEFAULT_SMALL_TILE;
    inpParams->sizeY = DEFAULT_SMALL_TILE;
    inpParams->mpp = 0.25; // 40x objective
    inpParams->tileSizeX = 0;
    inpParams->tileSizeY = 0;
    inpParams->outputLevel = MASK_ONLY;
    inpParams->isZipped = 0; // no compressed zip output

    inpParams->subjectId = "";
    inpParams->caseId = "";

    opterr = 0;
    int t_required = 0;
    int i_required = 0;
    int o_required = 0;
    int s_required = 0;
    int b_required = 0;
    int d_required = 0;
    int a_required = 0;
    int c_required = 0;
    int p_required = 0;
    while ((c = getopt(argc, argv, "ht:i:o:m:r:w:l:u:k:j:n:s:b:d:v:a:e:c:p:z:")) != -1) {
        switch (c) {
            case 'h':
                return 1;
            case 't': {
                if (!strcmp(optarg, "wsi")) {
                    inpParams->inpType = WSI;
                } else if (!strcmp(optarg, "tiles")) {
                    inpParams->inpType = TILES;
                } else if (!strcmp(optarg, "onetile")) {
                    inpParams->inpType = ONETILE;
                } else if (!strcmp(optarg, "img")) {
                    inpParams->inpType = IMG;
                } else {
                    fprintf(stderr, "Undefined input type.\n");
                    return 1;
                }
                t_required = 1;
                break;
            }
            case 'i':
                inpParams->inpFile = optarg;
                i_required = 1;
                break;
            case 'o':
                inpParams->outFolder = optarg;
                o_required = 1;
                break;
            case 'z':
                inpParams->isZipped = 1;
                inpParams->zipFile = optarg;
                break;
            case 'c':
                inpParams->caseId = optarg;
                c_required = 1;
                break;
            case 'p':
                inpParams->subjectId = optarg;
                p_required = 1;
                break;
            case 'a':
                inpParams->analysisId = optarg;
                a_required = 1;
                break;
            case 'e':
                inpParams->analysisDesc = optarg;
                break;
            case 'm':
                inpParams->mpp = atof(optarg);
                break;
            case 'r':
                inpParams->otsuRatio = atof(optarg);
                break;
            case 'w':
                inpParams->curvatureWeight = atof(optarg);
                break;
            case 'l':
                inpParams->sizeLowerThld = atof(optarg);
                break;
            case 'u':
                inpParams->sizeUpperThld = atof(optarg);
                break;
            case 'k':
                inpParams->msKernel = atof(optarg);
                break;
            case 'j': {
                // Check alpha or numeric
                std::string temp = strdup(optarg);
                if (isalpha(temp[0]))
                {
                    fprintf(stderr, "ERROR: Option -j should be integer [ 0 | 1 | 2 ] value.\n");
                    return 1;
                }
                else
                {
                    inpParams->declumpingType = atoi(optarg);
                }
                break;
            }
            case 'n':
                inpParams->levelsetNumberOfIteration = (int64_t) atoi(optarg);
                break;
            case 's': {
                std::istringstream ss(optarg);
                std::string token;
                if (std::getline(ss, token, ',')) {
                    inpParams->topLeftX = atoi(token.c_str());
                } else {
                    fprintf(stderr, "ERROR: Option -s is missing <leftX,leftY> value.\n");
                    return 1;
                }
                if (std::getline(ss, token, ',')) {
                    inpParams->topLeftY = atoi(token.c_str());
                } else {
                    fprintf(stderr, "ERROR: Option -s is missing <leftX,leftY> value.\n");
                    return 1;
                }
                s_required = 1;
                break;
            }
            case 'b': {
                std::istringstream ss(optarg);
                std::string token;
                if (std::getline(ss, token, ',')) {
                    inpParams->sizeX = atoi(token.c_str());
                } else {
                    fprintf(stderr, "ERROR: Option -b is missing <sizeX,sizeY> value.\n");
                    return 1;
                }
                if (std::getline(ss, token, ',')) {
                    inpParams->sizeY = atoi(token.c_str());
                } else {
                    fprintf(stderr, "ERROR: Option -b is missing <sizeX,sizeY> value.\n");
                    return 1;
                }
                b_required = 1;
                break;
            }
            case 'd': {
                std::istringstream ss(optarg);
                std::string token;
                if (std::getline(ss, token, ',')) {
                    inpParams->tileSizeX = atoi(token.c_str());
                } else {
                    fprintf(stderr, "ERROR: Option -d is missing <patchSizeX,patchSizeY> value.\n");
                    return 1;
                }
                if (std::getline(ss, token, ',')) {
                    inpParams->tileSizeY = atoi(token.c_str());
                } else {
                    fprintf(stderr, "ERROR: Option -d is missing <patchSizeX,patchSizeY> value.\n");
                    return 1;
                }
                d_required = 1;
                break;
            }
            case 'v':
                if (!strcmp(optarg, "mask")) {
                    inpParams->outputLevel = MASK_ONLY;
                } else if (!strcmp(optarg, "mask:img")) {
                    inpParams->outputLevel = MASK_IMG;
                } else if (!strcmp(optarg, "img:mask")) {
                    inpParams->outputLevel = MASK_IMG;
                } else if (!strcmp(optarg, "mask:img:overlay")) {
                    inpParams->outputLevel = MASK_IMG_OVERLAY;
                } else if (!strcmp(optarg, "mask:overlay:img")) {
                    inpParams->outputLevel = MASK_IMG_OVERLAY;
                } else if (!strcmp(optarg, "img:mask:overlay")) {
                    inpParams->outputLevel = MASK_IMG_OVERLAY;
                } else if (!strcmp(optarg, "img:overlay:mask")) {
                    inpParams->outputLevel = MASK_IMG_OVERLAY;
                } else if (!strcmp(optarg, "overlay:mask:img")) {
                    inpParams->outputLevel = MASK_IMG_OVERLAY;
                } else if (!strcmp(optarg, "overlay:img:mask")) {
                    inpParams->outputLevel = MASK_IMG_OVERLAY;
                } else {
                    fprintf(stderr, "Undefined output level value.\n");
                    return 1;
                }
                break;
            default:
                return 1;
        }
    }

    if (!(t_required && i_required && o_required
          && s_required && b_required && d_required
          && a_required && c_required && p_required)) {
        fprintf(stderr, "Missing required arguments.");
        return 1;
    }

    if (inpParams->subjectId.compare("") == 0 || inpParams->caseId.compare("") == 0) {
        fprintf(stderr, "Missing subject id and/or case id.");
        return 1;
    }

    if (inpParams->tileSizeX == 0 || inpParams->tileSizeY == 0) {
        if (inpParams->inpType == WSI) {
            inpParams->tileSizeX = DEFAULT_WSI_TILE;
            inpParams->tileSizeY = DEFAULT_WSI_TILE;
        } else {
            inpParams->tileSizeX = DEFAULT_SMALL_TILE;
            inpParams->tileSizeY = DEFAULT_SMALL_TILE;
        }
    }

    if (inpParams->topLeftX < 0 || inpParams->topLeftY < 0
        || inpParams->sizeX < 0 || inpParams->sizeY < 0) {
        fprintf(stderr, "Error in input parameter values. Check -s and -b values; one or more values are negative.\n");
        return 1;
    }

    if (opterr) return 1;

    return 0;
}
