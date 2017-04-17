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

set(nscalesrcdir ${CMAKE_BINARY_DIR}/app/nscale/src)
set(nscalebuilddir ${CMAKE_BINARY_DIR}/app/nscale-proj-prefix/src/nscale-proj-build)

#includes
include_directories(${nscalesrcdir}/segment)
include_directories(${nscalesrcdir}/segment/cuda)
include_directories(${nscalesrcdir}/common)
include_directories(${nscalesrcdir}/features)
include_directories(${nscalesrcdir}/normalization)
include_directories(${CMAKE_SOURCE_DIR}/lib/ConvertFunctions)

link_directories(${nscalebuilddir}/lib)

################################################################################
# Compile executables
SET(list1 "mainTileAndSegmentWSINucleiOutputFeature" "mainComputeShapeFeatures" "mainComputeTileFeatures")
foreach (item ${list1})
    message(STATUS "arg='${item}'")
    option(build_${item} "build ${item}?" ON)
    if (build_${item})
        set(cexx ${item})
        add_executable(${cexx} ${cexx}.cxx)
        target_link_libraries(${cexx} ${Libraries})
    endif (build_${item})
endforeach ()


#####################################################################
# Compile and link with NScale
SET(list2 "mainSegmentSmallImage" "mainSegmentFeatures" "computeFeatures" "computeFeaturesCombined")
foreach (item ${list2})
    message(STATUS "arg='${item}'")
    option(build_${item} "build ${item}?" ON)
    if (build_${item})
        set(cexx ${item})
        add_executable(${cexx} ${cexx}.cxx)
        target_link_libraries(${cexx} segment featuresAll ${Libraries}) # add libs
        add_dependencies(${cexx} nscale-proj) # make nscale available to cxx
    endif (build_${item})
endforeach ()
