#ifndef MOONSHINE_SERVER_DB_CLIENT_H_
#define MOONSHINE_SERVER_DB_CLIENT_H_

//#include <iostream>

#include <spdlog/spdlog.h>
#include <oatpp-sqlite/orm.hpp>

#include "dto/argument.h"
#include "dto/implant.h"
#include "dto/library.h"
#include "dto/listener.h"
#include "dto/task.h"
#include "dto/job.h"

namespace db {

#include OATPP_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

struct client : public oatpp::orm::DbClient {

  explicit client(const std::shared_ptr<oatpp::orm::Executor> &executor, const std::string &resource_path, const std::string &database_path) : oatpp::orm::DbClient(executor) {
    oatpp::orm::SchemaMigration migration(executor);
    migration.addFile(1 /* start from version 1 */, resource_path + "/migration/001_init.sql");
    // Add more migrations here...
    migration.migrate(); // <-- run migrations. This will throw on error.

    auto version = executor->getSchemaVersion();
    spdlog::info("Database version @ {}", version);
  }

  bool truncateTables() {
    bool success = true;
    std::vector<std::string> queries {
        "DELETE FROM argument;",
        "DELETE FROM implant;",
        "DELETE FROM library;",
        "DELETE FROM service;",
        "DELETE FROM listener;",
        "DELETE FROM task;",
        "DELETE FROM job;",
        "DELETE FROM config;"
    };
    for (auto& query : queries) {
      auto result = executeQuery(query, {} /* empty params map */);
      if (!result->isSuccess()) {
        success = false;
        spdlog::error("Error executing query, {}", result->getErrorMessage()->c_str());
      }
    }
    return success;
  };

  bool dropTables() {
    bool success = true;
    std::vector<std::string> queries {
        "DROP TABLE IF EXISTS argument;",
        "DROP TABLE IF EXISTS implant;",
        "DROP TABLE IF EXISTS library;",
        "DROP TABLE IF EXISTS service;",
        "DROP TABLE IF EXISTS listener;",
        "DROP TABLE IF EXISTS task;",
        "DROP TABLE IF EXISTS job;",
        "DROP TABLE IF EXISTS config;"
    };
    for (auto& query : queries) {
      auto result = executeQuery(query, {} /* empty params map */);
      if (!result->isSuccess()) {
        success = false;
        spdlog::error("Error executing query, {}", result->getErrorMessage()->c_str());
      }
    }
    return success;
  }

  /* argument */

  QUERY(createArgument,
        "INSERT INTO argument"
        "(owner, position, data) VALUES "
        "(:owner, :position, :argument.data);",
        PARAM(oatpp::String, owner),
        PARAM(oatpp::Int32, position),
        PARAM(oatpp::Object<dto::argument>, argument));

  QUERY(getArgumentsByOwnerId,
        "SELECT data FROM argument WHERE owner=:owner ORDER BY position ASC;",
        PARAM(oatpp::String, owner));

  QUERY(deleteArgumentsByOwnerId,
        "DELETE FROM argument WHERE owner=:owner;",
        PARAM(oatpp::String, owner));

  QUERY(getArgumentByRowId,
        "SELECT data FROM argument WHERE ROWID=:rowid;",
        PARAM(oatpp::Int64, rowid));

  /* implant */

  QUERY(createImplant,
        "INSERT INTO implant"
        "(id, listener, name, public_key) VALUES "
        "(:implant.id, :implant.listener, :implant.name, :implant.public_key);",
        PARAM(oatpp::Object<dto::implant>, implant));

  QUERY(getImplantById,
        "SELECT * FROM implant WHERE id=:id;",
        PARAM(oatpp::String, id));

