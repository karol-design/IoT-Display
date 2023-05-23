# This is a top-level project CMakeLists.txt file.

cmake_minimum_required(VERSION 3.5)

# Set component directory
set(EXTRA_COMPONENT_DIRS "components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(board-fw)
