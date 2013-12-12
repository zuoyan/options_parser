#include <iostream>
#include <cassert>

#include "options_parser/matcher.h"

using namespace options_parser;

int main(int argc, char *argv[]) {
  MatchFromDescription mfd(argv[1]);
  std::cerr << "doc:" << mfd.doc << std::endl;
  std::cerr << "name:" << mfd.name << std::endl;
  std::cerr << "opts:" << join(mfd.opts, " | ") << std::endl;
  std::cerr << "num. args:" << mfd.num_args << std::endl;
  std::cerr << "optinal?:" << mfd.is_arg_optional << std::endl;
  std::cerr << "raw:" << mfd.is_raw << std::endl;
  return 0;
}
