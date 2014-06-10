#ifndef FILE_A93FFB28_73C5_446E_8B92_F57759FA7E26_H
#define FILE_A93FFB28_73C5_446E_8B92_F57759FA7E26_H
#include <functional>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace test {

template <class T>
struct ValueMessage {
  T value;
  std::string message;
};

struct CondMessage {
  CondMessage(const CondMessage&) = default;
  CondMessage& operator=(const CondMessage&) = default;

  CondMessage(bool v = true) { value = v; }

  CondMessage(const std::string& m) {
    value = false;
    message = m;
  }

  explicit operator bool() const { return value; }

  bool value;
  std::string message;
};

template <class T>
struct ToStringFunctor {
  std::string operator()(const T& v) {
    return std::to_string(v);
  }
};

template <class T>
struct ToStringFunctor<ValueMessage<T>> {
  std::string operator()(const ValueMessage<T>& v) {
    ToStringFunctor<T> ts;
    return ts(v.value) + '(' + v.message + ')';
  }
};

struct ToString {
  template <class T>
  std::string operator()(const T& v) {
    ToStringFunctor<T> s;
    return s(v);
  }
};

template <class Func>
struct Predictor {
  Func func_;

  Predictor(const Predictor&) = default;
  Predictor(Predictor&&) = default;
  Predictor& operator=(const Predictor&) = default;
  Predictor& operator=(Predictor&&) = default;
  Predictor() = default;

  Predictor(Func func) : func_(func) {}

  template <class U>
  CondMessage operator()(const U& v) {
    return func_(v);
  }
};

template <class Func, class Modify>
struct CombineFunctor {
  Func func_;
  Modify modify_;

  template <class Func>
  auto operator()(const U& v)OPTIONS_PARSER_AUTO_RETURN(modify_(func_(v)));
};

template <class Func, class Modify>
CombineFunctor<Func, Modify> Combine(Func func, Modify modify)
    OPTIONS_PARSER_AUTO_RETURN(CombineFunctor<Func, Modify>{func, modify});

template <class Func>
Predictor<Func> Predict(const Func& func) {
  return Predictor<Func>{func};
}

template <class Func>
Predictor<Func> Predict(const Predictor<Func>& pred) {
  return pred;
}

template <class F1>
auto CombinePredict(F1 f1, std::function<CondMessage(CondMessage)> f2)
    OPTIONS_PARSER_AUTO_RETURN(Combine(f1, f2));

template <class Pred>
auto Not(Pred pred)
    OPTIONS_PARSER_AUTO_RETURN(CombinePredict(pred, [](CondMessage cm)
                                                        -> CondMessage {
      if (cm) {
        return "not " + cm.message;
      }
      return true;
    }));

struct EQFunctor {
  template <class L, class R>
  bool operator()(const L& l, const R& r) const {
    return l == r;
  }
};

struct NEFunctor {
  template <class L, class R>
  bool operator()(const L& l, const R& r) const {
    return l != r;
  }
};

struct LTFunctor {
  template <class L, class R>
  bool operator()(const L& l, const R& r) const {
    return l < r;
  }
};

struct LEFunctor {
  template <class L, class R>
  bool operator()(const L& l, const R& r) const {
    return l <= r;
  }
};

struct GTFunctor {
  template <class L, class R>
  bool operator()(const L& l, const R& r) const {
    return l > r;
  }
};

struct GEFunctor {
  template <class L, class R>
  bool operator()(const L& l, const R& r) const {
    return l >= r;
  }
};

template <class F, class First>
struct BindFirstFunctor {
  F func_;
  First first_;

  template <class Second>
  auto operator()(const Second& second) -> decltype(func_(first_, second)) {
    return func_(first_, second);
  }
};

template <class F, class Second>
struct BindSecondFunctor {
  F func_;
  Second second_;

  template <class First>
  auto operator()(const First& first) -> decltype(func_(first, second_)) {
    return func_(first, second_);
  }
};

template <class F, class First>
BindFirstFunctor<F, First> BindFirst(F f, First first) {
  return BindFirstFunctor{f, first};
}

template <class F, class Second>
BindSecondFunctor<F, Second> BindSecond(F f, Second second) {
  return BindSecondFunctor{f, second};
}

template <class VM, class Binary, class ToStr = ToString>
struct BinaryPredictor {
  VM vm_;
  Binary binary_;
  std::string op_;
  ToStr to_str_;

  template <class A>
  CondMessage operator()(const A& a) {
    CondMessage cm;
    cm.value = binary_(actual.value, expect.value);
    cm.message = to_str_(actual) + " " + op + " " + to_str_(expect);
    return cm;
  }
};

