#include <iostream>
#include <cassert>

#include "options_parser/matcher.h"
#include "options_parser/converter.h"
#include "test.h"
#include "test_lib.h"

namespace  options_parser {
namespace {

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

TEST(String) {
  Matcher m("--file file");
  auto mr = m(to_situation("--file", "test.file"));
  CHECK_EQ(mr.start.index, 0);
  CHECK_EQ(mr.start.off, 0);
  CHECK_EQ(mr.situation.position.index, 1);
  CHECK_EQ(mr.situation.position.off, 0);
  CHECK_EQ(mr.priority, MATCH_EXACT);

  mr = m(to_situation("--file-more", "test.file"));
  CHECK_EQ(mr.situation.position.index, 0);
  CHECK_EQ(mr.situation.position.off, 0);
  CHECK_EQ(mr.priority, MATCH_NONE);

  mr = m(to_situation("--fil", "test.file"));
  CHECK_EQ(mr.situation.position.index, 1);
  CHECK_EQ(mr.situation.position.off, 0);
  CHECK_EQ(mr.priority, MATCH_PREFIX);
}

}  // namespace
}  // namespace  options_parser
