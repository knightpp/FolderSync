cmake_minimum_required(VERSION 3.14)
project(folder_sync)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native")


# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native")
    message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_DEBUG}")
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_RELEASE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_RELEASE}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${PROJECT_DIR}/${OUTPUT_RELEASE}")
endif()

add_executable(folder_sync main.cpp ./cxxopts/include/cxxopts.hpp Duration.h Directory.h File.h Folder.cpp Folder.h SyncFolder.cpp SyncFolder.h)
