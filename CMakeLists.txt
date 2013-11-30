CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(options_parser)

SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++0x -O0 -ftemplate-backtrace-limit=0")

ADD_EXECUTABLE(options_parser_test options_parser_test.cc)

ADD_EXECUTABLE(git git.cc)
