#pragma once

#include <type_traits>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <any>
#include <optional>
#include <iostream>

namespace jinject {
  template <typename T>
    struct ConvertAndFilterType {
      using type = T;
    };

  template <std::size_t N>
    struct ConvertAndFilterType<char const [N]> {
      using type = std::string;
    };

  template <>
    struct ConvertAndFilterType<char const []> {
      using type = std::string;
    };

  template <>
    struct ConvertAndFilterType<char const *> {
      using type = std::string;
    };

  template <typename T>
    concept SharedPtrConcept = std::same_as<std::shared_ptr<typename T::element_type>, T>;

  template <typename T>
    concept UniquePtrConcept = std::same_as<std::unique_ptr<typename T::element_type>, T>;

  template <typename T>
    concept SmartPtrConcept = SharedPtrConcept<T> or UniquePtrConcept<T>;

  template <typename T>
    concept PointerConcept = SmartPtrConcept<T> or std::is_pointer<T>::value;

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

  template <typename T, typename ...Args>
    struct bind {
      public:
        template <typename Value>
          static void set(std::string key, Value value) {
            using type = typename ConvertAndFilterType<Value>::type;

            mValues[key] = static_cast<type>(value);
          }

        template <typename Value>
          static std::optional<typename ConvertAndFilterType<Value>::type> get(std::string key) {
            using type = typename ConvertAndFilterType<Value>::type;

            decltype(mValues)::const_iterator i = mValues.find(key);

            if (i != mValues.end()) {
              return std::any_cast<type>(mValues[key]);
            }

            return {};
          }

          static void clear(std::string key) {
            mValues.erase(key);
          }
      protected:
        bind() = default;

      private:
        static inline std::unordered_map<std::string, std::any> mValues;
    };

  template <typename ...Args>
    struct get {
      get(Args ...args): mArgs{args...} {
      }

      template <typename T>
        operator T () const {
          return std::apply(inject<T, typename ConvertAndFilterType<Args>::type...>, mArgs);
        }

      private:
      	std::tuple<Args...> mArgs;
    };

  template <typename T>
    struct single_base {
      operator T () const {
        using type = typename T::element_type;

        static std::weak_ptr<type> head;

        std::shared_ptr<type> previousPtr = head.lock();

        if (previousPtr != nullptr) {
          return previousPtr;
        }

        T instance = static_cast<T>(get{});

        head = instance;

        return instance;
      }
    };

  struct single {
    template <typename T>
      requires (SharedPtrConcept<T>)
      operator T () const {
        return single_base<T>{};
      }
    };

  template <typename ...Args>
    struct lazy {
      lazy(Args ...args): mArgs{args...} {
      }

      template <typename T>
        requires (std::is_pointer_v<T>)
        operator T () const {
          using type = std::remove_pointer_t<std::decay_t<T>>;

          std::shared_ptr<type> head;

          try {
            std::shared_ptr<type> head = std::any_cast<std::shared_ptr<type>>(mValue);

            if (head != nullptr) {
              return head.get();
            }
          } catch (std::bad_any_cast &e) {
          }

          T instance = std::make_from_tuple<get<Args...>>(mArgs);

          mValue = std::shared_ptr<type>{instance};
          
          return instance;
        }

      template <typename T>
        requires (SharedPtrConcept<T>)
        operator T () const {
          using type = typename T::element_type;

          std::weak_ptr<type> head;

          try {
            head = std::any_cast<std::weak_ptr<type>>(mValue);

            std::shared_ptr<type> previousPtr = head.lock();

            if (previousPtr != nullptr) {
              return previousPtr;
            }
          } catch (std::bad_any_cast &e) {
          }

          T instance = std::make_from_tuple<get<Args...>>(mArgs);

          mValue = std::weak_ptr{instance};

          return instance;
        }

      private:
      	std::tuple<Args...> mArgs;

        mutable std::any mValue;
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