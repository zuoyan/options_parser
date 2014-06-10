CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(options_parser)

SET(CTEST_MEMORYCHECK_COMMAND valgrind)
INCLUDE(CTest)

SET(CMAKE_INCLUDE_CURRENT_DIR 1)

SET(CMAKE_CXX_FLAGS "-Wall -std=c++0x")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-inline")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
ENDIF()

ADD_LIBRARY(options_parser SHARED options_parser/options_parser_lib.cc)

ADD_EXECUTABLE(value_test value_test.cc)
ADD_TEST(value_test value_test)

ADD_EXECUTABLE(matcher_test matcher_test.cc)
ADD_TEST(matcher_test matcher_test)

ADD_EXECUTABLE(options_parser_test options_parser_test.cc lib_test.cc)
TARGET_LINK_LIBRARIES(options_parser_test options_parser)
ADD_TEST(options_parser_test options_parser_test)

ADD_EXECUTABLE(apt apt.cc)

ADD_EXECUTABLE(stats stats.cc)

ADD_EXECUTABLE(ls ls.cc)
TARGET_LINK_LIBRARIES(ls options_parser)

ADD_EXECUTABLE(maybe_test maybe_test.cc)
ADD_TEST(maybe_test maybe_test)

ADD_EXECUTABLE(match_from_description_test match_from_description_test.cc)
ADD_TEST(match_from_description_test match_from_description_test)

ADD_EXECUTABLE(git git.cc)
TARGET_LINK_LIBRARIES(git options_parser)

ADD_EXECUTABLE(secure_sharing secure_sharing.cc)

ADD_TEST(secure_sharing_split
  secure_sharing -K 3 secure_sharing secure_sharing.part.[0-5] --split)

ADD_TEST(secure_sharing_combine
  secure_sharing -K 3 secure_sharing.whole secure_sharing.part.[0-5] --combine)
SET_TESTS_PROPERTIES(secure_sharing_combine PROPERTIES DEPENDS secure_sharing_split)

ADD_TEST(secure_sharing_check
  diff secure_sharing secure_sharing.whole)
SET_TESTS_PROPERTIES(secure_sharing_check PROPERTIES DEPENDS secure_sharing_combine)

ADD_EXECUTABLE(circumstance_test circumstance_test.cc)
ADD_TEST(circumstance_test circumstance_test)

ADD_EXECUTABLE(test_test test_test.cc)
ADD_TEST(test_test test_test)
