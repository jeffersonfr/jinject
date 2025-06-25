#pragma once

#include <type_traits>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <any>
#include <optional>
#include <functional>
#include <iostream>
#include <expected>
#include <iomanip>
#include <map>
#include <mutex>

#include <cxxabi.h>

#include "jmixin/jstring.h"

namespace jinject {
  template <typename T>
    concept SharedPtrConcept = std::same_as<std::shared_ptr<typename T::element_type>, T>;

  template <typename T>
    concept UniquePtrConcept = std::same_as<std::unique_ptr<typename T::element_type>, T>;

  template <typename T>
    concept SmartPtrConcept = SharedPtrConcept<T> or UniquePtrConcept<T>;

  template <typename T>
    concept PointerConcept = SmartPtrConcept<T> or std::is_pointer<T>::value;

  template <typename T>
    concept NoPointer = !PointerConcept<T>;
    
  enum instantiation_mode {
    UNKNOWN,
    SINGLE,
    FACTORY
  };

  namespace details {
    template <typename T>
      struct all_binds {
        inline static std::vector<std::function<T()>> mCallbacks;

        static void add(std::function<T()> callback) {
          mCallbacks.push_back(callback);
        }
      };
  }

  struct all {
    template <typename T, template <typename ...> class Container>
      operator Container<T> () {
        auto &callbacks = details::all_binds<T>::mCallbacks;

        Container<T> result;

        std::transform(callbacks.begin(), callbacks.end(), std::back_inserter(result),
          [](auto &&value) {
            return value();
          });

        return result;
      }
  };

  template <size_t N> struct StringLiteral {
  public:
    constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

    constexpr std::string to_string() const { return std::string{value, N - 1}; }

    char value[N];
  };

  struct named {
    named(std::string const &id, auto const &value) {
      if (sNames.find(id) != sNames.end()) {
        throw std::runtime_error(std::string("Name") + " '" + id + "' already defined");
      }

      sNames[id] = std::to_string(value);
    }

    named(std::string const &id, char const *value) {
      if (sNames.find(id) != sNames.end()) {
        throw std::runtime_error(std::string("Name") + " '" + id + "' already defined");
      }

      sNames[id] = value;
    }

    inline static std::map<std::string, jmixin::String> sNames;
  };

  template <StringLiteral ID>
    struct get_named {
      get_named(std::string const &value = "")
        : mDefault{value}
      {
      }

      std::expected<int32_t, std::string> get_int() {
        try {
          return std::stoi(get_string().value_or(""));
        } catch (std::invalid_argument &e) {
          // logt
        } catch (std::out_of_range &e) {
          // logt
        }

        return std::unexpected{"unable to convert named value to 'int'"};
      }

      std::expected<int64_t, std::string> get_long() {
        try {
          return std::stoll(get_string().value_or(""));
        } catch (std::invalid_argument &e) {
          // logt
        } catch (std::out_of_range &e) {
          // logt
        }

        return std::unexpected{"unable to convert named value to 'long'"};
      }

      std::expected<float, std::string> get_float() {
        try {
          return std::stof(get_string().value_or(""));
        } catch (std::invalid_argument &e) {
          // logt
        } catch (std::out_of_range &e) {
          // logt
        }

        return std::unexpected{"unable to convert named value to 'float'"};
      }

      std::expected<double, std::string> get_double() {
        try {
          return std::stod(get_string().value_or(""));
        } catch (std::invalid_argument &e) {
          // logt
        } catch (std::out_of_range &e) {
          // logt
        }

        return std::unexpected{"unable to convert named value to 'double'"};
      }

      std::expected<jmixin::String, std::string> get_string() {
        auto item = named::sNames.find(ID.to_string());

        if (item != named::sNames.end()) {
          return {item->second};
        }

        return std::unexpected{"no return registered"};
      }

      template <typename ...Args>
        jmixin::String format(Args ...args) {
          return get_string().value_or(mDefault).format(args...);
        }

      operator std::string () {
        return get_string().value_or(mDefault);
      }

      operator jmixin::String () {
        return get_string().value_or(mDefault);
      }

      private:
        jmixin::String mDefault;
    };

  template <typename ...Signature>
    struct get;

