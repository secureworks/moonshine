#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <thread>
#include <chrono>
#include <memory>

#include <client_http.hpp>

#include <util/zip.hpp>
#include <metadata.h>
#include <message.h>

#include <encryption.h>

#include <oatpp/core/provider/Provider.hpp>
#include <oatpp/core/macro/component.hpp>

#include "services/listener.h"
#include "services/job.h"
#include "services/config.h"
#include "server.h"
#include "../../server/executable/x509.h"

#include "scripts.h"

using namespace std::chrono_literals;
using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

void server_start(const std::shared_ptr<server> &s) {
  s->start();
}

struct MyListener : Catch::TestEventListenerBase {
  using TestEventListenerBase::TestEventListenerBase; // inherit constructor

  std::shared_ptr<server> s;
  std::thread t;

  bool is_readable(const std::string &path)
  {
    if (std::filesystem::exists(path)) {
      std::error_code ec; // For noexcept overload usage.
      auto perms = std::filesystem::status(path, ec).permissions();
      if ((perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none &&
          (perms & std::filesystem::perms::group_read) != std::filesystem::perms::none &&
          (perms & std::filesystem::perms::others_read) != std::filesystem::perms::none
          ) {
        return true;
      }
    }
    return false;
  }

  // Get rid of Weak-tables
  ~MyListener() override;

  // The whole test run starting
  void testRunStarting(Catch::TestRunInfo const &testRunInfo) override {
    auto cert_pem_path = "." PATH_SEPARATOR "moonshine.crt";
    auto cert_crt_path = "." PATH_SEPARATOR "moonshine.pem";
    auto cert_dhparams_path = "." PATH_SEPARATOR "dhparams.pem";

    if (!is_readable(cert_pem_path) || !is_readable(cert_crt_path)) {
      x509::generate_self_signed_certificate("moonshine", "." PATH_SEPARATOR, "US", "None", "localhost");
    }

    std::remove("." PATH_SEPARATOR "moonshine.sqlite"); // delete file
    std::remove("." PATH_SEPARATOR "moonshine.sqlite-shm"); // delete file
    std::remove("." PATH_SEPARATOR "moonshine.sqlite-wal"); // delete file

    s = std::make_shared<server>("127.0.0.1",
                                 9000,
                                 ".." PATH_SEPARATOR ".." PATH_SEPARATOR ".." PATH_SEPARATOR ".." PATH_SEPARATOR "server" PATH_SEPARATOR "resources",
                                 BUILD_DIR,
                                 "." PATH_SEPARATOR "moonshine.sqlite",
                                 cert_pem_path,
                                 cert_crt_path,
                                 cert_dhparams_path);

    t = std::thread(server_start, s);
    std::this_thread::sleep_for(100ms);
  }

  // The whole test run starting
  void testRunEnded(Catch::TestRunStats const &testRunStats) override {
    s->stop();
    t.join();
  }
};

CATCH_REGISTER_LISTENER(MyListener)

// Get rid of Weak-tables
MyListener::~MyListener() = default;

TEST_CASE("messages")
{
  OATPP_COMPONENT(std::shared_ptr<db::client>, database); // Inject database component
  REQUIRE(database->truncateTables());

  /* Generate server keys as the truncateTables call wipes them */
  monocypher::key_exchange server_key_exchange;
  auto server_secret_key = server_key_exchange.get_secret_key();
  auto server_secret_key_b64 = util::base64::encode(server_secret_key.data(), server_secret_key.size());
  auto server_public_key = server_key_exchange.get_public_key();
  auto server_public_key_b64 = util::base64::encode(server_public_key.data(), server_public_key.size());

  services::config config;
  config.set("secret_key", server_secret_key_b64);
  config.set("public_key", server_public_key_b64);

  /* Create and start a listener on the server */

  util::zip::zip_file file;
  file.write("init.lua", http_listener);
  auto lib = util::string::convert(util::base64::decode(httpd));
  file.write("httpd.lua", lib);
  std::vector<unsigned char> zip;
  file.save(zip);

  auto dto = dto::listener::createShared();
  dto->name = "http_listener";

  dto->package = util::base64::encode(zip.data(), zip.size());
  auto arg1 = dto::argument::createShared();
  arg1->data = util::base64::encode("127.0.0.1");
  dto->arguments = oatpp::Vector<dto::argument::Wrapper>::createShared();
  dto->arguments->push_back(arg1);
  auto arg2 = dto::argument::createShared();
  arg2->data = util::base64::encode("8123");
  dto->arguments->push_back(arg2);

  auto listener = services::listener::instance()->createWithChildren(dto);
  REQUIRE(!listener->id->empty());
  REQUIRE(listener->name == std::string("http_listener"));
  REQUIRE(!listener->package->empty());
  REQUIRE(listener->arguments->size() == 2);

  auto status = services::listener::instance()->startById(listener->id);
  REQUIRE(status->code == 200);

  std::this_thread::sleep_for(1000ms);

  services::implant implants;
  services::task tasks;
  services::job job;

  /* Generate client long-lived keypair */
  monocypher::key_exchange client_key_exchange;
  auto public_key = client_key_exchange.get_public_key();

  /* Configure fake client */

  HttpClient client("127.0.0.1:8123");

  // Generate metadata header
  auto m = std::make_shared<shared::metadata>("implant_12345", std::vector<unsigned char>(public_key.begin(), public_key.end()));
  std::vector<uint8_t> metadata = shared::metadata::serialize(m);
  std::vector<uint8_t> blob = {};
  REQUIRE(encryption::encryptMetadata(metadata, blob, config.get_server_public_key()));

  // We are using a http listener, so it expects the metadata blob to be in a cookie
  SimpleWeb::CaseInsensitiveMultimap header;
  header.emplace("Cookie", "implant=" + util::url::encode(util::base64::encode(&blob[0], blob.size())));

  /* Start sending tasks/responses */

  SECTION("send none, receive none")
  {
    REQUIRE(implants.getAll((v_uint32)0, 10)->count == 0);

    REQUIRE(services::listener::instance()->is_running(listener->id));
    auto response = client.request("GET", "/", "", header);
    REQUIRE(response->content.string().empty());
    REQUIRE(response->status_code == "204 No Content");
    REQUIRE(implants.exists(m->id));

    auto dataset = tasks.getAllByImplant(oatpp::String(m->id), (v_uint32) 0, 1);
    REQUIRE(dataset->count == 0);

    REQUIRE(implants.getAll((v_uint32)0, 10)->count == 1);
    REQUIRE(implants.getById(oatpp::String(m->id))->id == oatpp::String(m->id));
    REQUIRE(implants.getById(oatpp::String(m->id))->listener == listener->id);
  }

  SECTION("send one task message, receive one task message")
  {
    auto task_dto1 = dto::task::createShared();
    task_dto1->implant = m->id;
    task_dto1->script = std::string("blah");

    auto task1 = tasks.create(task_dto1);
    REQUIRE(!task1->id->empty());
    REQUIRE(task1->implant == m->id);
    REQUIRE(task1->script == std::string("blah"));
    REQUIRE(task1->status == dto::task_status::SUBMITTED);
    REQUIRE(task1->success == dto::task_success::UNKNOWN);

    auto task1b = tasks.getById(task1->id);
    REQUIRE(task1->id == task1b->id);
    REQUIRE(task1->implant == task1b->implant);
    REQUIRE(task1->script == task1b->script);
    REQUIRE(task1->status == task1b->status);
    REQUIRE(task1->success == task1b->success);

    REQUIRE(tasks.getAllByImplant(oatpp::String(m->id), (v_uint32)0, 1)->count == 1);

    REQUIRE(services::listener::instance()->is_running(listener->id));
    auto response = client.request("GET", "/", "", header);
    REQUIRE(response->status_code == "200 OK");
    std::vector<uint8_t> blob = util::base64::decode(response->content.string());
    std::vector<uint8_t> data = {};
    encryption::decryptData(blob, data, client_key_exchange.get_secret_key(), config.get_server_public_key());
    auto x = shared::message::deserialize(data);
    REQUIRE(x.size() == 1);
    REQUIRE(x.back()->id == task1->id->c_str());
    REQUIRE(x.back()->type == "task");

    auto task1c = tasks.getById(task1->id);
    REQUIRE(task1c->status == dto::task_status::RETRIEVED);

    std::vector<std::shared_ptr<shared::message>> a;
    a.push_back(std::make_shared<shared::message>(task1->id->c_str(), "task", "", "YAY", true));
    std::vector<uint8_t> data2 = shared::message::serialize(a);
    std::vector<uint8_t> blob2 = {};
    encryption::encryptData(data2, blob2, client_key_exchange.get_secret_key(), config.get_server_public_key());

    REQUIRE(services::listener::instance()->is_running(listener->id));
    response = client.request("POST", "/", util::base64::encode(blob2), header);
    REQUIRE(response->status_code == "204 No Content");

    std::this_thread::sleep_for(100ms); // pause to provide the server a moment to process the result

    oatpp::String id = a[0]->id;
    auto task0 = tasks.getById(const_cast<oatpp::String &>(id));
    REQUIRE(task0->id == a[0]->id);
    REQUIRE(task0->implant == m->id);
    REQUIRE(task0->output == util::base64::encode("YAY"));
    REQUIRE(task0->success == dto::task_success::SUCCESSFUL);
    REQUIRE(task0->status == dto::task_status::COMPLETED);

    REQUIRE(tasks.getAllByImplant(oatpp::String(m->id), (v_uint32)0, 1)->count == 1);
  }

  SECTION("send one jobs message, receive one jobs message")
  {
    auto job_dto1 = dto::job::createShared();
    job_dto1->implant = m->id;
    job_dto1->name = std::string("name");
    job_dto1->package = std::string("package");

    auto job1 = job.createWithChildren(job_dto1);
    REQUIRE(!job1->id->empty());
    REQUIRE(job1->implant == m->id);
    REQUIRE(job1->name == std::string("name"));
    REQUIRE(job1->package ==std::string("package"));
    REQUIRE(job1->status == dto::job_status::SUBMITTED);
    REQUIRE(job1->success == dto::job_success::UNKNOWN);

    auto job1b = job.getByIdWithChildren(job1->id);
    REQUIRE(job1->id == job1b->id);
    REQUIRE(job1->implant == job1b->implant);
    REQUIRE(job1->name == job1b->name);
    REQUIRE(job1->package == job1b->package);
    REQUIRE(job1->status == job1b->status);
    REQUIRE(job1->success == job1b->success);

    REQUIRE(job.getAllWithChildrenByImplant(oatpp::String(m->id), (v_uint32)0, 10)->count == 1);

    REQUIRE(services::listener::instance()->is_running(listener->id));
    auto response = client.request("GET", "/", "", header);
    REQUIRE(response->status_code == "200 OK");
    std::vector<uint8_t> blob = util::base64::decode(response->content.string());
    std::vector<uint8_t> data = {};
    encryption::decryptData(blob, data, client_key_exchange.get_secret_key(), config.get_server_public_key());
    auto x = shared::message::deserialize(data);
    REQUIRE(x.size() == 1);
    REQUIRE(x.back()->id == job1->id->c_str());
    REQUIRE(x.back()->type == "job_handler");

    auto job1c = job.getByIdWithChildren(job1->id);
    REQUIRE(job1c->status == dto::job_status::RETRIEVED);

    std::vector<std::shared_ptr<shared::message>> a;
    a.push_back(std::make_shared<shared::message>(job1->id->c_str(), job1->id->c_str(), "", "job_handler", "completed", "YAY", true));
    std::vector<uint8_t> data2 = shared::message::serialize(a);
    std::vector<uint8_t> blob2 = {};
    encryption::encryptData(data2, blob2, client_key_exchange.get_secret_key(), config.get_server_public_key());

    REQUIRE(services::listener::instance()->is_running(listener->id));
    response = client.request("POST", "/", util::base64::encode(blob2), header);
    REQUIRE(response->status_code == "204 No Content");

    std::this_thread::sleep_for(100ms); // pause to provide the server a moment to process the result

    oatpp::String id = a[0]->id;
    auto job0 = job.getByIdWithChildren(const_cast<oatpp::String &>(id));
    REQUIRE(job0->id == a[0]->id);
    REQUIRE(job0->implant == m->id);
    REQUIRE(job0->output->c_str() == util::base64::encode("YAY"));
    REQUIRE(job0->success == dto::job_success::SUCCESSFUL);
    REQUIRE(job0->status == dto::job_status::COMPLETED);

    REQUIRE(job.getAllWithChildrenByImplant(oatpp::String(m->id), (v_uint32)0, 1)->count == 1);
  }

  status = services::listener::instance()->stopById(listener->id, true);
  REQUIRE(status->code == 200);
  status = services::listener::instance()->deleteByIdWithChildren(listener->id);
  REQUIRE(status->code == 200);
  std::this_thread::sleep_for(100ms);
}
