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

#include <cxxabi.h>

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
    
  enum instantiation_type {
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

  template <typename ...Signature>
    struct get;

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
    static inline instantiation_type type = UNKNOWN;

    instantiation(): bind<T, Signature...>() {
      if (type != UNKNOWN) {
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

            instantiation<T, Signature...>::type = FACTORY;
          }
        }

      static T get() {
        return mCallback();
      }

      factory & operator = (std::function<T()> const &callback) {
        if (instantiation<std::shared_ptr<T>, Signature...>::type != UNKNOWN) {
          throw std::runtime_error("jinject::unable to replace instantiation");
        }

        mCallback = callback;

        instantiation<T, Signature...>::type = FACTORY;

        return *this;
      }

      private:
        static inline std::function<T()> mCallback;
    };

#define FACTORY(T, ...) \
  factory<T, ##__VA_ARGS__> { nullptr } = [=]() -> T 

  namespace details {
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

#define SHARED(T, ...) \
    details::shared<T, ##__VA_ARGS__> {details::InternalType{}} = []() -> T*

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

#define UNIQUE(T, ...) \
    details::unique<T, ##__VA_ARGS__> {details::InternalType{}} = []() -> T*

    std::string demangle(char const *name) {
      int status = -999;

      std::unique_ptr<char, void(*)(void*)> res {
          abi::__cxa_demangle(name, NULL, NULL, &status), std::free
      };

      return (status == 0)?res.get():name;
    }
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

          instantiation<T*, Signature...>::type = SINGLE;
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

            instantiation<std::shared_ptr<T>, Signature...>::type = SINGLE;
          }
        }

      static std::shared_ptr<T> const get() {
        return mInstance;
      }

      single & operator = (std::function<std::shared_ptr<T>()> const &callback) {
        if (instantiation<std::shared_ptr<T>, Signature...>::type != UNKNOWN) {
          throw std::runtime_error("jinject::unable to replace instantiation");
        }

        mInstance = callback();

        instantiation<std::shared_ptr<T>, Signature...>::type = SINGLE;

        return *this;
      }

      private:
        static inline std::shared_ptr<T> mInstance;
    };

#define SINGLE(T, ...) \
  single<T, ##__VA_ARGS__> { nullptr } = [=]() -> T 

  template <typename ...Signature>
    struct get {
      get() = default;

      template <typename T>
        operator T () const {
          if (instantiation<T, Signature...>::type == SINGLE) {
            if constexpr(SharedPtrConcept<T>) {
              return single<T, Signature...>::get();
            } else {
              throw std::runtime_error("jinject::single instantiation must use shared smart pointer");
            }
          } else if (instantiation<T, Signature...>::type == FACTORY) {
            return factory<T, Signature...>::get();
          }

          std::ostringstream o;

          o << "jinject::undefined instantiation [" << details::demangle(typeid(T).name());

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
    }
}
