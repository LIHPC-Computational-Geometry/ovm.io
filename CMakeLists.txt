
cmake_minimum_required(VERSION 3.0)

project(ovm_io)

set(
  GEOGRAM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ext/geogram/"
  CACHE PATH "full path to the Geogram installation"
)

include(${CMAKE_SOURCE_DIR}/cmake/ovm_io.cmake)

file(GLOB SRCFILES src/*.cpp )
include_directories(include)

add_subdirectory(ext/geogram)

add_executable(ovm.io ${SRCFILES} app/ovm.io.cpp)
target_link_libraries(ovm.io geogram)