template <class T, class Binary, class ToStr>
auto Binary(const T& expect, Binary binary, const std::string& op, ToStr to_str)
    OPTIONS_PARSER_AUTO_RETURN(BinaryPredictor<T, Binary, ToStr>{expect, binary,
                                                                 op, to_str});

template <class T, class ToStr = ToString>
auto EQ(const T& expect, ToStr to_str = ToStr())
    OPTIONS_PARSER_AUTO_RETURN(Binary(expect, EQFunctor{}, to_str));

template <class T, class ToStr = ToString>
auto NE(const T& expect, ToStr to_str = ToStr())
    OPTIONS_PARSER_AUTO_RETURN(Binary(expect, NEFunctor{}, to_str));

template <class T>
auto Predict(const T& v, const string& name)
    OPTIONS_PARSER_AUTO_RETURN(EQ(ValueMessage<T>{v, name}));

template <class Func>
auto Predict(const Predictor<Func>& v, const string& name)
    OPTIONS_PARSER_AUTO_RETURN(v);

#define CHECK_COND_(COUNTER, COND, ...)                                  \
  ::test::CondMessage PP_CAT(test_cond_, COUNTER) {COND};                \
  CHECK_FAIL_CODE(PP_CAT(test_cond_, COUNTER), ::test::ResultCode::FAIL, \
                  PP_CAT(test_cond_, COUNTER).message, ##__VA_ARGS__);

#define CHECK_COND(COND, ...) CHECK_COND_(__COUNTER__, COND, ##__VA_ARGS__);

#define CHECK_MATCH(VALUE, MATCH, ...) \
  CHECK_COND(::test::Predict(MATCH)(#VALUE, VALUE), ##__VA_ARGS__);

template <class VTS = ToString>
struct ContainerToStringFunctor {
  VTS value_to_str_;
  std::string lp_ = std::string("[");
  std::string rp_ = std::string("]");
  std::string sep_ = std::string(", ");

  template <class VS>
  std::string operator()(const VS& vs) {
    size_t  c = 0;
    std::string ret;
    for (auto&& v : vs) {
      if (c ++ > 0) {
        ret += sep_;
      }
      ret += value_to_str_(v);
    }
    return lp_ + ret + rp_;
  }
};

template <class VTS = ToString>
ContainerToStringFunctor<VTS> ContainerToString(const std::string& lp = "[",
                                                const std::string& rp = "]",
                                                const std::string& sep = ", ",
                                                VTS vts = VTS()) {
  ContainerToStringFunctor<VTS> c;
  c.value_to_str_ = vts;
  c.lp_ = lp;
  c.rp_ = rp;
  c.sep_  = sep;
  return c;
}

template <class ValueEQ = EQFunctor>
struct ContainerEQBinary {
  ValueEQ value_eq;

  template <class LS, class RS>
  bool operator()(const LS& lvs, const RS& rvs) const {
    auto lit = lvs.begin(), le = lvs.end();
    auto rit = rvs.begin(), re = rvs.end();
    while (lit != le && rit != re) {
      if (value_eq(*lit, *rit)) {
        ++lit;
        ++rit;
      } else {
        return false;
      }
    }
    return lit == le && rit == re;
  }
};

template <class ValueEQ = EQFunctor>
ContainerEQFunctor<ValueEQ> ConainerEQ(ValueEQ value_eq) {
  return ContainerEQFunctor<ValueEQ>{value_eq};
}

template <class T, class EQ = ContainerEqualFunctor<>,
          class TS = ContainerToStringFunctor<>>
auto ContainerEQ(const std::vector<T>& right, EQ eq = EQ(), TS ts = TS())
    -> decltype(EQ(right, eq, ts)) {
  return EQ(right, eq, ts);
}

template <class Pred>
struct ElementsPredictor {
  std::vector<Pred> preds_;

  template <class VS>
  CondMessage operator()(const VS& vs) {
    size_t c = 0;
    for (auto&& v : vs) {
      if (c < preds_.size()) {
        auto m = preds[c](v);
        if (!m) {
          return "At " + std::to_string(c) + " element: " + m.message;
        }
      }
      ++c;
    }
    std::string result;
  }
};

template <class Pred>
auto Elements(const std::initializer_list<Pred>& preds) {

};

template <class Matches>
CondMessage
  Matches matches_;

  template <class R>
  CondMessage operator()(const std::string& actual_name, const U& actual) {

  }
};

template <class ...M>
auto TupleString(const M& ...match) {
  auto matches = std::make_tuple(match...);
};

template <class ...M>
auto Tuple(const M& ...match) {

}

}  // namespace test
#endif // FILE_A93FFB28_73C5_446E_8B92_F57759FA7E26_H
