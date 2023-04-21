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

```

## 6. dependency injection with custom service locator

Here we use the location service customization feature described in the previous example. We extended the API so that it could know what to do, for example, when an instance of an interface was requested, or then how many instances would be created and/or shared during the execution of the system.

```
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
```

## 7. dependency injection with custom instantiation

In this example, we present a way to customize/customize the location service to handle complex issues such as entity instances with a particular set of parameters or return different instances given a different type or concept, that is, it shows a way to change the behavior of the location service from different execution contexts.

```
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
```

## 8. dependency injection using inheritance

This example shows how to implement dependency injection using inheritance. With this architectural model, developers would not need to change the use case interface and would still have the option of implementing dynamic dependency injection mechanisms at compile time.

```
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
```

## 9. dependency injection using single instance

Certain types should have only one instanciation in the project or, at least, one instation for shared use over a period of time. For cases like that there is the type single{}. The lifetime of single instances is extended until all instances release the reference.

```
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
```

## 10. dependency injection using lazy loading

Types that load a lot of objects should use a lazy type to avoid heavy latency during instantiation. The lazy type will create a instance only one time, the first time called (for a new type). The lifetime of the reference will be managed by lazy object for raw pointers. For shared pointer the lifetime could be extended.

```
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
```
