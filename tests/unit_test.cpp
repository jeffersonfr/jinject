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
        LoadPrimitiveModule();
        LoadDefaultConstructorModule();
        LoadNoDefaultConstructorModule();
        LoadSingleInstantiationModule();
        LoadSharedInstantiationModule();
        LoadUniqueInstantiationModule();
        LoadCustomInstantiationModule();
      }

};

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

// unique instatiation
TEST(InjectionSuite, UniqueInstantiation) {
  std::unique_ptr<UniqueInstantiation> value = G;

  SUCCEED();
}

// custom instatiation
TEST(InjectionSuite, CustomInstantiation) {
  CustomInstantiation value1 = get<SignatureType1>{};
  CustomInstantiation value2 = get<SignatureType2>{};

  ASSERT_EQ(value1.mValue, 1);
  ASSERT_EQ(value2.mValue, 2);
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new Environment);

    return RUN_ALL_TESTS();
}