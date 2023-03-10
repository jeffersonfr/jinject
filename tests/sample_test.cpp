#include "jinject/jinject.h"

#include <iostream>

//#include <gtest/gtest.h>

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

namespace jinject {
  template <>
    NoDefaultConstructor inject(int i, std::string s) {
      std::cout << "inject<custom injection>: " << i << ", " << s << "\n";

      return {i*i, s};
    }
}

void sample(
    int primitiveType = get{42},
    DefaultConstructor defaultConstructor = get{},
    NoDefaultConstructor noDefaultConstructor = get{42, "jeff"}) {
  std::cout << "sample called\n";
}

void sample_ptr(
    int *primitiveType = get{42},
    DefaultConstructor *defaultConstructor = get{},
    NoDefaultConstructor *noDefaultConstructor = get{42, "jeff"}) {
  std::cout << "sample_ptr called\n";
}

void sample_shared(
    std::shared_ptr<int> primitiveType = get{42},
    std::shared_ptr<DefaultConstructor> defaultConstructor = get{},
    std::shared_ptr<NoDefaultConstructor> noDefaultConstructor = get{42, "jeff"}) {
  std::cout << "sample_shared called\n";
}

void sample_unique(
    std::unique_ptr<int> primitiveType = get{42},
    std::shared_ptr<DefaultConstructor> defaultConstructor = get{},
    std::shared_ptr<NoDefaultConstructor> noDefaultConstructor = get{42, "jeff"}) {
  std::cout << "sample_unique called\n";
}

struct injection_test : injection<DefaultConstructor> {
  injection_test() {
    std::cout << "injection_test called\n";

    auto obj = dependency<DefaultConstructor>::instance();
  }
};

int main() {
  injection_test test{};

  int const * const pInt = new int{42};

  bind<DefaultConstructor>::set("type", "debug");
  bind<DefaultConstructor>::set("index", 42);
  bind<DefaultConstructor>::set("ptr", pInt);

  int i = *bind<DefaultConstructor>::get<int>("index");

  sample();
  sample_ptr();
  sample_shared();
  sample_unique();
}

