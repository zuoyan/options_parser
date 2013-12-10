#ifndef FILE_2D51037C_E4CE_417A_8210_6893881CF367_H
#define FILE_2D51037C_E4CE_417A_8210_6893881CF367_H
#include <typeinfo>

namespace options_parser {

struct Any {
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

  struct Interface {
    virtual Interface* clone() const = 0;
    virtual const std::type_info& type() const = 0;
    virtual string to_str() const = 0;
  };

  template <class T>
  struct Holder : Interface {
    template <class... Args>
    Holder(Args&&... args) : value(std::forward<Args>(args)...) {}

    virtual Interface* clone() const {
      return new Holder(value);
    }

    virtual const std::type_info& type() const {
      return typeid(T);
    }

    virtual string to_str() const { return options_parser::to_str(value); }

    T value;
  };

  std::shared_ptr<Interface> inst_;
};

}  // namespace options_parser
#endif // FILE_2D51037C_E4CE_417A_8210_6893881CF367_H
