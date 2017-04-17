SET( list1 "mainTileAndSegmentWSINucleiOutputFeature" "mainComputeShapeFeatures" "mainComputeTileFeatures" )
foreach(ll ${list1})
    # message(STATUS "arg='${ll}'")
    option(build_${ll} "build ${ll}?" ON)
    if (build_${ll})
        set(cexx ${ll})
        add_executable(${cexx} ${cexx}.cxx)
        target_link_libraries(${cexx} ${Libraries})
    endif (build_${ll})
endforeach()


#####################################################################
# Link with NScale
SET( list2 "mainSegmentSmallImage" "mainSegmentFeatures" "computeFeatures" "computeFeaturesCombined" )
option(build_${ll} "build ${ll}?" ON)
foreach(ll ${list2})
    if (build_${ll})
        set(cexx ${ll})
        add_executable(${cexx} ${cexx}.cxx)
        target_link_libraries(${cexx} segment featuresAll ${Libraries})
        add_dependencies(${ll} nscale-proj)
    endif (build_${ll})
endforeach()
