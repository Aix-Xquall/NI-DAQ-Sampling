cmake_minimum_required(VERSION 3.20)

project(cdaq_reader LANGUAGES CXX)

# C++ 標準 & 一些編譯選項
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# NI-DAQmx ANSI C Dev 根目錄（你給的路徑，做成 CMake cache 變數方便之後修改）
set(NI_DAQMX_ROOT
    "C:/Program Files (x86)/National Instruments/NI-DAQ/DAQmx ANSI C Dev"
    CACHE PATH "NI-DAQmx C Dev root directory")

# include 路徑
include_directories("${NI_DAQMX_ROOT}/include")

# 尋找 NIDAQmx.lib
find_library(NIDAQMX_LIBRARY
    NAMES NIDAQmx
    PATHS "${NI_DAQMX_ROOT}/lib/msvc"
    REQUIRED)

message(STATUS "Found NI-DAQmx library: ${NIDAQMX_LIBRARY}")

# 建 executable
add_executable(cdaq_reader
    src/main.cpp)

# 連結 NIDAQmx
target_link_libraries(cdaq_reader PRIVATE "${NIDAQMX_LIBRARY}")
