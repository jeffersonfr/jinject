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
    IFoo * inject(int id) {
        struct Impl1 : public IFoo {
            void foo() {
            }
        };

        struct Impl2 : public IFoo {
            void foo() {
            }
        };

        if (id == 0) {
            return new Impl1{};
        }

        return new Impl2{};
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
    MyUseCase(IFoo *foo = get{1}, IBar *bar = get{}) {
        std::cout << "foo: " << std::addressof(foo) << ", bar: " << std::addressof(bar) << std::endl;

        delete foo;
        delete bar;
    }
};

int main() {
    MyUseCase useCase;
}

