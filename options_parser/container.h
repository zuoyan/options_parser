#ifndef FILE_721CE6EE_5A5B_411F_AC55_C14112DAD511_H
#define FILE_721CE6EE_5A5B_411F_AC55_C14112DAD511_H
#include "options_parser/maybe.h"
#include "options_parser/position.h"

namespace options_parser {

template <typename T>
struct has_begin_end {
 private:
  template <typename C>
  static auto check_begin(int)
      -> decltype(std::make_pair(mpl::value_c<1>{}, std::declval<C>().begin()));

  template <typename C>
  static std::pair<mpl::value_c<0>, void> check_begin(...);

  template <typename C>
  static auto check_end(int)
      -> decltype(std::make_pair(mpl::value_c<1>{}, std::declval<C>().end()));

  template <typename C>
  static std::pair<mpl::value_c<0>, void> check_end(...);

 public:
  typedef decltype(check_begin<const T>(0)) begin_type;
  typedef decltype(check_end<const T>(0)) end_type;
  static const bool value =
      begin_type::first_type::value && end_type::first_type::value;
  typedef typename begin_type::second_type result_type;
};

template <class T>
struct container_traits {
  typedef typename has_begin_end<T>::result_type iterator_type;
  typedef typename std::iterator_traits<iterator_type>::value_type value_type;
};

show_type<typename container_traits<int>::iterator_type>();

}  // namespace options_parser

#endif  // FILE_721CE6EE_5A5B_411F_AC55_C14112DAD511_H
