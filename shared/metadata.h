#ifndef MOONSHINE_SHARED_METADATA_H_
#define MOONSHINE_SHARED_METADATA_H_

#include <vector>
#include <string>
#include <memory>

namespace shared {

struct metadata {
  metadata() = default;
  metadata(std::string id, std::vector<unsigned char> public_key)
      : id(std::move(id)), public_key(std::move(public_key)) {};

  std::string id;
  std::vector<unsigned char> public_key;

  static std::vector<unsigned char> serialize(std::shared_ptr<shared::metadata> object);
  static std::shared_ptr<shared::metadata> deserialize(std::vector<unsigned char> &raw);

  bool operator ==(const metadata& o) const
  {
    return this->id == o.id &&
        this->public_key == o.public_key;
  };

  [[nodiscard]] std::string to_string() const;
};

} // shared

#endif //MOONSHINE_SHARED_METADATA_H_
