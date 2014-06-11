#include <iostream>
#include <cassert>

#include "options_parser/matcher.h"
#include "test.h"
#include "test_lib.h"

namespace options_parser {

namespace {

std::ostream& operator<<(std::ostream& os, const MatchDescription& md) {
  os << "{ doc:" << md.doc << ", "
     << "name:" << md.name << ", "
     << "opts:" << join(md.opts, " | ") << ", "
     << "num. args:" << md.num_args << ", "
     << "optinal?:" << md.is_arg_optional << " }";
  return os;
}

}  // namespace

TEST(Basic) {
  MatchDescription md{"-f, --file FILE"};
  CHECK_EQ(md.doc, "-f, --file FILE");
  CHECK_EQ(md.name, "--file");
  CHECK_EQ(test::Container<std::string>(md.opts),
           test::Container<std::string>({"-f", "--file"}));
  CHECK_EQ(md.num_args, 1);
  CHECK_EQ(md.is_arg_optional, false);
}

TEST(BasicEqual) {
  MatchDescription md{"-f, --file=FILE"};
  CHECK_EQ(md.doc, "-f, --file=FILE");
  CHECK_EQ(md.name, "--file");
  CHECK_EQ(test::Container<std::string>(md.opts),
           test::Container<std::string>({"-f", "--file"}));
  CHECK_EQ(md.num_args, 1);
  CHECK_EQ(md.is_arg_optional, false);
}

TEST(BasicOptional) {
  MatchDescription md{"-f, --file [FILE]"};
  CHECK_EQ(md.doc, "-f, --file [FILE]");
  CHECK_EQ(md.name, "--file");
  CHECK_EQ(test::Container<std::string>(md.opts),
           test::Container<std::string>({"-f", "--file"}));
  CHECK_EQ(md.num_args, 1);
  CHECK_EQ(md.is_arg_optional, true);
}

TEST(Bracket) {
  MatchDescription md{"--remotes[=<pattern>]"};
  CHECK_EQ(md.doc, "--remotes[=<pattern>]");
  CHECK_EQ(md.name, "--remotes");
  CHECK_EQ(test::Container<std::string>(md.opts),
           test::Container<std::string>({"--remotes"}));
  CHECK_EQ(md.num_args, 1);
  CHECK_EQ(md.is_arg_optional, true);
};

}  // namespace options_parser
