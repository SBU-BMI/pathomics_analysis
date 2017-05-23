#ifndef CONVERTRGBTOLABEL_H
#define CONVERTRGBTOLABEL_H

#include <itkRGBPixel.h>
#include <itkImage.h>
#include "m_types.h"

itkLabelImageType::Pointer rgb2label(itkRGBImageType::Pointer maskOfTile);

#endif //CONVERTRGBTOLABEL_H
