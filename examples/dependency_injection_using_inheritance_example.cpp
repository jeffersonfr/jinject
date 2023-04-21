#include "jinject/jinject.h"

#include <iostream>

using namespace jinject;

struct IFoo {
};

struct IBar {
};

struct MyUseCase : injection<IFoo*, IBar*> {
    MyUseCase() {
        auto *bar = inject<IBar*>();
        auto *foo = inject<IFoo*>();

        std::cout << "foo: " << std::addressof(foo) << ", bar: " << std::addressof(bar) << std::endl;
    }
};

int main() {
    MyUseCase useCase;
}

