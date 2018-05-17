/*=========================================================================
 *
 *  CHECK TILE AND COMPUTE FEATURES
 *
 *  Use this to call feature computation algorithms.
 *  Yi = working dir
 *  Jun = nscale dir
 *
 *=========================================================================*/

#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <ConvertSlicerToLabel.h>

// Image conversion codes
#include "ReadUnknownImageType.h"
#include "IsImageBinary.h"
#include "ConvertRGBToLabel.h"
#include "ConvertBinaryToLabel.h"
#include "ConvertLabelToCvMat.h"

// Yi Feature Computation
#include "itkTypedefs.h"
#include "utilityIO.h"
#include "SingleObjectFeatureAnalysisFilter.h"
#include "MultipleObjectFeatureAnalysisFilter.h"

// Jun (nscale) Feature Computation
#include "ImageRegionNucleiData.h"
#include "ImageRegionNucleiDataIO.h"

using namespace std;
using namespace cv;

itkLabelImageType::Pointer processMask(imageInfo stData, char *mask);

template<typename itkImage_t>
typename itkImage_t::Pointer readImage(char *fileName);

void computeWriteYiFeatures(char *tileImageName,
                            itkLabelImageType::Pointer m_objectLabelImage,
                            char *outputFileName, int topLeftX, int topLeftY);

void computeWriteJunFeatures(char *tileImageName,
                             itkLabelImageType::Pointer m_objectLabelImage,
                             char *outputFileName);

/**
 * Write feature values, including Polygon data.
 * Combined = Yi's and Jun's features.
 */
int writeCombinedComputedFeaturesCSV(char *outputFile, int64_t topLeftX, int64_t topLeftY,
                            std::vector<std::string> &yiFeatureNames,
                            std::vector<std::vector<FeatureValueType> > &yiFeatureValues,
                            std::vector<std::string> &junFeatureNames,
                            std::vector<std::vector<double> > &junFeatureValues) {
    char outputFeatureName[1000];
    sprintf(outputFeatureName, "%s", outputFile);
    std::ofstream outputFeatureFile(outputFeatureName);

    // write the header
    int junFeaturesLength = junFeatureNames.size();
    for (int i = 0; i < junFeaturesLength; i++) {
        outputFeatureFile << junFeatureNames[i] << ",";
    }

    int yiFeaturesLength = yiFeatureNames.size();
    for (int i = 0; i < yiFeaturesLength - 1; i++) {
        outputFeatureFile << yiFeatureNames[i] << ",";
    }
    outputFeatureFile << yiFeatureNames[yiFeaturesLength - 1] << std::endl;

    // Write feature values. First, Jun's, then Yi's including Polygon data.
    // Format for each nucleus's feature is: 1,2,1.1,[polygonx1:polygony1:polygonx2:polygony2]
    for (int i = 0; i < junFeatureValues.size(); i++) {

        // output Jun's features
        for (int j = 0; j < junFeaturesLength; j++) {
            outputFeatureFile << junFeatureValues[i][j] << ",";
        }

        // output Yi's non-polygon features
        for (int j = 0; j < yiFeaturesLength - 1; j++) {
            outputFeatureFile << yiFeatureValues[i][j] << ",";
        }

        // output Yi's polygon features
        outputFeatureFile << "[";
        for (std::size_t itPolygon = yiFeaturesLength - 1; itPolygon < yiFeatureValues[i].size() - 1; ++itPolygon) {
            outputFeatureFile << yiFeatureValues[i][itPolygon] << ":";
        }
        outputFeatureFile << yiFeatureValues[i][yiFeatureValues[i].size() - 1] << "]\n";
    }

    outputFeatureFile.close();
}


