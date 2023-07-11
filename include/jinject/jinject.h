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
    
/*
  template <typename T, typename ...Args>
    requires (!SmartPtrConcept<T>)
    T inject(Args ...args) {
      if constexpr (!std::is_pointer_v<T>) {
        return T{std::forward<Args>(args)...};
      } else {
        using type = typename std::pointer_traits<T>::element_type;

        return new type{std::forward<Args>(args)...};
      }
    }

  template <typename T, typename ...Args>
    requires SharedPtrConcept<T>
    T inject(Args ...args) {
      using Value = typename T::element_type;

      return std::make_shared<Value>(std::forward<Args>(args)...);
    }

  template <typename T, typename ...Args>
    requires UniquePtrConcept<T>
    T inject(Args ...args) {
      using Value = typename T::element_type;

      return std::make_unique<Value>(std::forward<Args>(args)...);
    }
*/

  enum instantiation_type {
    UNKNOWN,
    SINGLE,
    FACTORY
  };

  template <typename T>
  struct instantiation {
    static inline instantiation_type type = UNKNOWN;

    instantiation() {
      if (type != UNKNOWN) {
        throw std::runtime_error("jinject::instantiation already defined");
      }
    }
  };

  template <typename T>
    struct factory: instantiation<T> {
      factory(factory const &) = delete;
      factory(factory &&) = delete;

      template <typename ...Args>
        factory(std::function<T()> callback): instantiation<T>() {
          mCallback = callback;

          instantiation<T>::type = FACTORY;
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

#define FACTORY(T) \
  factory<T> {[]() -> T { throw 0; }} = []() -> T 

  template <typename T>
    struct single: instantiation<T> {
      single(single const &) = delete;
      single(single &&) = delete;
    };

  template <typename T>
    struct single<T*>: instantiation<T*> {
      single(single const &) = delete;
      single(single &&) = delete;

      template <typename ...Args>
        single(std::function<T*()> callback): instantiation<T*>() {
          mInstance = callback();

          instantiation<T*>::type = SINGLE;
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

  template <typename T>
    struct single<std::shared_ptr<T>>: instantiation<std::shared_ptr<T>> {
      single(single const &) = delete;
      single(single &&) = delete;

      template <typename ...Args>
        single(std::function<std::shared_ptr<T>()> callback): instantiation<std::shared_ptr<T>>() {
          mInstance = callback();

          instantiation<std::shared_ptr<T>>::type = SINGLE;
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

#define SINGLE(T) \
  single<T> {[]() -> T { throw 0; }} = []() -> T 

  struct get {
    get() = default;

    template <typename T>
      operator T () const {
        if (instantiation<T>::type == SINGLE) {
          if constexpr(SharedPtrConcept<T>) {
            return single<T>::get();
          } else {
            throw std::runtime_error("jinject::single instantiation must use shared smart pointer");
          }
        } else if (instantiation<T>::type == FACTORY) {
          return factory<T>::get();
        }

        throw std::runtime_error("jinject::undefined instantiation");
      }
  };

  template <typename T>
    struct dependency_base {
      virtual ~dependency_base() {
      }

      T instance() {
        return mInstance;
      }

      protected:
      	T mInstance = get{};
    };

  template <typename T>
    struct dependency : public dependency_base<T> {
      virtual ~dependency() {
        if constexpr (std::is_pointer_v<T>) {
          delete this->mInstance;
        }
      }
    };

  template <typename ...Args>
    struct injection : protected dependency<Args>... {
      template <typename T>
        T inject() {
          return dependency<T>::instance();
        }
    };
}
