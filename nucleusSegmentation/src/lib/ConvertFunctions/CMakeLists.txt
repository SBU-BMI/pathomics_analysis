set(SOURCE_FILES
        ConvertBinaryToLabel.cpp
        ConvertLabelToCvMat.cpp
        ConvertRGBToLabel.cpp
        ConvertSlicerToLabel.cpp
        IsImageBinary.cpp
        ReadUnknownImageType.cpp
        )

set(nscalesrcdir ${CMAKE_BINARY_DIR}/app/nscale/src)
set(nscalebuilddir ${CMAKE_BINARY_DIR}/app/nscale-proj-prefix/src/nscale-proj-build)

include_directories(${nscalesrcdir}/segment)
include_directories(${nscalesrcdir}/segment/cuda)
include_directories(${nscalesrcdir}/common)
include_directories(${nscalesrcdir}/features)
include_directories(${nscalesrcdir}/normalization)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(ConvertFunctions ${SOURCE_FILES})
