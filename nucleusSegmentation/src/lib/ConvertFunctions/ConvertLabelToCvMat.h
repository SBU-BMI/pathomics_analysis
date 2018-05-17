#ifndef CONVERTLABELTOCVMAT_H
#define CONVERTLABELTOCVMAT_H

#include "itkImage.h"
#include "m_types.h"
#include "itkOpenCVImageBridge.h"
#include <opencv2/opencv.hpp>

cv::Mat_<int> label2CvMat(itkLabelImageType::Pointer m_objectLabelImage);

#endif //CONVERTLABELTOCVMAT_H
