#pragma once

#include <type_traits>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <any>
#include <optional>
#include <functional>
#include <iostream>

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

  template <typename T, typename ...Signature>
  struct instantiation {
    static inline instantiation_type type = UNKNOWN;

    instantiation() {
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
          mCallback = callback;

          instantiation<T, Signature...>::type = FACTORY;
        }

      static T get() {
        return mCallback();
      }

      factory & operator = (std::function<T()> const &callback) {
        mCallback = callback;

        return *this;
      }

      private:
        static inline std::function<T()> mCallback;
    };

#define FACTORY(T, ...) \
  factory<T, ##__VA_ARGS__> {[]() -> T { throw 0; }} = []() -> T 

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
          mInstance = callback();

          instantiation<std::shared_ptr<T>, Signature...>::type = SINGLE;
        }

      static std::shared_ptr<T> const get() {
        return mInstance;
      }

      single & operator = (std::function<std::shared_ptr<T>()> const &callback) {
        mInstance = callback();

        return *this;
      }

      private:
        static inline std::shared_ptr<T> mInstance;
    };

#define SINGLE(T, ...) \
  single<T, ##__VA_ARGS__> {[]() -> T { throw 0; }} = []() -> T 

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

          throw std::runtime_error("jinject::undefined instantiation");
        }
    };
}

#define G get{}