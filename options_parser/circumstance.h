#ifndef FILE_783C0585_1C8A_421D_A01D_952B4D77BF4E_H
#define FILE_783C0585_1C8A_421D_A01D_952B4D77BF4E_H
#include "options_parser/any.h"
#include "options_parser/join.h"

#include <typeinfo>
#include <map>
#include <memory>

namespace options_parser {

template <class K, class V>
struct VariableStack {
 public:
  VariableStack() = default;
  VariableStack(std::shared_ptr<VariableStack> parent) : parent_(parent) {}

  VariableStack* root() {
    auto p = this;
    while (p->parent_) {
      p = p->parent_.get();
    }
    return p;
  }

  V* local_get(const K& k) {
    auto it = local_.find(k);
    if (it != local_.end()) {
      return &it->second;
    }
    return nullptr;
  }

  V* get(const K& k) {
    auto p = this;
    V* v = nullptr;
    while (p && !v) {
      v = p->local_get(k);
      p = p->parent_.get();
    }
    return v;
  }

  V* global_get(const K& k) { return root()->local_get(k); }

  std::pair<V*, bool> local_insert(const K& k, const V& v) {
    auto it_b = local_.emplace(k, v);
    return std::make_pair(&it_b.first->second, it_b.second);
  }

  std::pair<V*, bool> insert(const K& k, const V& v) {
    auto p = this;
    while (p) {
      auto it = p->local_.find(k);
      if (it != p->local_.end()) {
        return std::make_pair(&it->second, false);
      }
      p = p->parent_.get();
    }
    return local_insert(k, v);
  }

  std::pair<V*, bool> global_insert(const K& k, const V& v) {
    return root()->local_insert(k, v);
  }

  V* local_set(const K& k, const V& v) {
    auto& d = local_[k];
    d = v;
    return &d;
  }

  V* set(const K& k, const V& v) {
    auto r = insert(k, v);
    if (!r.second) {
      *r.first = v;
    }
    return r.first;
  }

  V* global_set(const K& k, const V& v) { return root()->local_set(k, v); }

  void local_erase(const K& k) { local_.erase(k); }

  std::shared_ptr<VariableStack> parent() const {
    return parent_;
  }

  typename std::map<K, V>::iterator local_begin() { return local_.begin(); }
  typename std::map<K, V>::iterator local_end() { return local_.end(); }

 private:
  std::map<K, V> local_;
  std::shared_ptr<VariableStack> parent_;
};

struct Circumstance {
  typedef VariableStack<std::string, Any> NameVariableStack;
  typedef VariableStack<const std::type_info*, Any> TypeVariableStack;

  Circumstance() {
    names_ = std::make_shared<NameVariableStack>();
    types_ = std::make_shared<TypeVariableStack>();
  }

  Circumstance(const Circumstance&) = default;

  Circumstance new_child() {
    return Circumstance(std::make_shared<NameVariableStack>(names_),
                        std::make_shared<TypeVariableStack>(types_));
  }

  Circumstance parent() {
    Circumstance c(names_->parent(),
                   types_->parent());
    if (!c.names_) c.names_ = names_;
    if (!c.types_) c.types_ = types_;
    return c;
  }

  template <class T>
  T* get(const std::string& k) {
    Any* a = names_->get(k);
    if (a) return a->mutable_get<T>();
    return nullptr;
  }

  template <class T>
  T* local_get(const std::string& k) {
    Any* a = names_->local_get(k);
    if (a) return a->mutable_get<T>();
    return nullptr;
  }

  template <class T>
  T* global_get(const std::string& k) {
    Any* a = names_->global_get(k);
    if (a) return a->mutable_get<T>();
    return nullptr;
  }

  template <class T>
  T* get() {
    Any* a = types_->get(&typeid(T));
    if (a) return a->mutable_get<T>();
    return nullptr;
  }

  template <class T>
  T* local_get() {
    Any* a = types_->local_get(&typeid(T));
    if (a) return a->mutable_get<T>();
    return nullptr;
  }

  template <class T>
  T* global_get() {
    Any* a = types_->global_get(&typeid(T));
    if (a) return a->mutable_get<T>();
    return nullptr;
  }

  template <class T>
  T* set(const std::string& k, const T& v) {
    Any* a = names_->set(k, Any{v});
    return a->mutable_get<T>();
  }

  template <class T>
  T* local_set(const std::string& k, const T& v) {
    Any* a = names_->local_set(k, v);
    return a->mutable_get<T>();
  }

  template <class T>
  T* global_set(const std::string& n, const T& v) {
    Any* a = names_->global_set(n, v);
    return a->mutable_get<T>();
  }

  template <class T>
  T* set(const T& v) {
    Any* a = names_->set(&typeid(T), v);
    return a->mutable_get<T>();
  }

  template <class T>
  T* local_set(const T& v) {
    Any* a = types_->local_set(&typeid(T), v);
    return a->mutable_get<T>();
  }

  template <class T>
  T* global_set(const T& v) {
    Any* a = names_->global_set(&typeid(T), v);
    return a->mutable_get<T>();
  }

  void local_erase(const std::string& k) { names_->local_erase(k); }
  template <class T>
  void local_erase() {
    types_->local_erase(&typeid(T));
  }

  template <class T=std::string>
  T* flag(const std::string& name) {
    auto k = "/flag/" + name;
    auto v = global_get<T>(k);
    return v;
  }

  template <class T=std::string>
  T* flag(const std::string& name, const T& default_value) {
    auto k = "/flag/" + name;
    auto v = global_get<T>(k);
    if (!v) {
      v = global_set<T>(k, default_value);
    }
    return v;
  }

  template <class T>
  T* flag_set(const std::string& name, const T& value) {
    auto k = "/flag/" + name;
    return global_set<T>(k, value);
  }

  string to_str() {
    string ret = "{";
    string sep = "";
    auto append = [&](string v) {
      if (v.size()) {
        ret += sep + v;
        sep = ", ";
      }
    };
    if (names_) {
      append(join(names_->local_begin(), names_->local_end(), ", ",
                  pair_to_str(":")));
    }
    if (types_) {
      append(join(types_->local_begin(), types_->local_end(), ", ",
                  pair_to_str(":", [](const std::type_info* ti) {
                    return string("[") + ti->name() + "]";
                  })));
    }
    if (names_->parent() || types_->parent()) {
      append("parent: " + parent().to_str());
    }
    return ret + "}";
  }

 private:
  Circumstance(std::shared_ptr<NameVariableStack> names,
               std::shared_ptr<TypeVariableStack> types)
      : names_(names), types_(types) {}

  std::shared_ptr<NameVariableStack> names_;
  std::shared_ptr<TypeVariableStack> types_;
};

}  // namespace options_parser
#endif  // FILE_783C0585_1C8A_421D_A01D_952B4D77BF4E_H
