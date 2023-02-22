#pragma once

#include <type_traits>
#include <tuple>
#include <memory>
#include <unordered_map>
#include <any>
#include <optional>

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
    concept NoSmartPtrConcept = !SharedPtrConcept<T> and !UniquePtrConcept<T>;

  template <typename T, typename ...Args>
    requires (NoSmartPtrConcept<T>)
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

  template <typename ...Args>
    struct get {
      get(Args ...args): mArgs{args...} {
      }

      template <typename T>
        operator T () {
          return std::apply(inject<T, typename ConvertAndFilterType<Args>::type...>, mArgs);
        }

      private:
      	std::tuple<Args...> mArgs;
    };


  template <typename T>
    struct dependency {
      virtual ~dependency() {
        if constexpr (std::is_pointer_v<T>) {
          delete mInstance; // TODO:: cause trouble with singleton instances
        }
      }

      T instance() {
        return mInstance;
      }

      private:
      	T mInstance = get{};
    };

  template <typename ...Args>
    class injection : protected dependency<Args>... {
    };

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

      private:
        static inline std::unordered_map<std::string, std::any> mValues;
    };
}

