#include "jinject/jinject.h"

#include <iostream>

//#include <gtest/gtest.h>

using namespace jinject;

struct IQuery {
  virtual void dump() = 0;
};

struct IConnection {
  virtual IQuery * query(std::string) = 0;
};

struct IDatabase {
  virtual bool open() = 0;
  virtual IConnection * acquire() = 0;
};

struct QueryImpl : public IQuery {
  void dump() {
    std::cout << "dump() called\n";
  }
};

struct ConnectionImpl : public IConnection {
  virtual IQuery * query(std::string) {
    return new QueryImpl{};
  }
};

struct DatabaseImpl : public IDatabase {
  bool open() {
    return true;
  }

  IConnection * acquire() {
    return new ConnectionImpl{};
  }
};


struct MyUseCase {
  void listUsers() {
    IQuery *query = get{"select * from users"};

    query->dump();

    delete query;
  }

  void listUsersById(int id) {
    IQuery *query = get{"select * from users where id = " + std::to_string(id)};

    query->dump();

    delete query;
  }
};

namespace jinject {
  template <>
    IConnection * inject() {
      std::cout << "inject<custom IDatabase>\n";

      static DatabaseImpl db{};

      if (db.open() == false) {
        return nullptr;
      }

      return db.acquire();
    }
}

namespace jinject {
  template <>
    IQuery * inject(std::string sql) {
      std::cout << "inject<custom IQuery>\n";

      IConnection *conn = get{};

      IQuery *query = conn->query(sql);

      delete conn;

      return query;
    }
}

// ---------------------------------- my code

int main() {
  MyUseCase useCase = get{};

  useCase.listUsers();
}

