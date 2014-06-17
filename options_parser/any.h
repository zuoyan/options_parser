#ifndef FILE_2D51037C_E4CE_417A_8210_6893881CF367_H
#define FILE_2D51037C_E4CE_417A_8210_6893881CF367_H
#include <typeinfo>

#include "options_parser/string.h"
#include "options_parser/converter.h"

namespace options_parser {

class Any {
 public:
  Any() = default;
  Any(const Any&) = default;

  template <class T>
  Any(const T& v) {
    inst_ = std::make_shared<Holder<T>>(v);
  }

  template <class T>
  bool is_instance() {
    if (!inst_) return false;
    return inst_->type() == typeid(T);
  }

  bool is_empty() { return !inst_; }

  template <class T>
  const T* get() const {
    if (inst_ && inst_->type() == typeid(T)) {
      return &((Holder<T>*)inst_.get())->value;
    }
    return NULL;
  }

  template <class T>
  T* mutable_get() {
    if (inst_ && inst_->type() == typeid(T)) {
      if (inst_.use_count() > 1) {
        inst_.reset(inst_->clone());
      }
      return &((Holder<T>*)inst_.get())->value;
    }
    return NULL;
  }

  string to_str() const {
    if (inst_) {
      return inst_->to_str();
    }
    return "(nil)";
  }

 private:
  struct Interface {
    virtual ~Interface() {}
    virtual Interface* clone() const = 0;
    virtual const std::type_info& type() const = 0;
    virtual string to_str() const = 0;
  };

  template <class T>
  struct Holder : Interface {
    template <class... Args>
    Holder(Args&&... args)
        : value(std::forward<Args>(args)...) {}

    virtual Interface* clone() const { return new Holder(value); }

    virtual const std::type_info& type() const { return typeid(T); }

    template <class U>
    typename std::enable_if<has_to_str<U>::value, string>::type to_str_impl(
        const U& value) const {
      return options_parser::to_str(value);
    }

    template <class U>
    typename std::enable_if<!has_to_str<U>::value, string>::type to_str_impl(
        const U& value) const {
      return string("object<") + typeid(U).name() + ">";
    }

    virtual string to_str() const { return to_str_impl(value); }

    T value;
  };

  std::shared_ptr<Interface> inst_;
};

inline string to_str(const Any& a) { return a.to_str(); }

}  // namespace options_parser
#endif  // FILE_2D51037C_E4CE_417A_8210_6893881CF367_H
