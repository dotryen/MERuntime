cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0048 NEW)

project(MERuntime VERSION 1.0.0 DESCRIPTION "MANIFOLDEngine Runtime")

add_subdirectory(runtime)

function(MERuntime_configure target)
    MECore_configure(${target})
    target_link_libraries(${target} PRIVATE ME::Runtime)
endfunction()
