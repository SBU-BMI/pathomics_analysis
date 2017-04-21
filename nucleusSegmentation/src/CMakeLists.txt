cmake_minimum_required(VERSION 2.8)
if (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

set(projName "pathomicsAnalysis")
project(${projName})

set(CMAKE_C_FLAGS "-std=c99 -fPIC")
set(CMAKE_CXX_FLAGS " -fPIC")

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

################################################################################
# ITK, Should be with ItkVtkGlue
find_package(ITK)
if (ITK_FOUND)
    include(${ITK_USE_FILE})
endif (ITK_FOUND)

################################################################################
# put all libs together
set(Libraries
        ${ITK_LIBRARIES}
        ${Libraries}
        )

################################################################################
# openCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
set(Libraries
        ${OpenCV_LIBS}
        ${Libraries}
        )

include_directories(${CMAKE_SOURCE_DIR}/include/)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(GLIB)
if (HAVE_GLIB)
    include_directories(${GLIB_INCLUDES})
    set(Libraries
            ${GLIB_LIBRARIES}
            ${Libraries}
            )
endif (HAVE_GLIB)

option(build_mainTileAndSegmentWSINucleiOutputFeature "build mainTileAndSegmentWSINucleiOutputFeature?" ON)

################################################################################
# find OpenSlide path
if (build_mainTileAndSegmentWSINucleiOutputFeature)
    find_path(OpenSlide_INSTALLATION_PATH include/openslide/openslide.h REQUIRED)
    if (NOT OpenSlide_INSTALLATION_PATH)
        message(FATAL_ERROR "Please enter the installation path of OpenSlide, i.e., the one after --prefix= when configuring OpenSlide")
    endif (NOT OpenSlide_INSTALLATION_PATH)

    include_directories(${OpenSlide_INSTALLATION_PATH}/include/openslide/)

    set(OpenSlideLibs
            ${OpenSlide_INSTALLATION_PATH}/lib/libopenslide.so
            )
endif (build_mainTileAndSegmentWSINucleiOutputFeature)

################################################################################
# put all libs together
set(Libraries
        ${OpenSlideLibs}
        ${Libraries}
        )

################################################################################
# build local libraries

################################################################################
# Include external project nscale
include(ExternalProject)

ExternalProject_Add(nscale-proj
        DOWNLOAD_DIR nscale
        GIT_REPOSITORY https://github.com/SBU-BMI/nscale.git
        SOURCE_DIR nscale
        CMAKE_ARGS -DOpenCV_DIR=${OpenCV_DIR}
        INSTALL_COMMAND ""
        )

set(nscalesrcdir ${CMAKE_BINARY_DIR}/nscale/src)
set(nscalebuilddir ${CMAKE_BINARY_DIR}/nscale-proj-prefix/src/nscale-proj-build)

#includes
include_directories(${nscalesrcdir}/segment)
include_directories(${nscalesrcdir}/segment/cuda)
include_directories(${nscalesrcdir}/common)
include_directories(${nscalesrcdir}/features)
include_directories(${nscalesrcdir}/normalization)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/ConvertFunctions)

link_directories(${nscalebuilddir}/lib)

add_library(localLibraries
        lib/MultipleObjectFeatureAnalysisFilter.cxx
        lib/SingleObjectFeatureAnalysisFilter.cxx
        lib/BinaryMaskAnalysisFilter.cxx
        lib/InputParameters.cxx
        lib/Normalization.cpp
        lib/PixelOperations.cpp
        lib/Logger.cpp
        lib/ConvertFunctions/ConvertBinaryToLabel.cpp
        lib/ConvertFunctions/ConvertLabelToCvMat.cpp
        lib/ConvertFunctions/ConvertRGBToLabel.cpp
        lib/ConvertFunctions/ConvertSlicerToLabel.cpp
        lib/ConvertFunctions/IsImageBinary.cpp
        lib/ConvertFunctions/ReadUnknownImageType.cpp
        lib/utilityFeaturesIO.cxx)

add_dependencies(localLibraries nscale-proj) # make nscale available to lib

set(Libraries
        localLibraries
        ${Libraries}
        )

add_subdirectory(app)
