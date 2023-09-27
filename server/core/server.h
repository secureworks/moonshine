#ifndef MOONSHINE_SERVER_CORE_SERVER_H_
#define MOONSHINE_SERVER_CORE_SERVER_H_

#include "api/server.h"
#include "services/config.h"
#include "handlers/task.h"
#include "handlers/job.h"
#include "handlers/hello.h"

struct server {

  server(const std::string &address,
         unsigned short port,
         const std::string &resource_dir,
         const std::string &implants_dir,
         const std::string &database_path,
         const std::string &cert_pem_path,
         const std::string &cert_crt_path,
         const std::string &cert_dhparams_path);

  void start();
  void stop();

 private:
  api::server _api_server;
  services::config _config_service;
  handlers::task _task_worker;
  handlers::job _jobs_worker;
  handlers::hello _hello_worker;
};

#endif //MOONSHINE_SERVER_CORE_SERVER_H_
