#ifndef CONVERTSLICERTOLABEL_H
#define CONVERTSLICERTOLABEL_H

#include <string>
#include <itkRGBPixel.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include "m_types.h"
#include "itkOpenCVImageBridge.h"

itkLabelImageType::Pointer slicer2label(std::string filename);

#endif //CONVERTSLICERTOLABEL_H
