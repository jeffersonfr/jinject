#include "jinject/jinject.h"

#include <iostream>

#include <gtest/gtest.h>

using namespace jinject;

struct DefaultConstructor {
  DefaultConstructor() = default;
};

struct NoDefaultConstructor {
  NoDefaultConstructor(int i, std::string s) {
  }
};

struct SingleInstantiation {
};

struct UndefinedInstantiation {
};

struct SharedInstantiation {
};

struct UniqueInstantiation {
};

struct SignatureType1 {
};

struct SignatureType2 {
};

struct CustomInstantiation {
  CustomInstantiation(int value): mValue{value} {
  }

  int mValue{0};
};

class Environment : public ::testing::Environment {
  
  public:
    ~Environment() override {
      
    }

    void SetUp() override {
      LoadModules();
    }

    void TearDown() override {

    }

    private:
      void LoadNamedModule() {
        NAMED("url", "https://google.com");
      }

      void LoadPrimitiveModule() {
        FACTORY(int) {
          return 42;
        };

        FACTORY(int*) {
          return new int{42};
        };

        FACTORY(std::shared_ptr<int>) {
          return std::make_shared<int>(42);
        };

        FACTORY(std::unique_ptr<int>) {
          return std::make_unique<int>(42);
        };
      }

      void LoadDefaultConstructorModule() {
        FACTORY(DefaultConstructor) {
          return DefaultConstructor{};
        };

        FACTORY(DefaultConstructor*) {
          return new DefaultConstructor{};
        };

        FACTORY(std::shared_ptr<DefaultConstructor>) {
          return std::make_shared<DefaultConstructor>();
        };

        FACTORY(std::unique_ptr<DefaultConstructor>) {
          return std::make_unique<DefaultConstructor>();
        };
      }

      void LoadNoDefaultConstructorModule() {
        FACTORY(NoDefaultConstructor) {
          return NoDefaultConstructor{get{}, "Hello, world !"};
        };

        FACTORY(NoDefaultConstructor*) {
          return new NoDefaultConstructor{get{}, "Hello, world !"};
        };

        FACTORY(std::shared_ptr<NoDefaultConstructor>) {
          return std::make_shared<NoDefaultConstructor>(get{}, "Hello, world !");
        };

        FACTORY(std::unique_ptr<NoDefaultConstructor>) {
          return std::make_unique<NoDefaultConstructor>(get{}, "Hello, world !");
        };
      }

      void LoadSingleInstantiationModule() {
        FACTORY(SingleInstantiation*) {
          return new SingleInstantiation{};
        };

        FACTORY(std::shared_ptr<SingleInstantiation>) {
          return std::make_shared<SingleInstantiation>();
        };
      }

      void LoadSharedInstantiationModule() {
        SHARED(SharedInstantiation) {
          return new SharedInstantiation{};
        };
      }

      void LoadUniqueInstantiationModule() {
        UNIQUE(UniqueInstantiation) {
          return new UniqueInstantiation{};
        };
      }

      void LoadCustomInstantiationModule() {
        FACTORY(CustomInstantiation, SignatureType1) {
          return CustomInstantiation{1};
        };

        FACTORY(CustomInstantiation, SignatureType2) {
          return CustomInstantiation{2};
        };
      }

      void LoadModules() {
        LoadNamedModule();
        LoadPrimitiveModule();
        LoadDefaultConstructorModule();
        LoadNoDefaultConstructorModule();
        LoadSingleInstantiationModule();
        LoadSharedInstantiationModule();
        LoadUniqueInstantiationModule();
        LoadCustomInstantiationModule();
      }

};

// named tests
TEST(InjectionSuite, Named) {
  std::string value = get_string<"url">{};

  ASSERT_EQ(value, "https://google.com");
}

TEST(InjectionSuite, Named2x) {
  try {
    NAMED("url", "https://google.com");
    NAMED("url", "https://google.com");

    FAIL();
  } catch (...) {
  }
}

TEST(InjectionSuite, NamedNotFound) {
  std::string value = get_string<"jeff">{"none"};

  ASSERT_EQ(value, "none");
}

// primitive tests
TEST(InjectionSuite, Primitive) {
  int value = get{};

  ASSERT_EQ(value, 42);
}

TEST(InjectionSuite, PointerPrimitive) {
  int *value = get{};

  ASSERT_EQ(*value, 42);
}

TEST(InjectionSuite, SharedPrimitive) {
  std::shared_ptr<int> value = get{};

  ASSERT_EQ(*value, 42);
}

