#include "jinject/jinject.h"

#include <iostream>

using namespace jinject;

struct IFoo {
    virtual void foo() = 0;
};

struct IBar {
    virtual void bar() = 0;
};

namespace jinject {
    template <>
        IFoo * inject() {
            struct Impl : public IFoo {
                void foo() {
                }
            };

            return new Impl{};
        }

    template <>
        IBar * inject() {
            struct Impl : public IBar {
                void bar() {
                }
            };

            return new Impl{};
        }
}

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

