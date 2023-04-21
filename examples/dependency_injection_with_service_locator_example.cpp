#include "jinject/jinject.h"

#include <iostream>

using namespace jinject;

struct IFoo {
};

struct IBar {
};

struct MyUseCase {
    MyUseCase(IFoo *foo = get{}, IBar *bar = get{}) {
        std::cout << "foo: " << std::addressof(foo) << ", bar: " << std::addressof(bar) << std::endl;

        delete foo;
        delete bar;
    }
};

int main() {
  MyUseCase useCase;
}

