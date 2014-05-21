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

#define CHECK_VALUE(VALUE, SITUATION, ERROR, V, INDEX, OFF)                 \
  ({                                                                        \
    auto v_s = VALUE(SITUATION);                                            \
    std::string error_s = (ERROR);                                          \
    if (error_s.size()) {                                                   \
      ASSERT(get_error(v_s.first), "expect a error", error_s);              \
      CHECK_EQ(*get_error(v_s.first).get(), error_s, "error not expected"); \
    } else {                                                                \
      ASSERT(!get_error(v_s.first), "get a error",                          \
             *get_error(v_s.first).get());                                  \
      CHECK_EQ(get_value(v_s.first), V);                                    \
    }                                                                       \
    CHECK_EQ(v_s.second.position.index, INDEX);                             \
    CHECK_EQ(v_s.second.position.off, OFF);                                 \
  })

#define get_error_str(V)  \
  ({                      \
    ASSERT(get_error(V)); \
    *get_error(V).get();  \
  })

TEST(BasicString) {
  auto value = options_parser::value();
  auto v_s = value(to_situation("a string value", "more ..."));
  CHECK_EQ(v_s.second.position.index, 1);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK_EQ(get_value(v_s.first), "a string value");
  v_s = value(to_situation());
  CHECK_EQ(v_s.second.position.index, 0);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(get_error(v_s.first), "expect a error");
  CHECK_EQ(*get_error(v_s.first).get(), "no arguments rest");
}

TEST(BasicInt) {
  auto value = options_parser::value<int>();
  auto v_s = value(to_situation("13", "14", "15"));
  CHECK_EQ(v_s.second.position.index, 1);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK_EQ(get_value(v_s.first), 13);
  v_s = value(to_situation());
  CHECK_EQ(v_s.second.position.index, 0);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(get_error(v_s.first), "expect a error");
  CHECK_EQ(*get_error(v_s.first).get(), "no arguments rest");
  v_s = value(to_situation("13a", "14"));
  CHECK_EQ(v_s.second.position.index, 0);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(get_error(v_s.first), "expect a error");
  CHECK_EQ(*get_error(v_s.first).get(), "from_str<i>(\"13a\") rest \"a\"");
}

TEST(BasicBool) {
  auto value = options_parser::value<bool>().many();
  auto v_s = value(to_situation("t", "true", "false", "1", "0", "f", "False",
                                "True", "100"));
  CHECK_EQ(v_s.second.position.index, 8);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)),
           test::Container<bool>({1, 1, 0, 1, 0, 0, 0, 1}));
}

TEST(ManyInt) {
  auto value = options_parser::value<int>().many();
  auto v_s = value(to_situation("13", "14", "15"));
  CHECK_EQ(v_s.second.position.index, 3);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)),
           test::Container<int>({13, 14, 15}));
  v_s = value(to_situation());
  CHECK_EQ(v_s.second.position.index, 0);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)), test::Container<int>());
  v_s = value(to_situation("13", "14a", "15"));
  CHECK_EQ(v_s.second.position.index, 1);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)),
           test::Container<int>({13}));

  v_s = options_parser::value<int>().cons(value)(
      to_situation("13", "14", "15", "16a"));
  CHECK_EQ(v_s.second.position.index, 3);
  CHECK_EQ(v_s.second.position.off, 0);
  ASSERT(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)),
           test::Container<int>({13, 14, 15}));

  v_s = options_parser::value<int>().cons(value)(
      to_situation("13", "14a", "15", "16a"));
  CHECK_EQ(v_s.second.position.index, 1);
  CHECK_EQ(v_s.second.position.off, 0);
  ASSERT(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)),
           test::Container<int>({13}));
}

TEST(Gather) {
  auto value = value_gather(options_parser::value<int>(),
                            options_parser::value<std::string>(),
                            options_parser::value<double>());
  auto v_s = value(to_situation("13", "a string", "3.14"));
  CHECK_EQ(v_s.second.position.index, 3);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  static_assert(std::tuple_size<decltype(get_value(v_s.first))>::value == 3,
                "fix me first");
  CHECK_EQ(std::get<0>(get_value(v_s.first)), 13);
  CHECK_EQ(std::get<1>(get_value(v_s.first)), "a string");
  CHECK_EQ(std::get<2>(get_value(v_s.first)), 3.14);

  v_s = value(to_situation("13", "a string", "3.14a"));
  CHECK_EQ(v_s.second.position.index, 0);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK_EQ(get_error_str(v_s.first), "from_str<d>(\"3.14a\") rest \"a\"");
}

TEST(Check) {
  auto value = options_parser::value().check([](std::string n) {
                                               return n.size() && n[0] == '-';
                                             },
                                             "should starts with -");
  auto v_s = value(to_situation("13", "-a"));
  CHECK_EQ(v_s.second.position.index, 0);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK_EQ(get_error_str(v_s.first), "should starts with -");

  v_s = value(to_situation("-13", "-a"));
  CHECK_EQ(v_s.second.position.index, 1);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  CHECK_EQ(get_value(v_s.first), "-13");
}

TEST(Cons) {
  auto vs =
      options_parser::value().cons(options_parser::value().not_option().many());
  auto v_s = vs(to_situation("-first", "and", "all", "rest", "-not option"));
  CHECK_EQ(v_s.second.position.index, 4);
  CHECK_EQ(v_s.second.position.off, 0);
  CHECK(!get_error(v_s.first));
  CHECK_EQ(test::to_container(get_value(v_s.first)),
           (test::Container<std::string>{"-first", "and", "all", "rest"}));
}

}  // namespace options_parser
