#include <iostream>
#include <cassert>

#include "options_parser/options_parser_lib.h"
#include "clog/clog.hpp"

struct Test {
  virtual void Run() = 0;
};

std::map<std::string, std::unique_ptr<Test>> tests;

struct test_register {
  template <class T>
  test_register(const std::string& name, const T& v) {
    tests[name].reset(new T(v));
  }
};

#define TEST(NAME)                                                          \
  struct PP_CAT(Test, NAME) : Test {                                        \
    void Run() override;                                                    \
  };                                                                        \
  test_register PP_CAT(test_register_, NAME)(#NAME, PP_CAT(Test, NAME) {}); \
  void PP_CAT(Test, NAME)::Run()

#define CHECK_PARSE_RESULT(PR, ERROR, INDEX, OFF, ...)                    \
  {                                                                       \
    std::string error = ERROR;                                            \
    if (error.size()) {                                                   \
      if (PR.error) {                                                     \
        std::string es = *PR.error.get();                                 \
        CLOG_CHECK_EQ(error, es, ##__VA_ARGS__);                          \
      } else {                                                            \
        CLOG(FATAL, "expect a error", error, ##__VA_ARGS__);              \
      }                                                                   \
    } else {                                                              \
      CLOG_CHECK(!PR.error, "got error", *PR.error.get(), ##__VA_ARGS__); \
    }                                                                     \
    CLOG_CHECK_EQ(PR.situation.position.index, INDEX, ##__VA_ARGS__);     \
    CLOG_CHECK_EQ(PR.situation.position.off, OFF, ##__VA_ARGS__);         \
  }

#define CHECK_PARSE(PARSER, STR, ERROR, INDEX, OFF, ...)          \
  {                                                               \
    auto pr = PARSER.parse_string(STR);                           \
    CHECK_PARSE_RESULT(pr, ERROR, INDEX, OFF, "when parse", STR); \
  }

TEST(Basic) {
  options_parser::Parser parser;
  CHECK_PARSE(parser, "--help", "match-none", 0, 0);
  CHECK_PARSE(parser, "", "", 0, 0);
  int vi = 0;
  parser.add_option("--int-value <integer value>", &vi, "integer value");
  CHECK_PARSE(parser, "--int-value 13", "", 2, 0);
  CLOG_CHECK_EQ(vi, 13);
  CHECK_PARSE(parser, "--int-value 26 --int 39", "", 4, 0);
  CLOG_CHECK_EQ(vi, 39);
  CHECK_PARSE(parser, "--int-value 52 --int-values 65", "match-none", 2, 0);
  CLOG_CHECK_EQ(vi, 52);
  CHECK_PARSE(parser, "--int-value 78a", "take-error", 0, 0);
  std::string vs;
  parser.add_option("--str-value <string value>", &vs, "string value");
  CHECK_PARSE(parser, "--str-value a-string", "", 2, 0);
  CLOG_CHECK_EQ(vs, "a-string");
  CHECK_PARSE(parser, "--str-value 'a string value'", "", 2, 0);
  CLOG_CHECK_EQ(vs, "a string value");
  CHECK_PARSE(parser, "--int 81 --str 'a string'", "", 4, 0);
  CLOG_CHECK_EQ(vi, 81);
  CLOG_CHECK_EQ(vs, "a string");
}

TEST(Function) {
  options_parser::Parser parser;
  double va;
  std::string vb;
  int vc;
  parser.add_option("--func A B C", [&](double a, std::string b, int c) {
      va = a;
      vb = b;
      vc = c;
    }, "function a b c");
  CHECK_PARSE(parser, "--func 3.14 'b value' 17", "", 4, 0);
  CLOG_CHECK_EQ(va, 3.14);
  CLOG_CHECK_EQ(vb, "b value");
  CLOG_CHECK_EQ(vc, 17);
  CHECK_PARSE(parser, "--func 3.14 'b value'", "take-error", 0, 0);
}

TEST(SubParser) {
  options_parser::Parser parser, sub;
  parser.add_parser(sub);
  std::string sub_value;
  sub.add_option("--opt <value>", &sub_value, "");
  CHECK_PARSE(parser, "--opt 'a sub'", "", 2, 0);
  CLOG_CHECK_EQ(sub_value, "a sub");
  std::string opt_value;
  parser.add_option("--opt <value>", &opt_value, "");
  CHECK_PARSE(parser, "--opt 'a opt'", "match-multiple", 0, 0);
}

TEST(SubParserPriority) {
  options_parser::Parser parser, sub;
  parser.add_parser(sub, -1);
  std::string sub_value, opt_value;
  sub.add_option("--opt <value>", &sub_value, "");
  parser.add_option("--opt <value>", &opt_value, "");
  CHECK_PARSE(parser, "--opt 'a opt'", "", 2, 0);
  CLOG_CHECK_EQ(opt_value, "a opt");
  CLOG_CHECK_EQ(sub_value, "");
}

TEST(Flags) {
  options_parser::Parser parser;
  std::string flags = R"FLAGS(
-a, --all                  do not ignore entries starting with .
-A, --almost-all           do not list implied . and ..
    --author               with -l, print the author of each file
-b, --escape               print C-style escapes for nongraphic characters
    --block-size=SIZE      scale sizes by SIZE before printing them.  E.g.,
                             `--block-size=M' prints sizes in units of
                             1,048,576 bytes.  See SIZE format below.
-B, --ignore-backups       do not list implied entries ending with ~
-c                         with -lt: sort by, and show, ctime (time of last
                             modification of file status information)
                             with -l: show ctime and sort by name
                             otherwise: sort by ctime, newest first
-C                         list entries by columns
    --color[=WHEN]         colorize the output.  WHEN defaults to `always'
                             or can be `never' or `auto'.  More info below
-d, --directory            list directory entries instead of contents,
                             and do not dereference symbolic links
-D, --dired                generate output designed for Emacs' dired mode
)FLAGS";
  parser.add_flags_lines(options_parser::split(flags, "\n"));
  CHECK_PARSE(parser,
              "--all -a -A --almost-all --author -b --escape --block-size size "
              "-B --ignore-backups -c -C --color=when -d --dired",
              "", 16, 0);
  auto pr = parser.parse_string("-b --directory");
  auto c = pr.situation.circumstance;
  CLOG_CHECK(c.get("/flag/escape"), "circumstance", c.to_str());
  CLOG_CHECK(c.get("/flag/directory"), "circumstance", c.to_str());
  CLOG_CHECK(!c.get("/flag/all"), "circumstance", c.to_str());
  pr = parser.parse_string("-B --color=color-when", c);
  CLOG_CHECK(c.get("/flag/ignore-backups"), "circumstance", c.to_str());
  CLOG_CHECK(c.get("/flag/color"), "circumstance", c.to_str());
  CLOG_CHECK_EQ(*c.get<std::string>("/flag/color"), "color-when",
                "circumstance", c.to_str());
  CLOG_CHECK(c.get("/flag/escape"), "circumstance", c.to_str());
}

int main(int argc, char* argv[]) {
  for (auto && n_t : tests) {
    CLOG(INFO, "test", n_t.first, "...");
    n_t.second->Run();
    CLOG(INFO, "done", n_t.first);
  }
  return 0;
}
