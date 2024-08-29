cmake_minimum_required(VERSION 3.10)

project(Odetect)

set(WORKING_DIR ${CMAKE_SOURCE_DIR})

set(SRC_DIR ${WORKING_DIR}/src)
set(CAPI_SRC_DIR ${SRC_DIR}/capi)
set(MODELS_DIR ${WORKING_DIR}/include/models)

include_directories(${WORKING_DIR}/include ${WORKING_DIR}/thirdparty/include)

file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp)
list(APPEND SOURCES ${WORKING_DIR}/main.cpp)
file(GLOB SOURCES_CAPI ${CAPI_SRC_DIR}/*.c)

set(ALL_SOURCES ${SOURCES} ${SOURCES_CAPI})

find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_search_module(GSTREAMER-APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0)
pkg_search_module(OPENCV REQUIRED IMPORTED_TARGET opencv4)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/model_list.cpp
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_model_list.cmake
    DEPENDS ${MODELS_DIR}
    COMMENT "Generating model_list.cpp"
)

add_custom_target(generate_model_list ALL DEPENDS ${CMAKE_BINARY_DIR}/model_list.cpp)

add_executable(odetect ${ALL_SOURCES} ${CMAKE_BINARY_DIR}/model_list.cpp)

target_link_libraries(odetect
    PkgConfig::GSTREAMER
    PkgConfig::GSTREAMER-APP
    PkgConfig::OPENCV
)

install(TARGETS odetect DESTINATION bin)