  QUERY(getImplants,
        "SELECT * FROM implant LIMIT :limit OFFSET :offset;",
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  QUERY(getImplantsByListener,
        "SELECT * FROM implant LIMIT :limit OFFSET :offset WHERE listener=:listener;",
        PARAM(oatpp::String, listener),
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  QUERY(getImplantByRowId,
        "SELECT * FROM implant WHERE ROWID=:rowid;",
        PARAM(oatpp::Int64, rowid));

  QUERY(deleteImplantById,
        "DELETE FROM implant WHERE id=:id;",
        PARAM(oatpp::String, id));

  QUERY(updateImplant,
        "UPDATE implant "
        "SET "
        " architecture=:architecture, operating_system=:operating_system, "
        " process_id=:process_id, process_user=:process_user, "
        " process_path=:process_path, system_name=:system_name, "
        " system_addrs=:system_addrs "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::Int32, architecture),
        PARAM(oatpp::Int32, operating_system),
        PARAM(oatpp::Int32, process_id),
        PARAM(oatpp::String, process_user),
        PARAM(oatpp::String, process_path),
        PARAM(oatpp::String, system_name),
        PARAM(oatpp::String, system_addrs));

  /* library */

  QUERY(createLibrary,
        "INSERT INTO library"
        "(owner, type, name, data) VALUES "
        "(:owner, :library.type, :library.name, :library.data);",
        PARAM(oatpp::String, owner),
        PARAM(oatpp::Object<dto::library>, library));

  QUERY(getLibrariesByOwnerId,
        "SELECT type, name, data FROM library WHERE owner=:owner;",
        PARAM(oatpp::String, owner));

  QUERY(deleteLibrariesByOwnerId,
        "DELETE FROM library WHERE owner=:owner;",
        PARAM(oatpp::String, owner));

  QUERY(getLibraryByRowId,
        "SELECT type, name, data FROM library WHERE ROWID=:rowid;",
        PARAM(oatpp::Int64, rowid));

  /* listener */

  QUERY(createListener,
        "INSERT INTO listener"
        "(id, name, status, package, implant_package, implant_connection_string) VALUES "
        "(NULL, :name, :status, :package, :implant_package, :implant_connection_string);",
        PARAM(oatpp::String, name),
        PARAM(oatpp::UInt32, status),
        PARAM(oatpp::String, package),
        PARAM(oatpp::String, implant_package),
        PARAM(oatpp::String, implant_connection_string));

  QUERY(updateListenerWithStatus,
        "UPDATE listener "
        "SET "
        " status=:status "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::UInt32, status));

  QUERY(getListenerById,
        "SELECT * FROM listener WHERE id=:id;",
        PARAM(oatpp::String, id));

  QUERY(getListeners,
        "SELECT * FROM listener LIMIT :limit OFFSET :offset;",
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  QUERY(getStartedListeners,
        "SELECT * FROM listener WHERE status=1");

  QUERY(deleteListenerById,
        "DELETE FROM listener WHERE id=:id;",
        PARAM(oatpp::String, id));

  QUERY(getListenerByRowId,
        "SELECT * FROM listener WHERE ROWID=:rowid;",
        PARAM(oatpp::Int64, rowid));

  /* service */

  QUERY(createService,
    "INSERT INTO service"
    "(id, name, status, package) VALUES "
    "(NULL, :name, :status, :package);",
    PARAM(oatpp::String, name),
    PARAM(oatpp::UInt32, status),
    PARAM(oatpp::String, package));

  QUERY(updateServiceWithStatus,
    "UPDATE service "
    "SET "
    " status=:status "
    "WHERE "
    " id=:id;",
    PARAM(oatpp::String, id),
    PARAM(oatpp::UInt32, status));

  QUERY(getServiceById,
    "SELECT * FROM service WHERE id=:id;",
    PARAM(oatpp::String, id));

  QUERY(getServices,
    "SELECT * FROM service LIMIT :limit OFFSET :offset;",
    PARAM(oatpp::UInt32, offset),
    PARAM(oatpp::UInt32, limit));

  QUERY(getStartedServices,
    "SELECT * FROM service WHERE status=1");

  QUERY(deleteServiceById,
    "DELETE FROM service WHERE id=:id;",
    PARAM(oatpp::String, id));

  QUERY(getServiceByRowId,
    "SELECT * FROM service WHERE ROWID=:rowid;",
    PARAM(oatpp::Int64, rowid));

  /* task */

  QUERY(createTask,
        "INSERT INTO task "
        "(id, implant, status, script, success, output) VALUES "
        "(NULL, :implant, :status, :script, :success, :output);",
        PARAM(oatpp::String, implant),
        PARAM(oatpp::Enum<dto::task_status>::AsNumber, status),
        PARAM(oatpp::String, script),
        PARAM(oatpp::Enum<dto::task_success>::AsNumber, success),
        PARAM(oatpp::String, output));

  QUERY(createTaskWithId,
        "INSERT INTO task "
        "(id, implant, status, script, success, output) VALUES "
        "(:id, :implant, :status, :script, :success, :output);",
        PARAM(oatpp::String, id),
        PARAM(oatpp::String, implant),
        PARAM(oatpp::Enum<dto::task_status>::AsNumber, status),
        PARAM(oatpp::String, script),
        PARAM(oatpp::Enum<dto::task_success>::AsNumber, success),
        PARAM(oatpp::String, output));

  QUERY(updateTaskWithStatus,
        "UPDATE task "
        "SET "
        " status=:status "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::Enum<dto::task_status>::AsNumber, status));

