#ifndef READUNKNOWNIMAGETYPE_H
#define READUNKNOWNIMAGETYPE_H

#include <itkImage.h>
#include <itkImageFileReader.h>

typedef struct {
    bool valid;
    int pixelType;
    int componentSize;
    bool binary;
    std::string filename;
} imageInfo;


imageInfo getImageInfo(std::string filename);

#endif //READUNKNOWNIMAGETYPE_H
