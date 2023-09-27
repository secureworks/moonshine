#include "metadata.h"

#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#include <util/string.hpp>

std::vector<unsigned char> shared::metadata::serialize(std::shared_ptr<shared::metadata> object) {
  try {
    auto buf = yas::save<yas::mem | yas::binary | yas::no_header>(
      YAS_OBJECT_NVP("metadata", ("id", object->id), ("public_key", object->public_key))
    );
    return util::string::convert(buf.data.get(), buf.size);
  }
  catch (const std::exception &ex) {
    throw std::invalid_argument(ex.what());
  }
}

std::shared_ptr<shared::metadata> shared::metadata::deserialize(std::vector<unsigned char> &raw) {
  try {
    auto object = std::make_shared<shared::metadata>();
    yas::shared_buffer buf{raw.data(), raw.size()};
    yas::load<yas::mem | yas::binary | yas::no_header>(
        buf, YAS_OBJECT_NVP("metadata", ("id", object->id), ("public_key", object->public_key))
    );
    return object;
  }
  catch (const std::exception &ex) {
    throw std::invalid_argument(ex.what());
  }
}

std::string shared::metadata::to_string() const {
  return std::string(">>> METADATA >>>") +
      "\n        ID: " + id +
      "\nPublic Key: " + util::string::to_hex(&public_key[0], public_key.size());
}
