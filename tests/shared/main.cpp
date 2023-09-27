#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <thread>
#include <chrono>

#include <metadata.h>
#include <message.h>
#include <util/zip.hpp>
#include <util/debug.hpp>
#include "tservice.h"

using namespace std::chrono_literals;

using shared::metadata;
using shared::message;

TEST_CASE("metadata")
{
  auto m1 = std::make_shared<shared::metadata>("abc123", std::vector<unsigned char> { 'X', 'Y', 'Z' });
  auto raw = metadata::serialize(m1);
  REQUIRE(!raw.empty());
  auto m2 = metadata::deserialize(raw);
  REQUIRE(*m1.get() == *m2.get());
}

TEST_CASE("message")
{
  auto t1 = std::make_shared<message>("abc123", "from_me", "to_you", "hello", "command", "ABC", true);
  std::vector<std::shared_ptr<message>> v1;
  v1.push_back(t1);
  auto raw = message::serialize(v1);
  REQUIRE(!raw.empty());
  auto v2 = message::deserialize(raw);
  REQUIRE(v2.size() == 1);
  REQUIRE(*(t1.get()) == *(v2.back().get()));
}

TEST_CASE("worker")
{
  std::string script = R"(
local worker = require "worker"

print("started")

while worker.run() do
  local implant, src, command, data = worker.recv();
  if implant then
      worker.send(implant, src, command, data)
  end
end

print("finished")
)";
  util::zip::zip_file file;
  file.write("init.lua", script);
  std::vector<unsigned char> zip;
  file.save(zip);

  auto ts = tservice(
      "abcd1234",
      "Test Service",
      zip
  );
  auto m1 = std::make_shared<message>("a1b2c3d4", "wxyz9876", ts.id(), "job", "command", "ABC", true);
  auto q1 = std::make_shared<tservice::queue_item>("abcd1234", m1);

  if (!ts.start()) ERROR_PRINT("%s\n", ts.output().c_str());
  REQUIRE(ts.start());
  REQUIRE(ts.is_running());
  REQUIRE(ts.output() == "started\n");

  ts.recv_queue.push(q1);
  std::this_thread::sleep_for(100ms);

  auto q2 = ts.send_queue.wait_and_pop();
  REQUIRE(q1->first == q2->first);
  auto m2 = q2->second;
  REQUIRE(m1->type == m2->type);
  REQUIRE(m1->src == m2->dst);
  REQUIRE(m1->dst == m2->src);
  REQUIRE(m1->command == m2->command);
  REQUIRE(m1->data == m2->data);
  REQUIRE(m1->success == m2->success);

  ts.stop(true);
  std::this_thread::sleep_for(100ms);
  REQUIRE(!ts.is_running());
  REQUIRE(ts.output() == "started\nfinished\n");

  auto raw = tservice::serialize(ts);
  REQUIRE(!raw.empty());
  auto ts2 = tservice::deserialize(raw);
  REQUIRE(ts == ts2);
}
