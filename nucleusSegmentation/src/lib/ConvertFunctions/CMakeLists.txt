set(SOURCE_FILES
        ConvertBinaryToLabel.cpp
        ConvertLabelToCvMat.cpp
        ConvertRGBToLabel.cpp
        ConvertSlicerToLabel.cpp
        IsImageBinary.cpp
        ReadUnknownImageType.cpp
        )

include_directories(${PROJECT_SOURCE_DIR}/include)
	
add_library(ConvertFunctions ${SOURCE_FILES})
