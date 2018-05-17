#ifndef CONVERTBINARYTOLABEL_H
#define CONVERTBINARYTOLABEL_H

#include <string>
#include <itkRGBPixel.h>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include "m_types.h"

itkLabelImageType::Pointer bin2label(std::string filename);

#endif //CONVERTBINARYTOLABEL_H
