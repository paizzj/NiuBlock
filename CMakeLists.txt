project(niublock)

cmake_minimum_required(VERSION 2.8.2)


set(TOPDIR ${CMAKE_CURRENT_SOURCE_DIR})


set(OUTDIR ${TOPDIR}/cmake-build/out/)
set(BUILDDIR ${TOPDIR}/cmake-build/build)


#SET(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

add_compile_options(-Wall -Wno-unused-local-typedefs -O2 -Os -fomit-frame-pointer)
add_compile_options("--std=c++11")


add_subdirectory(${TOPDIR}/example/src ${BUILDDIR}/example/src)
add_subdirectory(${TOPDIR}/example/test ${BUILDDIR}/example/test)
