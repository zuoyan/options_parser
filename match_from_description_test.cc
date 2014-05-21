#include <iostream>
#include <cassert>

#include "options_parser/matcher.h"
#include "test.h"
#include "test_lib.h"

namespace options_parser {

namespace {

std::ostream& operator<<(std::ostream& os, const MatchFromDescription& mfd) {
  os << "{ doc:" << mfd.doc << ", "
     << "name:" << mfd.name << ", "
     << "opts:" << join(mfd.opts, " | ") << ", "
     << "num. args:" << mfd.num_args << ", "
     << "optinal?:" << mfd.is_arg_optional << ", "
     << "raw:" << mfd.is_raw << " }";
  return os;
}

}  // namespace

TEST(Basic) {
  MatchFromDescription mfd{"-f, --file FILE"};
  CHECK_EQ(mfd.doc, "-f, --file FILE");
  CHECK_EQ(mfd.name, "file");
  CHECK_EQ(test::Container<std::string>(mfd.opts),
           test::Container<std::string>({"f", "file"}));
  CHECK_EQ(mfd.num_args, 1);
  CHECK_EQ(mfd.is_arg_optional, false);
  CHECK_EQ(mfd.is_raw, false);
}

TEST(BasicEqual) {
  MatchFromDescription mfd{"-f, --file=FILE"};
  CHECK_EQ(mfd.doc, "-f, --file=FILE");
  CHECK_EQ(mfd.name, "file");
  CHECK_EQ(test::Container<std::string>(mfd.opts),
           test::Container<std::string>({"f", "file"}));
  CHECK_EQ(mfd.num_args, 1);
  CHECK_EQ(mfd.is_arg_optional, false);
  CHECK_EQ(mfd.is_raw, false);
}

TEST(BasicOptional) {
  MatchFromDescription mfd{"-f, --file [FILE]"};
  CHECK_EQ(mfd.doc, "-f, --file [FILE]");
  CHECK_EQ(mfd.name, "file");
  CHECK_EQ(test::Container<std::string>(mfd.opts),
           test::Container<std::string>({"f", "file"}));
  CHECK_EQ(mfd.num_args, 1);
  CHECK_EQ(mfd.is_arg_optional, true);
  CHECK_EQ(mfd.is_raw, false);
}

TEST(Bracket) {
  MatchFromDescription mfd{"--remotes[=<pattern>]"};
  CHECK_EQ(mfd.doc, "--remotes[=<pattern>]");
  CHECK_EQ(mfd.name, "remotes");
  CHECK_EQ(test::Container<std::string>(mfd.opts),
           test::Container<std::string>({"remotes"}));
  CHECK_EQ(mfd.num_args, 1);
  CHECK_EQ(mfd.is_arg_optional, true);
  CHECK_EQ(mfd.is_raw, false);
};

TEST(BracketOnly) {
  MatchFromDescription mfd{"|<path>"};
  CHECK_EQ(mfd.doc, "<path>");
  CHECK_EQ(mfd.name, "<path>");
  CHECK_EQ(test::Container<std::string>(mfd.opts),
           test::Container<std::string>({"<path>"}));
  CHECK_EQ(mfd.num_args, 0);
  CHECK_EQ(mfd.is_arg_optional, false);
  CHECK_EQ(mfd.is_raw, true);
};

TEST(Raw) {
  MatchFromDescription mfd{"|file|input-file"};
  CHECK_EQ(mfd.doc, "file, input-file");
  CHECK_EQ(mfd.name, "input-file");
  CHECK_EQ(test::Container<std::string>(mfd.opts),
           test::Container<std::string>({"file", "input-file"}));
  CHECK_EQ(mfd.num_args, 0);
  CHECK_EQ(mfd.is_arg_optional, false);
  CHECK_EQ(mfd.is_raw, true);
}

}  // namespace options_parser
