cmake_minimum_required(VERSION 3.5)

project(gate_recorder LANGUAGES CXX)

set (CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(jackcpp STATIC
    jackcpp/src/jackaudioio.cpp
    jackcpp/src/jackblockingaudioio.cpp
    jackcpp/src/jackmidiport.cpp
    )

target_include_directories(jackcpp PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/jackcpp/include>
    $<INSTALL_INTERFACE:include/jackcpp>
    )

#set(ENABLE_DFT ON)
add_subdirectory(kfr)

set(PLOG_BUILD_SAMPLES OFF)
add_subdirectory(plog)

set(CXXOPTS_BUILD_EXAMPLES OFF)
set(CXXOPTS_BUILD_TESTS OFF)
set(CXXOPTS_ENABLE_INSTALL OFF)
add_subdirectory(cxxopts)

add_executable(gate_recorder main.cpp gaterecorder.cpp)

target_link_libraries(gate_recorder kfr kfr_io jackcpp jack cxxopts plog)

install(
    TARGETS gate_recorder
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
