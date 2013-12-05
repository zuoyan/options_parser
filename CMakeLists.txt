CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(options_parser)

SET(CMAKE_INCLUDE_CURRENT_DIR 1)

SET(CMAKE_CXX_FLAGS "-Wall -std=c++0x")

ADD_LIBRARY(options_parser SHARED options_parser/options_parser_lib.cc)

ADD_EXECUTABLE(options_parser_test options_parser_test.cc lib_test.cc)
TARGET_LINK_LIBRARIES(options_parser_test options_parser)

ADD_EXECUTABLE(apt apt.cc)
