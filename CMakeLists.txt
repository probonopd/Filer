cmake_minimum_required(VERSION 3.5)

project(Filer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "/System/Filer.app")

set(CMAKE_CXX_COMPILER clang++)

add_subdirectory(src)
