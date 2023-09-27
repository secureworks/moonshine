#ifndef MOONSHINE_SHARED_MESSAGE_H_
#define MOONSHINE_SHARED_MESSAGE_H_

#include <vector>
#include <string>
#include <random>
#include <memory>
#include <algorithm>

#include <util/string.hpp>

namespace shared {

struct message {
  message() : success(true), id(util::string::random_alpha_numeric()) {};
  message(std::string id, std::string type, std::string data) : id(std::move(id)), src(), dst(), type(std::move(type)), command(), data(std::move(data)), success(true) {};
  message(std::string id, std::string type, std::string command, std::string data) : id(std::move(id)), src(), dst(), type(std::move(type)), command(std::move(command)), data(std::move(data)), success(true) {};
  message(std::string id, std::string type, std::string command, std::string data, bool success) : id(std::move(id)), src(), dst(), type(std::move(type)), command(std::move(command)), data(std::move(data)), success(success) {};
  message(std::string id, std::string src, std::string dst, std::string type, std::string command, std::string data) : id(std::move(id)), src(std::move(src)), dst(std::move(dst)), type(std::move(type)), command(std::move(command)), data(std::move(data)), success(true) {};
  message(std::string id, std::string src, std::string dst, std::string type, std::string command, std::string data, bool success) : id(std::move(id)), src(std::move(src)), dst(std::move(dst)), type(std::move(type)), command(std::move(command)), data(std::move(data)), success(success) {};

  std::string id;
  std::string type;
  std::string src;
  std::string dst;
  std::string command;
  std::string data;
  bool success;

  static std::vector<unsigned char> serialize(std::vector<std::shared_ptr<shared::message>>& messages);
  static std::vector<std::shared_ptr<shared::message>> deserialize(std::vector<unsigned char>& raw);

  bool operator ==(const message& o) const;
  [[nodiscard]] std::string to_string() const;
};

} // shared

#endif //MOONSHINE_SHARED_MESSAGE_H_
