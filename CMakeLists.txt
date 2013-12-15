CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(options_parser)

SET(CMAKE_INCLUDE_CURRENT_DIR 1)

SET(CMAKE_CXX_FLAGS "-Wall -std=c++0x -O0 -g -fno-inline")

ADD_LIBRARY(options_parser SHARED options_parser/options_parser_lib.cc)

ADD_EXECUTABLE(options_parser_test options_parser_test.cc lib_test.cc)
TARGET_LINK_LIBRARIES(options_parser_test options_parser)

ADD_EXECUTABLE(apt apt.cc)

ADD_EXECUTABLE(stats stats.cc)

ADD_EXECUTABLE(ls ls.cc)
TARGET_LINK_LIBRARIES(ls options_parser)

ADD_EXECUTABLE(maybe_test maybe_test.cc)

ADD_EXECUTABLE(match_from_description_test match_from_description_test.cc)

ADD_EXECUTABLE(git git.cc)
TARGET_LINK_LIBRARIES(git options_parser)

ADD_EXECUTABLE(secure_sharing secure_sharing.cc)