TEST(InjectionSuite, UniquePrimitive) {
  std::unique_ptr<int> value = get{};

  ASSERT_EQ(*value, 42);
}

// default constructor
TEST(InjectionSuite, DefaultConstructor) {
  DefaultConstructor value = get{};

  SUCCEED();
}

TEST(InjectionSuite, PointerDefaultConstructor) {
  DefaultConstructor *value = get{};

  SUCCEED();
}

TEST(InjectionSuite, SharedDefaultConstructor) {
  std::shared_ptr<DefaultConstructor> value = get{};

  SUCCEED();
}

TEST(InjectionSuite, UniqueDefaultConstructor) {
  std::unique_ptr<DefaultConstructor> value = get{};

  SUCCEED();
}

// no NoDefault constructor
TEST(InjectionSuite, NoDefaultConstructor) {
  NoDefaultConstructor value = get{};

  SUCCEED();
}

TEST(InjectionSuite, PointerNoDefaultConstructor) {
  NoDefaultConstructor *value = get{};

  SUCCEED();
}

TEST(InjectionSuite, SharedNoDefaultConstructor) {
  std::shared_ptr<NoDefaultConstructor> value = get{};

  SUCCEED();
}

TEST(InjectionSuite, UniqueNoDefaultConstructor) {
  std::unique_ptr<NoDefaultConstructor> value = get{};

  SUCCEED();
}

// single instatiation
TEST(InjectionSuite, SingleInstantiation) {
  SingleInstantiation *value = get{};

  SUCCEED();
}

TEST(InjectionSuite, SharedSingleInstantiation) {
  std::shared_ptr<SingleInstantiation> value = get{};

  SUCCEED();
}

// undefined instatiation
TEST(InjectionSuite, UndefinedInstatiation) {
  try {
    UndefinedInstantiation value = get{};

    FAIL();
  } catch (...) {
  }
}

TEST(InjectionSuite, PointerUndefinedInstatiation) {
  try {
    UndefinedInstantiation *value = get{};

    FAIL();
  } catch (...) {
  }
}

TEST(InjectionSuite, SharedUndefinedInstatiation) {
  try {
    std::shared_ptr<UndefinedInstantiation> value = get{};

    FAIL();
  } catch (...) {
  }
}

TEST(InjectionSuite, UniqueUndefinedInstatiation) {
  try {
    std::unique_ptr<UndefinedInstantiation> value = get{};

    FAIL();
  } catch (...) {
  }
}

// shared instatiation
TEST(InjectionSuite, SharedInstantiation) {
  std::shared_ptr<SharedInstantiation> value = get{};

  SUCCEED();
}

TEST(InjectionSuite, SharedInstantiationCompared) {
  std::shared_ptr<SharedInstantiation> value1 = get{};
  std::shared_ptr<SharedInstantiation> value2 = get{};

  if (value1.get() != value2.get()) {
    FAIL();
  }

  SUCCEED();
}

// custom instatiation
TEST(InjectionSuite, CustomInstantiation) {
  CustomInstantiation value1 = get<SignatureType1>{};
  CustomInstantiation value2 = get<SignatureType2>{};

  ASSERT_EQ(value1.mValue, 1);
  ASSERT_EQ(value2.mValue, 2);
}

// mutiple binds
TEST(InjectionSuite, MultipleBind) {
  // multiple bind instantiation
  struct MyType {};

  MyType *ptr1 = new MyType{};
  MyType *ptr2 = new MyType{};
  
  FACTORY(MyType*, SignatureType1) {
    return ptr1;
  };

  FACTORY(MyType*, SignatureType2) {
    return ptr2;
  };

  // list all binds
  std::vector<MyType*> binds = all{};

  if (binds.size() != 2) {
    FAIL();
  }

  ASSERT_EQ(binds[0], ptr1);
  ASSERT_EQ(binds[1], ptr2);
}

// auto return
TEST(InjectionSuite, AutoReturn) {
  decltype(auto) value = inject<int>();

  ASSERT_EQ(value, 42);
}

TEST(InjectionSuite, CastingAutoReturn) {
  long value = inject<int>();

  ASSERT_EQ(value, 42L);
}

// inject with
TEST(InjectionSuite, InjectWith) {
  auto value = inject_by<int>().value_or(21);

  ASSERT_EQ(value, 42);
}

TEST(InjectionSuite, EmptyInjectWith) {
  auto value = inject_by<long>().value_or(21L);

  ASSERT_EQ(value, 21L);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new Environment);

    return RUN_ALL_TESTS();
}
