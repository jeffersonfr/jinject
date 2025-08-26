# Dependency injection library

This library provides mechanisms for resolving and instantiating arbitrary classes or structs at compile time. It supports both automatic instantiation and user-defined construction strategies, enabling precise control over entity creation. The following tutorial outlines the core capabilities and primary usage patterns of the library.

## 1. no dependency injection

In this initial scenario, all entity instantiations are explicitly defined by the developer. The developer is responsible for determining the appropriate construction mechanism, configuring the instances, and managing their complete lifecycle.

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

This example operates on interfaces rather than concrete implementations. In this context, the developer is required to instantiate an interface contract by selecting an appropriate implementing class and determining the instantiation strategy. Such knowledge regarding the mapping between interfaces and their concrete implementations must be explicitly defined and consistently shared across the development team.

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

This example illustrates a basic implementation of a system employing a rudimentary form of dependency injection. In this design, the class declares its dependencies explicitly and delegates the responsibility for their instantiation to an external mechanism. Despite its simplicity, this structure already enables the integration of a testing architecture within the system. Nevertheless, it still inherits the same limitation identified in the previous example.

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

In this example, the previously identified issues are systematically addressed. Consider now a large-scale system comprising numerous entities and use cases, where certain use cases may depend on others. In such scenarios, additions, removals, or structural modifications within the codebase can have cascading effects throughout the system. For instance, altering the instantiation order of a single use case may impact multiple components. Introducing a centralized orchestration mechanism provides a robust solution to these challenges, ensuring consistency while preserving immutability in the system’s public interface.

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

In this example, a mechanism commonly referred to as a Service Locator is employed. The get{} method functions as a service retrieval interface, responsible for resolving and instantiating the required entities. However, its current implementation is highly constrained, as it supports only concrete types that expose a default constructor, thereby limiting its applicability in more complex scenarios.

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

Certain types are intended to have a single instantiation within the project, or at minimum, a single shared instance maintained over a defined period of time. For such cases, the single{} type is provided. The lifetime of these singleton instances is extended as long as active references exist, and the instance is released only once all references have been disposed.

```
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

In certain scenarios, an injected value may require a subsequent type conversion. The example below demonstrates a value of type long that must be retrieved as an int. However, a runtime exception is raised because the API attempts to resolve a long instantiation rather than the previously defined int instance.

```
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
