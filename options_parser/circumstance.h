#ifndef FILE_783C0585_1C8A_421D_A01D_952B4D77BF4E_H
#define FILE_783C0585_1C8A_421D_A01D_952B4D77BF4E_H
#include "options_parser/any.h"
#include "options_parser/string.h"

#include <typeinfo>
#include <map>

namespace options_parser {

struct Circumstance {
  Circumstance() = default;

  Circumstance(const Circumstance&) = default;

  void init() {
    if (!holder_) {
      holder_ = std::make_shared<Holder>();
    }
  }

  Any* get(const std::string& key) {
    if (!holder_) return NULL;
    auto it = holder_->key_values.find(key);
    if (it == holder_->key_values.end()) return NULL;
    return &it->second;
  }

  template <class T>
  T* get(const std::string& key) {
    auto a = this->get(key);
    return a->mutable_get<T>();
  }

  template <class T>
  T* get_or_set(const std::string& key, const T& default_value = T()) {
    init();
    auto it =
        holder_->key_values.insert(std::make_pair(key, default_value)).first;
    Any& a = it->second;
    auto p = a.mutable_get<T>();
    if (!p) {
      a = default_value;
      p = a.mutable_get<T>();
    }
    return p;
  }

  Any* get(const std::type_info& k) {
    if (!holder_) return NULL;
    auto it = holder_->type_values.find(&k);
    if (it == holder_->type_values.end()) return NULL;
    return &it->second;
  }

  template <class T>
  T* get_or_set(const T& default_value = T()) {
    init();
    auto it = holder_->type_values.insert(std::make_pair(&typeid(T),
                                                         default_value)).first;
    Any& a = it->second;
    auto p = a.mutable_get<T>();
    if (!p) {
      a = default_value;
      p = a.mutable_get<T>();
    }
    return p;
  }

  template <class T>
  T* get() {
    auto p = get(typeid(T));
    if (p) {
      return p->mutable_get<T>();
    }
    return NULL;
  }

  template <class T>
  T* flag(const string& name, const T& default_value = T()) {
    return get_or_set("/flag/" + name, default_value);
  }

  bool has_flag(const string& name) { return get("/flag/" + name) != NULL; }

  string to_str() {
    if (!holder_) return "{}";
    std::vector<string> vs;
    for (const auto& k_v : holder_->key_values) {
      vs.push_back(k_v.first + ": " + k_v.second.to_str());
    }
    for (const auto& t_v : holder_->type_values) {
      vs.push_back(string("[") + t_v.first->name() + "]" + ": " +
                   t_v.second.to_str());
    }
    return "{" + join(vs, ", ") + "}";
  }

  void erase(const string& name) {
    if (holder_) {
      holder_->key_values.erase(name);
    }
  }

  template <class T>
  void erase() {
    if (holder_) {
      holder_->type_values.erase(&typeid(T));
    }
  }

  // Another indirect layer makes Circumstance value semantic and any copy holds
  // the same deeper object.
  struct Holder {
    std::map<string, Any> key_values;
    std::map<const std::type_info*, Any> type_values;
  };

  std::shared_ptr<Holder> holder_;
};

}  // namespace options_parser
#endif  // FILE_783C0585_1C8A_421D_A01D_952B4D77BF4E_H
