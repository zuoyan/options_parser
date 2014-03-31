#include "options_parser/options_parser.h"

#include <fstream>
#include <iomanip>

std::vector<double> g_sums(5, 0.0);
double g_min;
double g_max;
bool g_print_value = false;

void add_value(double x) {
  if (g_print_value) {
    std::cout << "value:" << x << std::endl;
  }
  if (!g_sums.size()) return;
  double pgs = g_sums[0];
  double a = 1;
  for (size_t i = 0; i < g_sums.size(); ++i) {
    g_sums[i] += a;
    a *= x;
  }
  if (pgs) {
    g_min = std::min(g_min, x);
    g_max = std::max(g_max, x);
  } else {
    g_min = g_max = x;
  }
}

template <class Vs>
void add_values(const Vs& vs) {
  for (auto v : vs) {
    add_value(v);
  }
}

void add_istream(std::istream* is) {
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

size_t combine(size_t n, size_t i) {
  size_t p = 1, q = 1;
  for (size_t j = 1; j <= i; ++j) {
    p *= j;
  }
  for (size_t j = 1; j <= i; ++j) {
    q *= (n - j + 1);
  }
  return q / p;
}

double pown(double x, size_t c) {
  double a = 1;
  while (c) {
    if (c & 1) {
      a *= x;
    }
    x *= x;
    c >>= 1;
  }
  return a;
}

int main(int argc, char* argv[]) {
  std::cout << std::setprecision(10);

  options_parser::Parser app(
      "Calculation and print the stats of values, gathered from command "
      "line arguments, or/and stdin/files.\n\n",
      "\nThis's a toy program to demo options_parser only, but i use it a "
      "lot.");
  app.add_help();
  app.add_parser(options_parser::parser());
  app.add_option("--print-value", [&]() { g_print_value = true; },
                 "print out every value");

  app.add_option("--moment NUM", [&](size_t m) {
                                   if (m >= g_sums.size()) {
                                     g_sums.resize(m);
                                   }
                                 },
                 "Set # of moments");

  app.add_option("--value <float value>", [&](double v) {
                                            add_value(v);
                                            return 1;
                                          },
                 "Add value from argument");

  app.add_option("--file FILE", [&](std::string fn) {
                                  if (fn.size() && fn != "-") {
                                    std::ifstream ifs(fn);
                                    add_istream(&ifs);
                                  } else {
                                    add_istream(&std::cin);
                                  }
                                },
                 "Add values from FILE, all non value texts are inogred");

  app.add_option(options_parser::MATCH_POSITION, [&](std::string s) {
                                                   std::istringstream is(s);
                                                   add_istream(&is);
                                                 },
                 {"<values string>",
                  "Add values from string, all non value texts are ignored"});

  app.parse(argc, argv).check_print();

  double n = g_sums[0];
  double s = g_sums[1];
  double ss = g_sums[2];

  std::cout << "num: " << n << std::endl;
  std::cout << "mean: " << s / n << std::endl;
  std::cout << "min: " << g_min << std::endl;
  std::cout << "max: " << g_max << std::endl;
  std::cout << "stdev: " << std::sqrt(ss * n - square(s)) / n << std::endl;
  std::cout << "sample-dev:" << std::sqrt(ss * n - square(s)) / (n - 1)
            << std::endl;
  std::cout << "sum: " << s << std::endl;

  std::vector<double> moments(g_sums.size(), 0);
  for (size_t i = 0; i < g_sums.size(); ++i) {
    moments[i] = g_sums[i] / n;
  }

  std::vector<double> central_moments(g_sums.size(), 0);
  central_moments[0] = 1;
  central_moments[1] = 0;
  for (size_t i = 2; i < moments.size(); ++i) {
    for (size_t j = 0; j <= i; ++j) {
      double x = combine(i, j) * moments[j] * pown(-moments[1], i - j);
      central_moments[i] += x;
    }
  }

  for (size_t i = 0; i < moments.size(); ++i) {
    std::cout << "origin-moment " << i << ": " << moments[i] << std::endl;
  }

  for (size_t i = 0; i < central_moments.size(); ++i) {
    std::cout << "central-moment " << i << ": " << central_moments[i]
              << std::endl;
  }

  std::vector<double> kumulants(g_sums.size(), 0);
  kumulants[0] = 0;
  for (size_t i = 2; i < g_sums.size(); ++i) {
    kumulants[i] = moments[i];
    for (size_t j = 1; j < i; ++j) {
      kumulants[i] -= combine(i - 1, j - 1) * kumulants[j] * moments[i - j];
    }
  }

  for (size_t i = 0; i < kumulants.size(); ++i) {
    std::cout << "kumulant " << i << ": " << kumulants[i] << std::endl;
  }
}
