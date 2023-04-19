#include "jinject/jinject.h"

#include <iostream>

#include <gtest/gtest.h>

using namespace jinject;

struct DefaultConstructor {
  DefaultConstructor() {
    std::cout << "DefaultConstructor:: " << bind<DefaultConstructor>::get<std::string>("type").value_or("unknown") << "\n";
  }
};

struct NoDefaultConstructor {
  NoDefaultConstructor(int i, std::string s) {
    std::cout << "NoDefaultConstructor: " << i << ", " << s << "\n";
  }
};

struct SingleInstance {
  SingleInstance() {
    std::cout << "SingleInstance ctor\n";

    ctor++;
  }

  ~SingleInstance() {
    dtor++;
  }

  inline static int ctor = 0;
  inline static int dtor = 0;
};

namespace jinject {
  template <>
    NoDefaultConstructor inject(int i, std::string s) {
      std::cout << "inject<custom injection>: " << i << ", " << s << "\n";

      return {i*i, s};
    }
}

TEST(InjectionSuite, PrimitiveNoPointerInjection) {
  int value = get{42};

  ASSERT_EQ(value, 42);
}

TEST(InjectionSuite, DefaultConstructorNoPointerInjection) {
  DefaultConstructor value = get{};
}

TEST(InjectionSuite, NoDefaultConstructorNoPointerInjection) {
  NoDefaultConstructor value = get{42, "hello, world"};
}

TEST(InjectionSuite, PrimitivePointerInjection) {
  int *value = get{42};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, DefaultConstructorPointerInjection) {
  DefaultConstructor *value = get{};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, NoDefaultConstructorPointerInjection) {
  NoDefaultConstructor *value = get{42, "hello, world"};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, PrimitiveSharedPointerInjection) {
  std::shared_ptr<int> value = get{42};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, DefaultConstructorSharedPointerInjection) {
  std::shared_ptr<DefaultConstructor> value = get{};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, NoDefaultConstructorSharedPointerInjection) {
  std::shared_ptr<NoDefaultConstructor> value = get{42, "hello, world"};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, PrimitiveUniquePointerInjection) {
  std::unique_ptr<int> value = get{42};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, DefaultConstructorUniquePointerInjection) {
  std::unique_ptr<DefaultConstructor> value = get{};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, NoDefaultConstructorUniquePointerInjection) {
  std::unique_ptr<NoDefaultConstructor> value = get{42, "hello, world"};

  ASSERT_NE(value, nullptr);
}

TEST(InjectionSuite, SingleInstanceSharedPointerInjection) {
  SingleInstance::ctor = 0;
  SingleInstance::dtor = 0;
  
  {
    std::shared_ptr<SingleInstance> value1 = single{};
    std::shared_ptr<SingleInstance> value2 = single{};
    std::shared_ptr<SingleInstance> value3 = single{};
    std::shared_ptr<SingleInstance> value4 = single{};
    std::shared_ptr<SingleInstance> value5 = single{};

    ASSERT_NE(value1, nullptr);
    ASSERT_NE(value2, nullptr);
    ASSERT_NE(value3, nullptr);
    ASSERT_NE(value4, nullptr);
    ASSERT_NE(value5, nullptr);
  }

  ASSERT_EQ(SingleInstance::ctor, 1);
  ASSERT_EQ(SingleInstance::dtor, 1);
}

TEST(InjectionSuite, PrimitiveBind) {
  bind<DefaultConstructor>::set("value", 42);

  ASSERT_EQ(*bind<DefaultConstructor>::get<int>("value"), 42);
}

TEST(InjectionSuite, LazyInstanceSharedPointerInjection) {
  SingleInstance::ctor = 0;
  SingleInstance::dtor = 0;

  {  
    auto object = lazy{};

    std::shared_ptr<SingleInstance> value1 = object;
    std::shared_ptr<SingleInstance> value2 = object;
    std::shared_ptr<SingleInstance> value3 = object;
    std::shared_ptr<SingleInstance> value4 = object;
    std::shared_ptr<SingleInstance> value5 = object;

    ASSERT_NE(value1, nullptr);
    ASSERT_NE(value2, nullptr);
    ASSERT_NE(value3, nullptr);
    ASSERT_NE(value4, nullptr);
    ASSERT_NE(value5, nullptr);
  }

  ASSERT_EQ(SingleInstance::ctor, 1);
  ASSERT_EQ(SingleInstance::dtor, 1);
}

TEST(InjectionSuite, InheritanceInjection) {
  struct InjectionTest : injection<DefaultConstructor> {
    InjectionTest() {
      auto obj = inject<DefaultConstructor>();
    }
  };
}
