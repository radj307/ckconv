# ckconv/
cmake_minimum_required (VERSION 3.20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/307lib/307modules")

include(VersionTag)
set(ENV{ckconv_VERSION} "0.0.0")

project("ckconv" VERSION "${ckconv_VERSION}" LANGUAGES CXX)

add_subdirectory("307lib")
add_subdirectory("ckconv")
