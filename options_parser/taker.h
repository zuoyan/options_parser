#ifndef FILE_958B555F_6C70_4ED7_B6C6_6F14F6CE653F_H
#define FILE_958B555F_6C70_4ED7_B6C6_6F14F6CE653F_H
#include "options_parser/matcher.h"
#include "options_parser/converter.h"

namespace options_parser {

struct TakeResult {
  Position end;
  Arguments *args;
  Maybe<std::string> error;
};

struct Taker {
  Taker() {}

  Taker(const Taker &) = default;

  template <class F, typename std::enable_if<
                         std::is_same<typename mpl::is_callable<
                                          F, const MatchResult &>::result_type,
                                      TakeResult>::value,
                         int>::type = 0>
  Taker(const F &func) {
    take_ = func;
  }

  template <class F,
            typename std::enable_if<
                mpl::is_callable<F, const MatchResult &>::value &&
                    !std::is_same<typename mpl::is_callable<
                                      F, const MatchResult &>::result_type,
                                  TakeResult>::value,
                int>::type = 0>
  Taker(const F &func) {
    take_ = [func](const MatchResult &mr) {
      TakeResult tr;
      auto pa = func(mr);
      tr.end = pa.position;
      tr.args = pa.args;
      return tr;
    };
  }

  template <class T,
            typename std::enable_if<!std::is_function<T>::value, int>::type = 0>
  Taker(T *ptr) {
    take_ = [ptr](const MatchResult &mr) {
      TakeResult tr;
      auto v_s = value()(PositionArguments{mr.end, mr.args});
      tr.error = get_error(v_s.first);
      if (!tr.error) {
        tr.error = from_str<T>(get_value(v_s.first), ptr);
      }
      tr.end = v_s.second.position;
      tr.args = v_s.second.args;
      return tr;
    };
  }

  template <class F, typename std::enable_if<
                         mpl::is_callable<F, const PositionArguments &>::value,
                         int>::type = 0>
  Taker(const F &func) {
    take_ = [func, this](const MatchResult &mr) {
      TakeResult tr;
      PositionArguments pa{mr.end, mr.args};
      auto v_s = func(pa);
      tr.end = v_s.second.position;
      tr.args = v_s.second.args;
      tr.error = to_error(v_s.first);
      return tr;
    };
  }

  template <class F,
            typename std::enable_if<
                !mpl::is_callable<F, const MatchResult &>::value &&
                    !mpl::is_callable<F, const PositionArguments &>::value,
                int>::type = 0>
  Taker(const F &func) {
    take_ = [func, this](const MatchResult &mr) {
      TakeResult tr;
      auto get_values = tuple_value_indices<
          typename mpl::function_traits<F>::parameters_type>(
          typename mpl::vector_range<mpl::function_traits<F>::nary>::type{});
      PositionArguments pa{mr.end, mr.args};
      auto v_s = get_values.apply(check_invoke(void_wrap(func)))(pa);
      tr.end = v_s.second.position;
      tr.args = v_s.second.args;
      tr.error = to_error(v_s.first);
      return tr;
    };
  }

  TakeResult operator()(const MatchResult &mr) const { return take_(mr); }

  static Maybe<std::string> to_error(Nothing) { return nothing; }

  static Maybe<std::string> to_error(void_) { return nothing; }

  template <class T>
  static Maybe<std::string> to_error(const Maybe<T> &v) {
    if (!v) return std::string("empty result");
    return to_error(*v.get());
  }

  template <class T>
  static Maybe<std::string> to_error(const Either<T> &ve) {
   auto e = get_error(ve);
   if (e) return e;
   return to_error(get_value(ve));
  }

  static Maybe<std::string> to_error(int c) {
    if (c > 0) return nothing;
    return "error code: " + std::to_string(c);
  }

  static Maybe<std::string> to_error(bool f) {
    if (f) return nothing;
    return std::string("failed");
  }

  static Maybe<std::string> to_error(const std::string &e) {
    if (!e.size()) return nothing;
    return e;
  }

  std::function<TakeResult(const MatchResult &)> take_;
};

}  // namespace options_parser
#endif  // FILE_958B555F_6C70_4ED7_B6C6_6F14F6CE653F_H