  namespace details {
    template <typename T, typename ...Signature>
      struct bind {
        bind() {
          auto callback = []() {
            return static_cast<T>(get<Signature...>{});
          };

          details::all_binds<T>::add(callback);
        }
      };

    template <typename T, typename ...Signature>
    struct instantiation : public bind<T, Signature...> {
      using type = std::remove_cvref_t<T>;

      static inline instantiation_mode mode = UNKNOWN;

      instantiation(): bind<T, Signature...>() {
        if (mode != UNKNOWN) {
          throw std::runtime_error("jinject::instantiation already defined");
        }
      }
    };

    template <typename T, typename ...Signature>
      struct factory: instantiation<T, Signature...> {
        factory(factory const &) = delete;
        factory(factory &&) = delete;

        template <typename ...Args>
          factory(std::function<T()> callback): instantiation<T, Signature...>() {
            if (callback) {
              mCallback = callback;

              instantiation<T, Signature...>::mode = FACTORY;
            }
          }

        static T get() {
          return mCallback();
        }

        factory & operator = (std::function<T()> const &callback) {
          if (instantiation<std::shared_ptr<T>, Signature...>::mode != UNKNOWN) {
            throw std::runtime_error("jinject::unable to replace instantiation");
          }

          mCallback = callback;

          instantiation<T, Signature...>::mode = FACTORY;

          return *this;
        }

        private:
          static inline std::function<T()> mCallback;
      };

    struct InternalType {};

    template <typename T, typename ...Signature>
      requires (!SmartPtrConcept<T>) && (!PointerConcept<T>)
      struct shared {
        shared(shared const &) = delete;
        shared(shared &&) = delete;

        shared(InternalType) {
        }

        shared & operator = (std::function<T*()> const &callback) {
          std::weak_ptr<T, Signature...> weak;

          factory<std::shared_ptr<T>, Signature...> {
            [=]() mutable {
              if (auto ptr = weak.lock()) {
                return ptr;
              }

              auto ptr = std::shared_ptr<T, Signature...>(callback());

              weak = ptr;

              return ptr;
            }
          };

          return *this;
        }
      };


    template <typename T, typename ...Signature>
      requires (!SmartPtrConcept<T>) && (!PointerConcept<T>)
      struct unique {
        unique(unique const &) = delete;
        unique(unique &&) = delete;

        unique(InternalType) {
        }

        unique & operator = (std::function<T*()> const &callback) {
          factory<std::unique_ptr<T>, Signature...> {
            [=]() {
              return std::unique_ptr<T>(callback());
            }
          };

          return *this;
        }
      };

    std::string demangle(char const *name) {
      int status = -999;

      std::unique_ptr<char, void(*)(void*)> res {
          abi::__cxa_demangle(name, NULL, NULL, &status), std::free
      };

      return (status == 0)?res.get():name;
    }

    template <typename T, typename ...Signature>
      struct single: instantiation<T, Signature...> {
        single(single const &) = delete;
        single(single &&) = delete;
      };

    template <typename T, typename ...Signature>
      struct single<T*, Signature...>: instantiation<T*, Signature...> {
        single(single const &) = delete;
        single(single &&) = delete;

        template <typename ...Args>
          single(std::function<T*()> callback): instantiation<T*, Signature...>() {
            mInstance = callback();

            instantiation<T*, Signature...>::mode = SINGLE;
          }

        static T const * const get() {
          return mInstance;
        }

        single & operator = (std::function<T*()> const &callback) {
          mInstance = callback();

          return *this;
        }

        private:
          static inline T *mInstance;
      };

    template <typename T, typename ...Signature>
      struct single<std::shared_ptr<T>, Signature...>: instantiation<std::shared_ptr<T>, Signature...> {
        single(single const &) = delete;
        single(single &&) = delete;

        template <typename ...Args>
          single(std::function<std::shared_ptr<T>()> callback): instantiation<std::shared_ptr<T>, Signature...>() {
            if (callback) {
              mInstance = callback();

              instantiation<std::shared_ptr<T>, Signature...>::mode = SINGLE;
            }
          }

        static std::shared_ptr<T> const get() {
          return mInstance;
        }

