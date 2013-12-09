#include "options_parser/options_parser.h"

#include <fstream>

std::vector<double> moments(5);
double g_min;
double g_max;
void add_value(double x) {
  if (!moments.size()) return;
  double a = 1;
  for (size_t i = 0; i < moments.size(); ++i) {
    moments[i] += a;
    a *= x;
  }
  if (moments[0]) {
    g_min = std::min(g_min, x);
    g_max = std::max(g_max, x);
  } else {
    g_min = g_max = x;
  }
}

template <class Vs>
void add_values(const Vs& vs) {
  for (auto v: vs) {
    add_value(v);
  }
}

void add_istream(std::istream *is) {
  while (is->good()) {
    double v;
    (*is) >> v;
    if (is->fail()) {
      if (is->eof()) break;
      is->clear();
      char c;
      (*is) >> c;
      if (is->fail()) break;
      continue;
    }
    add_value(v);
  }
}

double square(double x) { return x * x; }

int main(int argc, char* argv[]) {

  options_parser::Parser app(
      "Calculation and print the stats of values, gathered from command "
      "line arguments, or/and stdin/files.\n\n",
      "\nThis's a toy program to demo options_parser only, but i use it a "
      "lot.");
  app.add_help();
  app.add_parser(options_parser::parser());

  app.add_option("moment", [&](size_t m) {
                             if (m >= moments.size()) {
                               moments.resize(m);
                             }
                           },
                 {"--moment NUM", "Set # of moments"});

  app.add_option("value", [&](double v) {
                            add_value(v);
                            return 1;
                          },
                 {"--value <float value>", "Add value from argument"});

  app.add_option(
      "file", [&](std::string fn) {
                if (fn.size() && fn != "-") {
                  std::ifstream ifs(fn);
                  add_istream(&ifs);
                } else {
                  add_istream(&std::cin);
                }
              },
      {"--file FILE", "Add values from FILE, all non value texts are inogred"});

  app.add_option(options_parser::MATCH_POSITION, [&](std::string s) {
                                                   std::istringstream is(s);
                                                   add_istream(&is);
                                                 },
                 {"<values string>",
                  "Add values from string, all non value texts are ignored"});

  options_parser::ArgvArguments arguments(argc, argv);
  auto parse_result = app.parse({{1, 0}, arguments});

  if (parse_result.error) {
    std::cerr << *parse_result.error.get() << std::endl;
    if (parse_result.error_full) {
      std::cerr << *parse_result.error_full.get() << std::endl;
    }
    return 1;
  }

  for (size_t i = 0; i < moments.size(); ++i) {
    std::cout << "moment " << i << ": " << moments[i] << std::endl;
  }

  double n = moments[0];
  double s = moments[1];
  double ss = moments[2];

  std::cout << "num: " << n << std::endl;
  std::cout << "mean: " << s / n << std::endl;
  std::cout << "min: " << g_min << std::endl;
  std::cout << "max: " << g_max << std::endl;
  std::cout << "stdev: " << std::sqrt(ss * n - square(s)) / n << std::endl;
  std::cout << "sample-dev:" << std::sqrt(ss * n - square(s)) / (n - 1)
            << std::endl;
  std::cout << "sum: " << s << std::endl;
}
