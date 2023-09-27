#ifndef MOONSHINE_SERVER_DATABASE_H_
#define MOONSHINE_SERVER_DATABASE_H_

#include <sqlite3.h>

#include "db/client.h"

struct database_component {

 private:
  std::string database_path;
  std::string resource_path;

 public:

  explicit database_component(const std::string &database_path, const std::string &resource_path) : database_path(database_path), resource_path(resource_path) {
    spdlog::info("Database @ {}", database_path);
  };

  /**
   * Create database connection provider component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, dbConnectionProvider)
  ([this] {

    /* Create database-specific ConnectionProvider */
    auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(database_path);

    auto nativeConnection = std::static_pointer_cast<oatpp::sqlite::Connection>(connectionProvider->get().object);
    sqlite3_exec(nativeConnection->getHandle(), "PRAGMA journal_mode=WAL; PRAGMA synchronous = normal; PRAGMA temp_store = memory; PRAGMA page_size = 32768; PRAGMA mmap_size = 30000000000;", 0, 0, 0);

    /* Create database-specific ConnectionPool */
    return oatpp::sqlite::ConnectionPool::createShared(connectionProvider,
                                                       10 /* max-connections */,
                                                       std::chrono::seconds(5) /* connection TTL */);
  }());

  /**
   * Create database client
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<db::client>, _database)([this] {

    /* Get database ConnectionProvider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, connectionProvider);

    /* Create database-specific Executor */
    auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionProvider);

    /* Create MyClient database client */
    return std::make_shared<db::client>(executor, resource_path, database_path);
  }());
};

#endif //MOONSHINE_SERVER_DATABASE_H_
