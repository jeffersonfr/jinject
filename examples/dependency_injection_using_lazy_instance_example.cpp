#include "jinject/jinject.h"

#include <iostream>

using namespace jinject;

struct IFoo {
};

struct MyUseCase {
    MyUseCase() {
        std::shared_ptr<IFoo> foo = lazyFoo;

        std::cout << "foo: " << std::addressof(foo) << std::endl;
    }

    private:
        lazy<> lazyFoo = lazy{};
};

int main() {
    MyUseCase useCase;
}