  QUERY(updateTask,
        "UPDATE task "
        "SET "
        " status=:status, success=:success, output=:output "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::Enum<dto::task_status>::AsNumber, status),
        PARAM(oatpp::Enum<dto::task_success>::AsNumber, success),
        PARAM(oatpp::String, output));

  QUERY(getTaskByRowId,
        "SELECT * FROM task WHERE ROWID=:rowid;",
        PARAM(oatpp::Int64, rowid));

  QUERY(getTaskById,
        "SELECT * FROM task WHERE id=:id;",
        PARAM(oatpp::String, id));

  QUERY(getTasksByImplant,
        "SELECT * FROM task WHERE implant=:implant_id LIMIT :limit OFFSET :offset;",
        PARAM(oatpp::String, implant_id),
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  QUERY(getTasksByImplantWhereStatus,
        "SELECT * FROM task WHERE implant=:implant_id AND status=:status LIMIT :limit OFFSET :offset;",
        PARAM(oatpp::String, implant_id),
        PARAM(oatpp::Enum<dto::task_status>::AsNumber, status),
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  /* job */

  QUERY(createJob,
        "INSERT INTO job "
        "(id, name, implant, status, package, success, output) VALUES "
        "(NULL, :name, :implant, :status, :package, :success, :output);",
        PARAM(oatpp::String, name),
        PARAM(oatpp::String, implant),
        PARAM(oatpp::Enum<dto::job_status>::AsNumber, status),
        PARAM(oatpp::String, package),
        PARAM(oatpp::Enum<dto::job_success>::AsNumber, success),
        PARAM(oatpp::String, output));

  QUERY(createJobWithId,
        "INSERT INTO job "
        "(id, name, implant, status, package, success, output) VALUES "
        "(:id, :name, :implant, :status, :package, :success, :output);",
        PARAM(oatpp::String, id),
        PARAM(oatpp::String, name),
        PARAM(oatpp::String, implant),
        PARAM(oatpp::Enum<dto::job_status>::AsNumber, status),
        PARAM(oatpp::String, package),
        PARAM(oatpp::Enum<dto::job_success>::AsNumber, success),
        PARAM(oatpp::String, output));

  QUERY(updateJobWithStatus,
        "UPDATE job "
        "SET "
        " status=:status "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::Enum<dto::job_status>::AsNumber, status));

  QUERY(updateJob,
        "UPDATE job "
        "SET "
        " status=:status, success=:success, output=:output "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::Enum<dto::job_status>::AsNumber, status),
        PARAM(oatpp::Enum<dto::job_success>::AsNumber, success),
        PARAM(oatpp::String, output));

  QUERY(updateJobOutput,
        "UPDATE job "
        "SET "
        " output=:output "
        "WHERE "
        " id=:id;",
        PARAM(oatpp::String, id),
        PARAM(oatpp::String, output));

  QUERY(getJobByRowId,
        "SELECT * FROM job WHERE ROWID=:rowid;",
        PARAM(oatpp::Int64, rowid));

  QUERY(getJobById,
        "SELECT * FROM job WHERE id=:id;",
        PARAM(oatpp::String, id));

  QUERY(getJobsByImplant,
        "SELECT * FROM job WHERE implant=:implant_id LIMIT :limit OFFSET :offset;",
        PARAM(oatpp::String, implant_id),
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  QUERY(getJobsByImplantWhereStatus,
        "SELECT * FROM job WHERE implant=:implant_id AND status=:status LIMIT :limit OFFSET :offset;",
        PARAM(oatpp::String, implant_id),
        PARAM(oatpp::Enum<dto::job_status>::AsNumber, status),
        PARAM(oatpp::UInt32, offset),
        PARAM(oatpp::UInt32, limit));

  /* config */

  QUERY(setConfig,
        "INSERT INTO config "
        "(key, value) VALUES "
        "(:key, :value) "
        "ON CONFLICT(key) DO UPDATE SET value=excluded.value",
        PARAM(oatpp::String, key),
        PARAM(oatpp::String, value));

  QUERY(getConfig,
        "SELECT value "
        "FROM config  "
        "WHERE key=:key;",
        PARAM(oatpp::String, key));

};

#include OATPP_CODEGEN_END(DbClient) //<- End Codegen

}

#endif //MOONSHINE_SERVER_DB_CLIENT_H_
