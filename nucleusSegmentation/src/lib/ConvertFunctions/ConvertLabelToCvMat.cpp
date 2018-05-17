/**
 * Purpose: Convert ITK image to OpenCV image
 */
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "ConvertLabelToCvMat.h"
#include "itkOpenCVImageBridge.h"
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

Mat_<int> label2CvMat(itkLabelImageType::Pointer input) {

    int cols = input->GetLargestPossibleRegion().GetSize()[0]; // width
    int rows = input->GetLargestPossibleRegion().GetSize()[1]; // height

    const int nx = cols;
    const int ny = rows;

    // zeros(int rows, int cols)
    // Mat_<int> output(rows, cols);
    Mat_<int> output = Mat_<int>::zeros(rows, cols);

    ImageType::IndexType index;
    for (int x = 0; x < nx; x++) // for all Columns
    {
        index[0] = x;
        for (int y = 0; y < ny; y++) // for all Rows
        {
            index[1] = y;
            int result = input->GetPixel(index);

            // y, x
            output(y, x) = result;

            //if (result > 0) {
            //std::cout << result << std::endl;
            //}
        }
    }

    return output;
}
