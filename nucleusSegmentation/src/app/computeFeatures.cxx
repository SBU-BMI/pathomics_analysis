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

    itkLabelImageType::Pointer m_objectLabelImage = processMask(stData, mask);

    string var0 = (string) pgm;
    char p = var0[0];

    switch (p) {
        case 'Y':
            computeWriteYiFeatures(tile, m_objectLabelImage, output, x, y);
            break;
        case 'J':
            computeWriteJunFeatures(tile, m_objectLabelImage, output);
            break;
        default:
            cout << "Which program to run?  Next time type Y for Yi's, or J for Jun's." << endl;
    }

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
