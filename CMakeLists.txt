# On macOS, there is a conflict between MacPorts OpenCV compiled using the
# Command Line Tools and XCode that might be installed on the user's computer.
# We attempt to ensure a consistent compilation experience by pointing CMake to
# the Command Line Tools instead of the XCode headers. This has to be set prior
# to the first project() command (see
# https://cmake.org/cmake/help/latest/variable/CMAKE_OSX_SYSROOT.html for more
# details).
if(APPLE)
    set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk)
endif()

project(faceAnalyzer)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/tools/cmake/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

cmake_policy(SET CMP0074 NEW)

add_subdirectory(external/OpenFace)

add_subdirectory(src)
