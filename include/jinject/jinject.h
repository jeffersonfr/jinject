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

  template <typename T, typename ...Args>
  T inject(Args ...args) {
      if constexpr (!std::is_pointer_v<T>) {
          return T{std::forward<Args>(args)...};
      } else {
          using type = typename std::pointer_traits<T>::element_type;

          return new type{std::forward<Args>(args)...};
      }
  }

  template <typename ...Args>
  struct get {
      get(Args ...args): mArgs{args...} {
      }

      template <typename T>
      operator T () {
          return std::apply(inject<T, Args...>, mArgs);
      }

      template <typename T>
      operator std::shared_ptr<T> () {
          T *value = std::apply(inject<T*, Args...>, mArgs);

          std::shared_ptr<T> valuePtr{value};

          return valuePtr;
      }

      template <typename T>
      operator std::unique_ptr<T> () {
          T *value = std::apply(inject<T*, Args...>, mArgs);

          std::unique_ptr<T> valuePtr{value};

          return valuePtr;
      }

      private:
          std::tuple<Args...> mArgs;
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