        single & operator = (std::function<std::shared_ptr<T>()> const &callback) {
          if (instantiation<std::shared_ptr<T>, Signature...>::mode != UNKNOWN) {
            throw std::runtime_error("jinject::unable to replace instantiation");
          }

          mInstance = callback();

          instantiation<std::shared_ptr<T>, Signature...>::mode = SINGLE;

          return *this;
        }

        private:
          static inline std::shared_ptr<T> mInstance;
      };
  }

  template <typename T>
  struct introspection {
    static std::string to_string() {
      return details::demangle(typeid(T).name());
    }
  };

  template <typename T>
  struct introspection<T*> {
    static std::string to_string() {
      return introspection<T>::to_string() + "*";
    }
  };

  template <typename T>
  struct introspection<std::shared_ptr<T>> {
    static std::string to_string() {
      return "std::shared_ptr<" + introspection<T>::to_string() + ">";
    }
  };

  template <typename T>
  struct introspection<std::unique_ptr<T>> {
    static std::string to_string() {
      return "std::unique_ptr<" + introspection<T>::to_string() + ">";
    }
  };

  template <typename ...Signature>
    struct get {
      get() = default;

      template <typename T>
        operator T () const {
          if (details::instantiation<T, Signature...>::mode == SINGLE) {
            if constexpr(SharedPtrConcept<T>) {
              return details::single<T, Signature...>::get();
            } else {
              throw std::runtime_error("jinject::single instantiation must use shared smart pointer");
            }
          } else if (details::instantiation<T, Signature...>::mode == FACTORY) {
            return details::factory<T, Signature...>::get();
          }

          std::ostringstream o;

          o << "jinject::undefined instantiation of " << std::quoted(introspection<T>::to_string());

          throw std::runtime_error(o.str());
        }
    };

  template <typename T, typename ...Signature>
    decltype(auto) inject() {
      return static_cast<T>(get<Signature...>{});
    }

  template <typename T, typename ...Signature>
    [[nodiscard]] std::expected<T, std::string> inject_by() {
      try {
        return {static_cast<T>(get<Signature...>{})};
      } catch (std::runtime_error &e) {
        return std::unexpected{e.what()};
      }
    };

  template <typename T, typename ...Signature>
    struct lazy {
      T operator() () {
        std::call_once(mFlag,
          [this]() {
            mReference = get<Signature...>{};
          });

        return mReference;
      }

      private:
        T mReference;
        std::once_flag mFlag;
    };

  template <typename T, typename ...Signature>
    struct lazy<std::unique_ptr<T>, Signature...> {
      std::unique_ptr<T> operator() () {
        return get<Signature...>{};
      }
    };











  /*
   * struct Base {
   *   virtual void f() = 0;
   * };
   *
   * struct Derived : public Base {
   * };
   *
   * struct BaseImpl : public Base {
   *   void f() {
   *     std::cout << "Hello, world !" << std::endl;
   *   }
   * };
   *
   * Derived derived = by<BaseImpl>{}; // static_cast<Derived *>(new struct NewDerived : Derived, BaseImpl);
   *
   */
  template <typename Base, typename ...Signature>
    struct by {
      by() = default;

    template <typename T>
      operator T () const {
        static_assert(false, "unable to implement static class");
      }

    template <typename T>
      operator T* () const {
        struct internal_class : public Base, public T {};

        return reinterpret_cast<T *>(static_cast<Base *>(new internal_class{}));
      }

    template <typename T>
      operator std::shared_ptr<T> () const {
        return std::shared_ptr<T>{static_cast<T*>(*this)};
      }

    template <typename T>
      operator std::unique_ptr<T> () const {
        return std::unique_ptr<T>{static_cast<T*>(*this)};
      }

  };
}

#define NAMED(ID, VALUE) \
  named{ID, VALUE}

#define FACTORY(T, ...) \
  details::factory<T, ##__VA_ARGS__> { nullptr } = [=]() -> T 

#define SHARED(T, ...) \
  details::shared<T, ##__VA_ARGS__> {details::InternalType{}} = []() -> T*

#define UNIQUE(T, ...) \
  details::unique<T, ##__VA_ARGS__> {details::InternalType{}} = []() -> T*

#define SINGLE(T, ...) \
  details::single<T, ##__VA_ARGS__> { nullptr } = [=]() -> T
