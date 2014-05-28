#ifndef FILE_A1F24FBA_5A7E_4271_B00E_8450C8459AF6_H
#define FILE_A1F24FBA_5A7E_4271_B00E_8450C8459AF6_H
#include "clog/pp.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

namespace test {

enum ResultCode { SUCC = 0, FAIL, CRITICAL, EXCEPTION };

struct Noodle {
  const char* file_;
  int line_;
  int result_;
  std::string message_;

  Noodle(const Noodle&) = default;

  Noodle(const char* file, int line, int result, std::string message)
      : file_(file), line_(line), result_(result), message_(message) {}
};

struct Test {
  std::string name_;
  const char* file_;
  int line_;
  std::vector<Noodle> noodles_;

  inline bool succ() {
    for (const auto& n : noodles_) {
      if (n.result_ != ResultCode::SUCC) return false;
    }
    return true;
  }

  virtual void RunRoutine() = 0;

  virtual bool Run();

  virtual void AddNoodle(const char* file, int line, int result,
                         std::string message);

  static constexpr const char* color_red() { return "\x1b[31m"; }
  static constexpr const char* color_reset() { return "\x1b[0m"; }
  static constexpr const char* color_green() { return "\x1b[32m"; }
};

struct TestCritical : std::runtime_error {
  using std::runtime_error::runtime_error;
};

void Test::AddNoodle(const char* file, int line, int result, std::string message) {
  noodles_.emplace_back(file, line, result, message);
  if (result == ResultCode::CRITICAL) {
    throw TestCritical("test-critical");
  }
}

bool Test::Run() {
  std::cerr << "[test]\t" << name_ << " " << file_ << "@" << line_
            << " ..." << std::endl;
  noodles_.clear();
  try {
    RunRoutine();
  } catch(const TestCritical&) {
    // pass
  } catch(const std::exception& e) {
    std::cerr << "find uncaught exception '" << e.what() << "' in test "
              << name_ << "[" << file_ << "@" << line_ << "]" << std::endl;
    noodles_.emplace_back(file_, line_, ResultCode::EXCEPTION, e.what());
    return false;
  }
  bool r = succ();
  if (r) {
    std::cerr << color_green() << "[pass]\t" << color_reset() << name_
              << std::endl;
  } else {
    std::cerr << color_red() << "[fail]\t" << color_reset() << name_
              << std::endl;
  }
  return r;
}

inline std::vector<std::unique_ptr<Test>>& tests() {
  static std::vector<std::unique_ptr<Test>> ts;
  return ts;
}

struct test_register {
  template <class T>
  test_register(const T& v) {
    tests().emplace_back(new T(v));
  }
};

#define TEST(NAME)                                                           \
  struct PP_CAT(TEST, NAME) : ::test::Test {                                 \
    PP_CAT(TEST, NAME)() {                                                   \
      name_ = #NAME;                                                         \
      file_ = __FILE__;                                                      \
      line_ = __LINE__;                                                      \
    }                                                                        \
    void RunRoutine() override;                                              \
  };                                                                         \
  ::test::test_register PP_CAT(test_register_, NAME)(PP_CAT(TEST, NAME) {}); \
  void PP_CAT(TEST, NAME)::RunRoutine()

#define TEST_MESSAGE(OS, X) OS << ' ' << X;

#define TEST_NOODLE(COUNTER, FILE, LINE, RESULT, ...)                       \
  if ((RESULT) != ::test::ResultCode::SUCC) {                               \
    std::ostringstream PP_CAT(test_oss_, COUNTER);                          \
    PP_FOREACH(TEST_MESSAGE, PP_CAT(test_oss_, COUNTER), ##__VA_ARGS__);    \
    std::cerr << color_red() << "failed" << color_reset() << " in " << FILE \
              << "@" << LINE << ":";                                        \
    std::cerr << PP_CAT(test_oss_, COUNTER).str() << std::endl;             \
    AddNoodle(FILE, LINE, (RESULT), PP_CAT(test_oss_, COUNTER).str());      \
  } else {                                                                  \
    AddNoodle(FILE, LINE, (RESULT), "succ");                                \
  }

#define CHECK_(COUNTER, FILE, LINE, COND, FAIL_CODE, ...)             \
  int PP_CAT(test_result_, COUNTER) =                                 \
      (COND) ? ::test::ResultCode::SUCC : FAIL_CODE;                  \
  TEST_NOODLE(__COUNTER__, FILE, LINE, PP_CAT(test_result_, COUNTER), \
              ##__VA_ARGS__);

#define CHECK_FAIL_CODE(COND, FAIL_CODE, ...) \
  CHECK_(__COUNTER__, __FILE__, __LINE__, (COND), FAIL_CODE, ##__VA_ARGS__)

#define CHECK(COND, ...) \
  CHECK_FAIL_CODE(COND, ::test::ResultCode::FAIL, #COND, ##__VA_ARGS__)

#define ASSERT(COND, ...) \
  CHECK_FAIL_CODE(COND, ::test::ResultCode::CRITICAL, #COND, ##__VA_ARGS__)

#define CHECK_BIN_FAIL_CODE(L, BIN, R, FAIL_CODE, ...) \
  CHECK_FAIL_CODE(L BIN R, FAIL_CODE, "(" << L, #BIN, R << ")", ##__VA_ARGS__)

#define CHECK_EQ(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, ==, B, ::test::ResultCode::FAIL, ##__VA_ARGS__)
#define ASSERT_EQ(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, ==, B, ::test::ResultCode::CRITICAL, ##__VA_ARGS__)

#define CHECK_NE(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, !=, B, ::test::ResultCode::FAIL, ##__VA_ARGS__)
#define ASSERT_NE(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, !=, B, ::test::ResultCode::CRITICAL, ##__VA_ARGS__)

#define CHECK_LE(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, <=, B, ::test::ResultCode::FAIL, ##__VA_ARGS__)
#define ASSERT_LE(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, <=, B, ::test::ResultCode::CRITICAL, ##__VA_ARGS__)

#define CHECK_LT(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, <, B, ::test::ResultCode::FAIL, ##__VA_ARGS__)
#define ASSERT_LT(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, <, B, ::test::ResultCode::CRITICAL, ##__VA_ARGS__)

#define CHECK_GE(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, >=, B, ::test::ResultCode::FAIL, ##__VA_ARGS__)
#define ASSERT_GE(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, >=, B, ::test::ResultCode::CRITICAL, ##__VA_ARGS__)

#define CHECK_GT(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, >, B, ::test::ResultCode::FAIL, ##__VA_ARGS__)
#define ASSERT_GT(A, B, ...) \
  CHECK_BIN_FAIL_CODE(A, >, B, ::test::ResultCode::CRITICAL, ##__VA_ARGS__)

}  // namespace test

#ifndef TEST_NO_MAIN

#include <regex>

int main(int argc, char* argv[]) {
  std::vector<std::regex> select_tests;
  std::vector<std::regex> ignore_tests;
  bool list_tests = false;

#ifdef FILE_02522C8F_5040_40F1_ACBE_AEFC05B4514B_H
  options_parser::Parser app("run/list tests\n", "\n");
  app.add_parser(options_parser::parser());
  app.add_help();
  app.add_option("--test <name/pattern>",
                 [&](std::string n) {
                   select_tests.emplace_back(n, std::regex_constants::icase);
                 },
                 "select test, the pattern is supported by std::regex and "
                 "case insensitive");
  app.add_option("--Test <name/pattern>",
                 [&](std::string n) { select_tests.emplace_back(n); },
                 "select test, the pattern is supported by std::regex, and "
                 "case sensitive");
  app.add_option("--ignore-test <name/pattern>",
                 [&](std::string n) {
                   ignore_tests.emplace_back(n, std::regex_constants::icase);
                 },
                 "ignore test, the pattern is supported by std::regex and "
                 "case insensitive");
  app.add_option("--Ignore-Test <name/pattern>",
                 [&](std::string n) { ignore_tests.emplace_back(n); },
                 "ignore test, the pattern is supported by std::regex, and "
                 "case sensitive");
  app.add_option("--list-tests", [&]() { list_tests = true; },
                 "list all matched tests, instead of run them");
  {
    auto pr = app.parse(argc, argv);
    pr.check_print();
  }
#endif

  auto will_run_test = [&](std::string name) {
    auto is_select = [&]() {
      if (select_tests.size()) {
        for (auto& r : select_tests) {
          if (regex_search(name, r)) return true;
        }
        return false;
      }
      return true;
    };
    auto is_ignored = [&]() {
      if (ignore_tests.size()) {
        for (auto& r : ignore_tests) {
          if (regex_search(name, r)) return true;
        }
      }
      return false;
    };
    return is_select() && !is_ignored();
  };

  int num_fail = 0, num_succ = 0, num_ignore = 0;
  for (const auto& t : ::test::tests()) {
    if (!will_run_test(t->name_)) {
      ++num_ignore;
      continue;
    }
    if (list_tests) {
      std::cout << t->name_ << std::endl;
    } else {
      bool r = t->Run();
      if (r) {
        num_succ++;
      } else {
        num_fail++;
      }
    }
  }
  if (!list_tests) {
    if (num_fail) {
      std::cerr << "Test succ " << num_succ << "/ " << ::test::Test::color_red()
                << "fail " << num_fail << ::test::Test::color_reset()
                << std::endl;
    } else {
      std::cerr << "Test succ " << num_succ << "/ fail " << num_fail
                << std::endl;
    }
    if (num_ignore) {
      std::cerr << "*Note*\t" << num_ignore << " test(s) ignored";
    }
  }
  if (num_fail <= 64) return num_fail;
  return 64;
}

#endif

#endif // FILE_A1F24FBA_5A7E_4271_B00E_8450C8459AF6_H
