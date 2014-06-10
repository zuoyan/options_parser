#include <iostream>
#include <cassert>

#include "options_parser/options_parser.h"

#include "test.h"
#include "test_lib.h"

namespace options_parser {

template <class... S>
VectorStringArguments to_argv(const S&... s) {
  return VectorStringArguments(std::vector<string>{s...});
}

template <class... S>
Situation to_situation(const S&... s) {
  Situation situation;
  situation.args = to_argv(s...);
  situation.position.index = 0;
  situation.position.off = 0;
  return situation;
}

#define CHECK_GET_ERROR(V)                 \
  ({                                       \
    ASSERT(is_error(V), "expect a error"); \
    get_error(V);                          \
  })

#define CHECK_GET_VALUE(V)                         \
  ({                                               \
    ASSERT(is_ok(V), "get a error", get_error(V)); \
    get_error(V);                                  \
  })

#define CHECK_VALUE(VALUE, SITUATION, ERROR, V, INDEX, OFF)                \
  ({                                                                       \
    auto v_s = VALUE(SITUATION);                                           \
    std::string error_s = (ERROR);                                         \
    if (error_s.size()) {                                                  \
      CHECK_EQ(CHECK_GET_ERROR(v_s.first), error_s, "error not expected"); \
    } else {                                                               \
      CHECK_MATCH(CHECK_GET_VALUE(v_s.first), V);                          \
    }                                                                      \
    CHECK_EQ(v_s.second.position.index, INDEX);                            \
    CHECK_EQ(v_s.second.position.off, OFF);                                \
  })

TEST(BasicChar) {
  auto value = options_parser::value<char>();
  auto v_s = value(to_situation("a", "b"));
  CHECK_VALUE(value, to_situation("a", "b"), "", 'a', 1, 0);
}

TEST(BasicString) {
  auto value = options_parser::value();
  CHECK_VALUE(value, to_situation("a string value", "more ..."), "",
              "a string value", 1, 0);
  CHECK_VALUE(value, to_situation(), "no arguments rest", "", 0, 0);
}

TEST(BasicInt) {
  auto value = options_parser::value<int>();
  CHECK_VALUE(value, to_situation("13", "14", "15"), "", 13, 1, 0);
  CHECK_VALUE(value, to_situation(), "no arguments rest", -1, 0, 0);
  CHECK_VALUE(value, to_situation("13a", "14"),
              "from_str<i>(\"13a\") rest \"a\"", -1, 0, 0);
}

TEST(BasicBool) {
  auto value = options_parser::value<bool>().many();
  CHECK_VALUE(
      value,
      to_situation("t", "true", "false", "1", "0", "f", "False", "True", "100"),
      "", test::ContainerEQ(std::vector<bool>{1, 1, 0, 1, 0, 0, 0, 1}), 9, 0);
}

TEST(ManyInt) {
  auto value = options_parser::value<int>().many();
  CHECK_VALUE(value, to_situation("13", "14", "15"), "",
              test::ContainerEQ(std::vector<int>{13, 14, 15}), 3, 0);
  CHECK_VALUE(value, to_situation(), "", test::ContainerEQ(std::vector<int>{}),
              0, 0);
  CHECK_VALUE(value, to_situation("13", "14a", "15"), "",
              test::ContainerEQ(std::vector<int>{{13}}), 1, 0);
  CHECK_VALUE(options_parser::value<int>().cons(value),
              to_situation("13", "14", "15", "16a"), "",
              test::ContainerEQ(std::vector<int>{13, 14, 15}), 3, 0);
  CHECK_VALUE(options_parser::value<int>().cons(value),
              to_situation("13", "14a", "15", "16a"), "",
              test::ContainerEQ(std::vector<int>{{13}}), 1, 0);
}

TEST(Gather) {
  auto value = value_gather(options_parser::value<int>(),
                            options_parser::value<std::string>(),
                            options_parser::value<double>());
  CHECK_VALUE(value, to_situation("13", "a string", "3.14"), "",
              test::TupleEQ(13, "a string", 3.14), 3, 0);
  CHECK_VALUE(value, to_situation("13", "a string", "3.14a"),
              "from_str<d>(\"3.14a\") rest \"a\"", test::AnyFalse(), 0, 0);
}

TEST(Check) {
  auto value = options_parser::value().check([](std::string n) {
                                               return n.size() && n[0] == '-';
                                             },
                                             "should starts with -");
  CHECK_VALUE(value, to_situation("13", "-a"),
              "should starts with -", "", 0, 0);
  CHECK_VALUE(value, to_situation("-13", "-a"), "", "-13", 1, 0);
}

TEST(Cons) {
  auto vs =
      options_parser::value().cons(options_parser::value().not_option().many());
  CHECK_VALUE(vs, to_situation("-first", "and", "all", "rest", "-not option"),
              "", test::ContainerEQ(
                      std::vector<std::string>{"-first", "and", "all", "rest"}),
              4, 0);
}

}  // namespace options_parser
