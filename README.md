# Dependency injection library

This library simplify localization and instantiation of any type of classe or struct at compile time. The developers could use the automatic or customized instantiation of entities. This short tutorial will show some of the mainly functionalities of the library.

1. test/no dependency injection

In this case all instantiation are created by the developer. The developter needs to known how instantiate, configurate and manage the lifecycle of the entities. Now, what is happen if the entity is an interface ?

```struct Foo {
    };

    struct Bar {
    };

    struct UseCase {
        UseCase() {
            auto foo = Foo{};
            auto bar = Bar{};
        }
    };
```

2. test/no dependency injection using interfaces

This example uses interfaces instead of concrete entities. The problem here is that the developer probably unknown the concrete implementation of the entity and the it's necessary that the concrete entity be found. Let's suppose that all these requirements are suppressed by the developers and think about testing issues. How are developers going to test these internal entities ?

```struct IFoo {
    };

    struct IBar {
    };

    struct UseCase {
        UseCase() {
            auto foo = who implements IFoo ... ?
            auto bar = who implements IBar ... ?
        }
    };
```

3. test/naive dependency injection

With this naive implementation the developer already can using test facilities to observe intenal aspects of the use case (in this case). All that the use case needs is show its dependencies in form of interfaces. Ok, alright ... but, the developer still instantiating manually the interfaces and put alltogheter in the right way. Problems should occurs If one or more arguments swap positions or be removed from entitiy's constructor.

```struct IFoo {
    };

    struct IBar {
    };

    struct UseCase {
        UseCase(IFoo *foo, IBar *bar) {
        }
    };
```

4. test/naive dependency injection with default instantiation

The next use case resolve some issues related above in a straightforward way. Now, imagine a huge system with a lot of use cases and a lot of dependencies. In this scenario, some creation could be refactored and changed its behavior for this new implementation, so different parts of the system could begin to work differently even using the same "instance" of the entity. A very knowning way to fix this problem is using some kind of service to locate the entities and do its instantiation (mapping interfaces/implementations)

```struct IFoo {
    };

    struct IBar {
    };

    struct UseCase {
        UseCase(IFoo *foo = new Foo{}, IBar *bar = Bar{}) {
            ...

            delete foo;
            delete bar;
        }
    };
```

5. test/dependency injection with service locator

The use case is now using the service locator. The `get{}` will searching for a interface and try to instantiate.This will work only for non-interfaces and non-abstract entities. For unconstructable entities, the developer could specialize the construction of the entity.

```#include <jinject/jinject.h>

    using jinject;

    struct IFoo {
    };

    struct IBar {
    };

    struct UseCase {
        UseCase(IFoo *foo = get{}, IBar *bar = get{}) {
            ...

            delete foo;
            delete bar;
        }
    };
```

6. test/dependency injection with custom service locator

This test uses a virtual entity and a custom service locator. some code was written to enable the instantiation, but this code is unique for all parts of the code with a well defined construction that is transparent for all developers. Now, could the developers specifying different instantiations to a same interface ?

```#include <jinject/jinject.h>

    using jinject;

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

            return new Impl;
        }

        template <>
        IBar * inject() {
            struct Impl : public IBar {
                void bar() {
                }
            };
        }
    }

    struct UseCase {
        UseCase(IFoo *foo = get{}, IBar *bar = get{}) {
            ...

            delete foo;
            delete bar;
        }
    };
```

7. test/dependency injection with custom service locator

A custom injection could receive parameters during the instantiation process. Then, a developer could create different behaviors from different parameters. So now, we have a entity using dependency injection, however the developer must change its construction for this. Is there any way to injection of dependencies without change the interface of a entity ?

```#include <jinject/jinject.h>

    using jinject;

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
                return new Impl1;
            }

            return new Impl2
        }

        template <>
        IBar * inject() {
            struct Impl : public IBar {
                void bar() {
                }
            };
        }
    }

    struct UseCase {
        UseCase(IFoo *foo = get{1}, IBar *bar = get{}) {
            ...

            delete foo;
            delete bar;
        }
    };
```

8. test/dependency injection at compile time

Using `jinject::injection<Interface...>` the developers could inject any dependency, in any order without change the interface of the entity. Using this struct the developers could to change the dependencies and change the behavior of entity at compile time, in addition to not having to take care of the lifecycle of the instances.

```#include <jinject/jinject.h>

    using jinject;

    struct IFoo {
    };

    struct IBar {
    };

    struct UseCase : injection<IFoo*, IBar*> {
        UseCase() {
            ...
            
            auto *bar = dependency<IBar*>::instance();
            auto *foo = dependency<IFoo*>::instance();
        }
    };
```

