SET(list1 "mainTileAndSegmentWSINucleiOutputFeature" "mainComputeShapeFeatures" "mainComputeTileFeatures")
foreach (item ${list1})
    # message(STATUS "arg='${item}'")
    option(build_${item} "build ${item}?" ON)
    if (build_${item})
        set(cexx ${item})
        add_executable(${cexx} ${cexx}.cxx)
        target_link_libraries(${cexx} ${Libraries})
    endif (build_${item})
endforeach ()


#####################################################################
# Link with NScale
SET(list2 "mainSegmentSmallImage" "mainSegmentFeatures" "computeFeatures" "computeFeaturesCombined")
foreach (item ${list2})
    # message(STATUS "arg='${item}'")
    option(build_${item} "build ${item}?" ON)
    if (build_${item})
        set(cexx ${item})
        add_executable(${cexx} ${cexx}.cxx)
        target_link_libraries(${cexx} ${Libraries}) # add libs
        add_dependencies(${cexx} nscale-proj) # make nscale available to cxx
    endif (build_${item})
endforeach ()
