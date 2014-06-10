#include <iostream>
#include <cassert>

#include "options_parser/options_parser_lib.h"
#include "unistd.h"

#include "test.h"
#include "test_lib.h"

#define CHECK_PARSE_RESULT(PR, ERROR, INDEX, OFF, ...)                      \
  {                                                                         \
    std::string error = ERROR;                                              \
    if (error.size()) {                                                     \
      if (PR.error) {                                                       \
        std::string es = *PR.error.get();                                   \
        ASSERT_EQ(error, es, ##__VA_ARGS__);                                \
      } else {                                                              \
        ASSERT(false, "expect a error", error, ##__VA_ARGS__);              \
      }                                                                     \
    } else {                                                                \
      std::string error_full;                                               \
      if (PR.error_full) error_full = *PR.error_full.get();                 \
      CHECK(!PR.error, "got error", *PR.error.get(), "full=" << error_full, \
            ##__VA_ARGS__);                                                 \
    }                                                                       \
    CHECK_EQ(PR.situation.position.index, INDEX, ##__VA_ARGS__);            \
    CHECK_EQ(PR.situation.position.off, OFF, ##__VA_ARGS__);                \
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
  CHECK_EQ(vi, 13);
  CHECK_PARSE(parser, "--int-value 26 --int 39", "", 4, 0);
  CHECK_EQ(vi, 39);
  CHECK_PARSE(parser, "--int-value 52 --int-values 65", "match-none", 2, 0);
  CHECK_EQ(vi, 52);
  CHECK_PARSE(parser, "--int-value 78a", "take-error", 0, 0);
  std::string vs;
  parser.add_option("--str-value <string value>", &vs, "string value");
  CHECK_PARSE(parser, "--str-value a-string", "", 2, 0);
  CHECK_EQ(vs, "a-string");
  CHECK_PARSE(parser, "--str-value 'a string value'", "", 2, 0);
  CHECK_EQ(vs, "a string value");
  CHECK_PARSE(parser, "--int 81 --str 'a string'", "", 4, 0);
  CHECK_EQ(vi, 81);
  CHECK_EQ(vs, "a string");
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
                                    },
                    "function a b c");
  CHECK_PARSE(parser, "--func 3.14 'b value' 17", "", 4, 0);
  CHECK_EQ(va, 3.14);
  CHECK_EQ(vb, "b value");
  CHECK_EQ(vc, 17);
  CHECK_PARSE(parser, "--func 3.14 'b value'", "take-error", 0, 0);
}

TEST(SubParser) {
  options_parser::Parser parser, sub;
  parser.add_parser(sub);
  std::string sub_value;
  sub.add_option("--opt <value>", &sub_value, "");
  CHECK_PARSE(parser, "--opt 'a sub'", "", 2, 0);
  CHECK_EQ(sub_value, "a sub");
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
  CHECK_EQ(opt_value, "a opt");
  CHECK_EQ(sub_value, "");
}

TEST(HideFromHelp) {
  options_parser::Parser parser;
  parser.add_option(0, nullptr, "exist-in-help");
  parser.add_option(0, nullptr, "not-shown")->hide_from_help();
  std::string s = parser.help_message(10000, 80);
  CHECK_NE(s.find("exist-in-help"), std::string::npos);
  CHECK_EQ(s.find("not-shown"), std::string::npos);
}

TEST(HelpLevel) {
  options_parser::Parser parser;
  parser.add_option(0, nullptr, "a-help");
  parser.add_option(0, nullptr, "b-help")->set_help_level(10);
  std::string s = parser.help_message(9, 80);
  CHECK_NE(s.find("a-help"), std::string::npos);
  CHECK_EQ(s.find("b-help"), std::string::npos);
  s = parser.help_message(10, 80);
  CHECK_NE(s.find("a-help"), std::string::npos);
  CHECK_NE(s.find("b-help"), std::string::npos);
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
  CHECK(c.flag<bool>("escape"), "circumstance", c.to_str());
  CHECK(c.flag<bool>("directory"), "circumstance", c.to_str());
  CHECK(!c.flag<bool>("all"), "circumstance", c.to_str());
  pr = parser.parse_string("-B --color=color-when", pr.situation);
  CHECK(c.flag<bool>("ignore-backups"), "circumstance", c.to_str());
  CHECK(c.flag("color"), "circumstance", c.to_str());
  CHECK_EQ(*c.flag("color"), "color-when", "circumstance",
           c.to_str());
  CHECK(c.flag<bool>("escape"), "circumstance", c.to_str());
}

TEST(MultiMany) {
  options_parser::Parser parser;
  parser.add_option("--func <int>... <str>... -SEP <str>...",
                    value_gather(options_parser::value<int>().many(1),
                                 options_parser::value().not_option().many(1),
                                 options_parser::value(),
                                 options_parser::value().not_option().many(1))
                        .apply(options_parser::check_invoke([&](
                            std::vector<int> vi, std::vector<std::string> vsa,
                            std::string sep, std::vector<std::string> vsb) {
                          CHECK_EQ(test::to_container(vi),
                                   test::Container<int>({1, 3, 5, 7, 9}));
                          CHECK_EQ(test::to_container(vsa),
                                   test::Container<std::string>({
                                       "11a", "12b", "13c"}));
                          CHECK_EQ(sep, "-12");
                          CHECK_EQ(
                              test::to_container(vsb),
                              test::Container<std::string>({"more", "vs"}));
                        })),
                    "complicate function");
  std::string last;
  parser.add_option("--last", &last, "check last");
  CHECK_PARSE(parser,
              "--func 1 3 5 7 9 11a 12b 13c -12 more vs --last last-value", "",
              14, 0);
  CHECK_EQ(last, "last-value");
}

template <class Func>
struct AutoRunner {
  bool active;
  Func func;
  AutoRunner() { active = false; }

  template <class FF>
  AutoRunner(FF&& func)
      : func(std::forward<FF>(func)) {
    active = true;
  }

  AutoRunner(const AutoRunner&) = delete;
  AutoRunner(AutoRunner&) = delete;
  AutoRunner& operator=(const AutoRunner&) = delete;
  AutoRunner& operator=(AutoRunner&&) = delete;

  AutoRunner(AutoRunner&& r) : active(r.active), func(std::move(r.func)) {
    r.active = false;
  }

  ~AutoRunner() {
    if (active) {
      func();
    }
  }
};

struct AddAutoRunner {
  template <class Func>
  AutoRunner<typename std::remove_const<
      typename std::remove_reference<Func>::type>::type>
  operator+(Func&& func) {
    return AutoRunner<typename std::remove_const<
        typename std::remove_reference<Func>::type>::type>{
        std::forward<Func>(func)};
  }
};

#define DEFER DEFER_(__COUNTER__)
#define DEFER_(COUNTER) auto PP_CAT(auto_runner_, COUNTER) = AddAutoRunner{} + [&]()

struct TempFile {
  TempFile(const std::string& tmp) {
    tmp_ = tmp;
    fd_ = mkstemp(&tmp_[0]);
  }

  TempFile(const std::string& tmp, int suffix) {
    tmp_ = tmp;
    fd_ = mkstemps(&tmp_[0], suffix);
  }

  TempFile(const TempFile&) = delete;

  TempFile(TempFile&& r) {
    tmp_ = r.tmp_;
    fd_ = r.fd_;
    r.fd_ = -1;
  }

  ~TempFile() {
    if (fd_ != -1) {
      unlink(tmp_.c_str());
      close(fd_);
    }
  }

  void SetContent(const std::string& content) {
    int r = ftruncate(fd_, 0);
    if (r != 0) {
      std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
      abort();
    }
    auto s = write(fd_, content.data(), content.size());
    if (s != static_cast<ssize_t>(content.size())) {
      std::cerr << "write failed: " << strerror(errno) << std::endl;
    }
  }

  TempFile& operator=(TempFile&& r) {
    if (this != &r) {
      this->~TempFile();
      new (this) TempFile(std::move(r));
    }
    return *this;
  }

  std::string tmp_;
  int fd_;
};

TEST(ConfigFile) {
  options_parser::Parser app;
  app.add_option("--conf-file <config file>",
                 &options_parser::take_config_file,
                 "conf from file");
  std::string va, vb;
  app.add_option("--func A B", [&](std::string a, std::string b) {
                                 if (a != b) {
                                   return "should equal";
                                 }
                                 va = a;
                                 vb = b;
                                 return "";
                               },
                 "func-doc");
  int vi = 13;
  app.add_option("--vi INT", [&](int i) {
                               if (i % 100 != 13) {
                                 return "bad value";
                               }
                               vi = i;
                               return "";
                             },
                 "vi-doc");
  TempFile tf("options_parser_test.XXXXXX.conf", 5);
  ASSERT_GE(tf.fd_, 0, "mkstemps failed:", strerror(errno));
  std::string content =
      "--vi 1013 --func str-file-a str-file-a\n"
      "--func str-file \\\n"
      " str-file\n"
      "# ignored line\n"
      "--vi 1113";
  tf.SetContent(content);
  CHECK_PARSE(app,
              "--vi 113 --func str str --conf-file " + tf.tmp_, "",
              7, 0);
  CHECK_EQ(vi, 1113);
  CHECK_EQ(va, vb);
  CHECK_EQ(va, "str-file");
  tf = TempFile("options_parser_test.XXXXXX.conf", 5);
  content =
      "--vi 1213 --func str-0 str-0\n"
      "--func str-1\n"
      "str-2\n";
  tf.SetContent(content);
  CHECK_PARSE(app, "--conf-file " + tf.tmp_, "take-error", 0, 0);
  CHECK_EQ(vi, 1213);
  CHECK_EQ(va, vb);
  CHECK_EQ(va, "str-0");
}
