#ifndef FILE_BBFE563E_83FC_4DB0_BCF6_8B3056687A3B_H
#define FILE_BBFE563E_83FC_4DB0_BCF6_8B3056687A3B_H
namespace options_parser {
namespace mpl {

template <class T>
struct protect {
  typedef T type;
};

template <int V>
struct value_c {
  typedef value_c type;
  static constexpr int value = V;
};

template <int... V>
struct vector_c {
  typedef vector_c type;
  static constexpr int size = sizeof...(V);
};

template <class... T>
struct vector_t {
  typedef vector_t type;
  static constexpr int size = sizeof...(T);
};

template <class A, class B>
struct cat_c;

template <template <int...> class SA, int... A, template <int...> class SB,
          int... B>
struct cat_c<SA<A...>, SB<B...>> {
  typedef SA<A..., B...> type;
};

template <template <class...> class N, class T>
struct rewrap;

template <template <class...> class N, template <class...> class O, class... T>
struct rewrap<N, O<T...>> {
  typedef N<T...> type;
};

constexpr intmax_t amount() { return 0; }

template <class... T>
constexpr intmax_t amount(intmax_t h, T... v) {
  return h + amount(v...);
}

template <int First, int Step, int Last,
          bool Check = ((Last - First) * Step > 0)>
struct vector_range_impl
    : cat_c<vector_c<First>,
            typename vector_range_impl<First + Step, Step, Last>::type> {};

template <int First, int Step, int Last>
struct vector_range_impl<First, Step, Last, false> : vector_c<> {};

template <int N>
struct vector_range : vector_range_impl<0, 1, N> {};

template <class T>
struct cdr;

template <template <class...> class S, class... T>
struct cdr<S<T...>> {
  template <class UH, class... UT>
  static protect<S<UT...>> deduce(vector_t<UH, UT...>);

  typedef decltype(deduce(std::declval<vector_t<T...>>())) protect_type;
  typedef typename protect_type::type type;
};

template <class F, class... Args>
struct is_callable {
  struct dummy {};

  template <class U>
  static auto check(int)
      -> decltype(std::declval<U>()(std::declval<Args>()...));
  template <class U>
  static dummy check(...);

  typedef decltype(check<F>(0)) result_type;
  static constexpr bool value = !std::is_same<result_type, dummy>::value;
};

struct function_parameter_deduce {
  template <class T>
  struct apply {
    typedef typename std::remove_cv<
        typename std::remove_reference<T>::type>::type type;
  };
};

template <class F>
struct function_traits {
  typedef function_traits<decltype(&F::operator())> caller;
  typedef typename caller::result_type result_type;
  typedef typename cdr<typename caller::arguments_type>::type arguments_type;
  typedef typename cdr<typename caller::parameters_type>::type parameters_type;
  static constexpr int nary = caller::nary - 1;
};

template <class F>
struct function_traits<F &> : function_traits<F> {};

template <class F>
struct function_traits<F &&> : function_traits<F> {};

template <class R, class... Args>
struct function_traits<R(Args...)> {
  typedef vector_t<Args...> arguments_type;
  typedef std::tuple<typename function_parameter_deduce::template apply<
      Args>::type...> parameters_type;
  typedef R result_type;
  typedef void class_type;
  static constexpr int nary = sizeof...(Args);
};

template <class R, class... Args>
struct function_traits<R (*)(Args...)> : function_traits<R(Args...)> {};

template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R(C *, Args...)> {
  typedef C class_type;
  static constexpr int nary = sizeof...(Args)+1;
};

template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const>
    : function_traits<R(C *, Args...)> {
  typedef C class_type;
  static constexpr int nary = sizeof...(Args)+1;
};

}  // namespace mpl
}  // namespace options_parser
#endif  // FILE_BBFE563E_83FC_4DB0_BCF6_8B3056687A3B_H
