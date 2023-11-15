# Dependency injection library

This library simplifies finding and instantiating any type of class or struct at compile time. Developers can use automatic or custom instantiation of entities. This short tutorial will show you some of the main features of the library.

## 1. no dependency injection

In this first case, all instantiations are created by the developer. The developer needs to know how to instantiate, configure and manage the lifecycle of entities.

```
    struct Foo {
    };

    struct Bar {
    };

    struct MyUseCase {
        MyUseCase() {
            auto foo = Foo{};
            auto bar = Bar{};
        }
    };
```

## 2. no dependency injection using interfaces

This example works with interfaces rather than concrete entities. In this case, the developer needs to instantiate an interface definition and, for that, he needs to know which class to instantiate and how to instantiate it. All this knowledge must be shared for all developers.

```
    struct IFoo {
    };

    struct IBar {
    };

    struct MyUseCase {
        MyUseCase() {
            auto foo = who implements IFoo ... ?
            auto bar = who implements IBar ... ?
        }
    };
```

## 3. naive dependency injection

Here we have a basic version of a system using some kind of dependency injection. In this example, we have a class that exposes its dependencies and exempts itself from the responsibility of instantiating them. Although simple, this code structure already allows the implementation of a test architecture in the system, however, we continue with the same problem reported in the previous example.

```
    struct IFoo {
    };

    struct IBar {
    };

    struct MyUseCase {
        MyUseCase(IFoo *foo, IBar *bar) {
        }
    };
```

## 4. naive dependency injection with default instantiation

In this example, we simply solve the previously reported problems. Now, let's imagine a large system, with many entities and use cases, sometimes making use of other use cases. Additions, removals, or any other restructuring of the code could affect many other parts of the system. As an example, we can cite the change in the instantiation order of any use case. A centralized entity could easily solve these problems and keep the public part of the code immutable.

```
    #include <iostream>

    struct IFoo {
    };

    struct IBar {
    };

    struct MyUseCase {
        MyUseCase(IFoo *foo = new Foo{}, IBar *bar = Bar{}) {
            std::cout << "foo: " << std::addressof(foo) << ", bar: " << std::addressof(bar) << std::endl;

            delete foo;
            delete bar;
        }
    };
```

## 5. dependency injection with service locator

In this example, we use a mechanism known as a service locator. The `get{}` method works as a 'service finder' and has the function of looking for and instantiating the necessary entities. However, this method works in a very simplified way where only concrete entities with default constructors could be automatically instantiated.

```
    #include "jinject/jinject.h"

    #include <iostream>

    using namespace jinject;

    struct IFoo {
    };

    struct IBar {
    };  

    void LoadModules() {
        FACTORY(IFoo) {
            return new IFoo{};
        };

        FACTORY(IBar) {
            return new IBar{};
        };
    }

    struct MyUseCase {
        MyUseCase(IFoo *foo = get{}, IBar *bar = get{}) {
            std::cout << "foo: " << std::addressof(foo) << ", bar: " << std::addressof(bar) << std::endl;
      
            delete foo;
            delete bar;
        }
    };

    int main() {
        LoadModules();

        MyUseCase useCase;
    }

```

## 6. dependency injection using single instance

Certain types should have only one instanciation in the project or, at least, one instation for shared use over a period of time. For cases like that there is the type single{}. The lifetime of single instances is extended until all instances release the reference.

    #include "jinject/jinject.h"

    #include <iostream>

    using namespace jinject;

    struct IFoo {
    };

    void LoadModules() {
        SINGLE(IFoo) {
            return new IFoo{};
        };
    }

    struct MyUseCase {
        MyUseCase(IFoo *foo = get{}) {
            std::cout << "foo: " << std::addressof(foo) << std::endl;
        }
    };

    int main() {
        LoadModules();

        MyUseCase useCase;
    }

```

## 7. eagle injection

In some situations a injection must be followed by a second convertion of type. The example below shows a value of type 'long' that need get a 'int' value, but the system raises a runtime exception, because the api search for a 'long' instantiation, instead of a 'int' that was defined previously.

    #include "jinject/jinject.h"

    #include <iostream>

    using namespace jinject;

    void LoadModules() {
        FACTORY(int) {
            return 42;
        };
    }

    int main() {
        LoadModules();

				long value = get{}; // compiles, but throws a runtime exeception because there isn't a 'long' instantiation
				long value = inject<int>(); // refers to 'int' instantiation enabling the auto-casting
    }

```