int main(int argc, char *argv[]) {

    // Check the number of parameters
    if (argc < 5) {
        // Tell the user how to run the program
        cerr << "Usage: " << argv[0] << " IMAGE MASK ALG OUTFILE [x y (optional)]" << endl;
        cerr << "Note: mask should be labeled" << endl;
        exit(EXIT_FAILURE);
    }

    char *tile = argv[1];
    char *mask = argv[2];
    char *pgm = argv[3];
    char *output = argv[4];

    int x = 0;
    int y = 0;

    if (argc > 5) {
        x = atoi(argv[5]);
        y = atoi(argv[6]);
    }

    cout << argc << endl;

    string inputFilename(mask);
    imageInfo stData = getImageInfo(inputFilename);

    itkLabelImageType::Pointer labelImage = processMask(stData, mask);

	// Read in the image
    cv::Mat thisTile = imread(tile);
    // Convert image to ITK image.
    itkRGBImageType::Pointer thisTileItk = itk::OpenCVImageBridge::CVMatToITKImage<itkRGBImageType>(thisTile);

	// Compute Yi's features
    ImagenomicAnalytics::MultipleObjectFeatureAnalysisFilter featureAnalyzer;
    featureAnalyzer.setInputRGBImage(thisTileItk);
    featureAnalyzer.setObjectLabeledMask(labelImage);
    featureAnalyzer.setTopLeft(x,y);
    featureAnalyzer.setFeatureNames();
    featureAnalyzer.update();

    std::vector<std::vector<FeatureValueType> > yiFeatureValues = featureAnalyzer.getFeatures();
    std::vector<std::string> yiFeatureNames = featureAnalyzer.getFeatureNames();

	// Compute Jun's features
    ImageRegionNucleiData nucleiData(0, 0, thisTile.cols - 1, thisTile.rows - 1);

    Mat_<int> labeledMask = label2CvMat(labelImage);

    nucleiData.extractBoundingBoxesFromLabeledMask(labeledMask);

    if (yiFeatureValues.size() != nucleiData.getNumberOfNuclei()) {
        fprintf(stderr, "the sizes of Yi's and Jun's feature sets do not match: %lu != %d\n", yiFeatureValues.size(),
                nucleiData.getNumberOfNuclei());
        exit(-1);
    }

    // cout << "COMP COUNT: " << nucleiData.getNumberOfNuclei() << endl;

    if (nucleiData.getNumberOfNuclei() > 0) {
        nucleiData.extractPolygonsFromLabeledMask(labeledMask);
        nucleiData.extractCytoplasmRegions(labeledMask);
        nucleiData.computeShapeFeatures(labeledMask);
        nucleiData.computeRedBlueChannelTextureFeatures(thisTile, labeledMask);
    }
    std::vector<std::string> junFeatureNames = nucleiData.getFeatureNamesVector();
    std::vector<std::vector<double> > junFeatureValues = nucleiData.getFeatureValuesVector();

    // Write features
    writeCombinedComputedFeaturesCSV(output, x, y, yiFeatureNames, yiFeatureValues, junFeatureNames, junFeatureValues);

    return EXIT_SUCCESS;
}

/**
 * Figure out what to do, based on the information we got back from
 * ReadUnknownImageType->getImageInfo().
 * @param stData
 */
