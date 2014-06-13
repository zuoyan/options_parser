#include <iostream>

#include "options_parser/options_parser.h"

int main(int argc, char* argv[]) {
  int vi;
  std::string vs;
  double vd;

  options_parser::Parser app("usage and info", "further info ...");
  app.add_parser(options_parser::parser());  // add default global parser
  app.add_help();
  app.add_option("--int NUM", &vi, "assigned int value");
  app.add_option("--str STRING", &vs, "assigned string value");
  app.add_option(
      "--func <int-arg> <string-arg> <double-arg>",
      [&](int i, std::string s, double d) {
        vi = i;
        vs = s;
        vd = d;
      },
      "call the function over arguments, assign vi, vs, and vd at once.");
  app.add_option("--config-file FILE",
                 options_parser::config_file().ignore_value(),
                 "load config file, which just contains command arguments");

  app.add_option("--verbose-config-file FILE",
                 options_parser::config_file().apply([&](std::string filename) {
                   std::cerr << "loaded config file " << filename << std::endl;
                 }),
                 "load config file and print");
  app.parse(argc, argv).check_print();  // if has a error, print and exit

  std::cerr << "vi " << vi << std::endl;
  std::cerr << "vs " << vs << std::endl;
  std::cerr << "vd " << vd << std::endl;

  return 0;
}
