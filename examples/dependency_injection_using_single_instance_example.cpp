#include "jinject/jinject.h"

#include <iostream>

using namespace jinject;

struct IFoo {
};

struct MyUseCase {
    MyUseCase() {
        std::shared_ptr<IFoo> foo = single();

        std::cout << "foo: " << std::addressof(foo) << std::endl;
    }
};

int main() {
    MyUseCase useCase;
}