itkLabelImageType::Pointer processMask(imageInfo stData, char *fileName) {

    //itkLabelImageType::Pointer m_objectLabelImage = itkLabelImageType::New();
    itkLabelImageType::Pointer m_objectLabelImage;

    if (stData.valid) {

        switch (stData.pixelType) {
            case 1: {
                if (stData.componentSize > 1) {
                    cout << "Image from Slicer." << endl;

                    // Read file to find out if binary or labeled.
                    stData.binary = isBinary(stData.filename);
                    if (stData.binary) {
                        cout << "Slicer image is binary. Converting..." << endl;
                        m_objectLabelImage = slicer2label(stData.filename);
                    } else {
                        // labeled = this is what we want. Continue.
                        cout << "Slicer image is labeled." << endl;
                        m_objectLabelImage = readImage<itkLabelImageType>(fileName);
                    }

                } else {
                    // Read file to find out if binary or labeled.
                    stData.binary = isBinary(stData.filename);
                    if (stData.binary) {
                        cout << "Image is binary. Converting..." << endl;
                        m_objectLabelImage = bin2label(stData.filename);
                    } else {
                        // labeled = this is what we want. Continue.
                        cout << "Image is labeled. Good!" << endl;
                        m_objectLabelImage = readImage<itkLabelImageType>(fileName);
                    }
                }
                break;
            }
            case 2: {

                cout << "Image is RGB. Converting..." << endl;
                itkRGBImageType::Pointer maskOfTile = readImage<itkRGBImageType>(fileName);
                m_objectLabelImage = rgb2label(maskOfTile);
                break;
            }
            default:
                // Do something else.
                break;
        }
    } else {
        cerr << "Something went wrong." << endl;
    }
    return m_objectLabelImage;
}

/**
 * READ AN IMAGE.
 *
 * @param fileName
 * @return
 */
template<typename itkImage_t>
typename itkImage_t::Pointer readImage(char *fileName) {
    typedef itk::ImageFileReader <itkImage_t> ImageReaderType;
    typename ImageReaderType::Pointer reader = ImageReaderType::New();
    reader->SetFileName(fileName);

    typename itkImage_t::Pointer image;

    try {
        reader->Update();
        image = reader->GetOutput();
    }
    catch (itk::ExceptionObject &err) {
        cerr << "ExceptionObject caught !" << endl;
        cerr << err << endl;
        raise(SIGABRT);
    }

    return image;
}

/**
 * YI FEATURES.
 *
 * @param tileImageName
 * @param m_objectLabelImage
 * @param outputFileName
 */
void computeWriteYiFeatures(char *tileImageName,
                            itkLabelImageType::Pointer m_objectLabelImage,
                            char *outputFileName, int topLeftX, int topLeftY) {

    ofstream outputFeatureFile(outputFileName);
    itkRGBImageType::Pointer tile = ImagenomicAnalytics::IO::readImage<itkRGBImageType>(tileImageName);

    ImagenomicAnalytics::MultipleObjectFeatureAnalysisFilter featureAnalyzer;
    featureAnalyzer.setInputRGBImage(tile);
    featureAnalyzer.setObjectLabeledMask(m_objectLabelImage);
    featureAnalyzer.setTopLeft(topLeftX, topLeftY);
    featureAnalyzer.setFeatureNames();
    featureAnalyzer.update();

    vector <vector<FeatureValueType> > features = featureAnalyzer.getFeatures();
    featureAnalyzer.outputFeaturesCSV(outputFeatureFile, features);

    outputFeatureFile.close();
    return;
}

/**
 * JUN FEATURES.
 *
 * @param tileImageName
 * @param m_objectLabelImage
 * @param outputFileName
 */
void computeWriteJunFeatures(char *tileImageName,
                             itkLabelImageType::Pointer m_objectLabelImage,
                             char *outputFileName) {

    Mat inpImage = imread(tileImageName, CV_LOAD_IMAGE_COLOR);

    ImageRegionNucleiData nucleiData(0, 0, inpImage.cols - 1, inpImage.rows - 1);

    Mat_<int> labeledMask = label2CvMat(m_objectLabelImage);

    nucleiData.extractBoundingBoxesFromLabeledMask(labeledMask);

    cout << "COMP COUNT: " << nucleiData.getNumberOfNuclei() << endl;

    if (nucleiData.getNumberOfNuclei() > 0) {
        nucleiData.extractPolygonsFromLabeledMask(labeledMask);
        nucleiData.extractCytoplasmRegions(labeledMask);
        nucleiData.computeShapeFeatures(labeledMask);
        nucleiData.computeRedBlueChannelTextureFeatures(inpImage, labeledMask);
        writeU24CSVFile(outputFileName, nucleiData);
    }

    return;
}
