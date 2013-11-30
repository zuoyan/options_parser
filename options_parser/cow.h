#ifndef FILE_A86993AE_1816_4880_819D_220C0D5ADBB7_H
#define FILE_A86993AE_1816_4880_819D_220C0D5ADBB7_H

namespace options_parser {

template <class T>
struct cow {
  cow(const T &value) { holder_ = new Holder(value); }

  cow(const cow &o) {
    holder_ = o.holder_;
    if (holder_) {
      __sync_add_and_fetch(&holder_->refcnt, 1);
    }
  }

  cow(cow &&o) {
    holder_ = o.holder_;
    o.holder_ = nullptr;
  }

  cow(std::nullptr_t) { holder_ = nullptr; }

  cow() { holder_ = nullptr; }

  ~cow() {
    if (holder_ && __sync_sub_and_fetch(&holder_->refcnt, 1) == 0) {
      delete holder_;
    }
  }

  cow &operator=(const cow &o) {
    if (&o != this) {
      this->~cow();
      new (this) cow(o);
    }
    return *this;
  }

  void swap(cow &o) { std::swap(holder_, o.holder_); }

  const T *get() const { return holder_ ? &holder_->value : nullptr; }

  T *mutable_get() {
    prepare_write();
    return holder_ ? &holder_->value : nullptr;
  }

  void prepare_write() {
    if (holder_ && holder_->refcnt > 1) {
      cow t(holder_->value);
      std::swap(t.holder_, holder_);
    }
  }

  struct Holder {
    volatile intptr_t refcnt;
    T value;

    Holder(const T &v) : value(v) { refcnt = 1; }
  };

  Holder *holder_;

  explicit operator bool() const { return holder_ != nullptr; }

  template <class U>
  static cow<U> wrap(const U &v) {
    return cow<U>(v);
  }

  template <class Func>
  auto bind(Func &&func) const -> decltype(wrap(func(holder_ -> value))) {
    if (holder_) {
      return wrap(func(holder_->value));
    }
    return nullptr;
  }
};

}  // namespace options_parser
#endif  // FILE_A86993AE_1816_4880_819D_220C0D5ADBB7_H
