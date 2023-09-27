#include "message.h"

#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#include <util/string.hpp>

namespace yas {

template<typename Archive>
void serialize(Archive &ar, shared::message &o) {
  ar & o.id & o.type & o.src & o.dst & o.command & o.data & o.success;
}

}

std::vector<unsigned char> shared::message::serialize(std::vector<std::shared_ptr<shared::message>>& tasks) {
  try {
    std::vector<shared::message> v;
    v.reserve(tasks.size());
    for (const auto &t : tasks) {
      v.push_back(*t);
    }
    auto buf = yas::save<yas::mem | yas::binary | yas::no_header>(
        YAS_OBJECT_NVP("messages", ("v", v))
    );
    return util::string::convert(buf.data.get(), buf.size);
  }
  catch (const std::exception &ex) {
    throw std::invalid_argument(ex.what());
  }
}

std::vector<std::shared_ptr<shared::message>> shared::message::deserialize(std::vector<unsigned char>& raw)
{
  try {
    std::vector<shared::message> v;
    yas::shared_buffer buf{raw.data(), raw.size()};
    yas::load<yas::mem | yas::binary | yas::no_header>(
        buf, YAS_OBJECT_NVP("messages", ("v", v))
    );

    std::vector<std::shared_ptr<shared::message>> rtn;
    for (auto & message : v) {
      auto shared_message = std::make_shared<shared::message>(std::move(message));
      rtn.push_back(shared_message);
    }
    return rtn;
  }
  catch (const std::exception &ex) {
    throw std::invalid_argument(ex.what());
  }
}

bool shared::message::operator ==(const message& o) const
{
  return this->id == o.id &&
    this->type == o.type &&
    this->src == o.src &&
    this->dst == o.dst &&
    this->command == o.command &&
    this->data == o.data &&
    this->success == o.success;
}

[[nodiscard]] std::string shared::message::to_string() const {
  return std::string("[[ MESSAGE ]]") +
    "\n         ID: " + id +
    "\n       Type: " + type +
    "\n     Source: " + src +
    "\nDestination: " + dst +
    "\n    Command: " + command +
    "\n    Success: " + std::to_string(success) +
    "\n       Data: " + data;
}
