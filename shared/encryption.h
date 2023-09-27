#ifndef MOONSHINE_SHARED_ENCRYPTION_H_
#define MOONSHINE_SHARED_ENCRYPTION_H_

#include <vector>
#include <Monocypher.hh>

namespace encryption {

void init();

bool encryptMetadata(const std::vector<uint8_t> &plaintext,
                     std::vector<uint8_t> &blob,
                     const std::vector<unsigned char> &their_public_key);

bool encryptMetadata(const std::vector<uint8_t> &plaintext,
                     std::vector<uint8_t> &blob,
                     const monocypher::key_exchange::public_key &their_public_key);

bool decryptMetadata(const std::vector<uint8_t> &blob,
                     std::vector<uint8_t> &plaintext,
                     const monocypher::key_exchange::secret_key &my_secret_key,
                     const monocypher::key_exchange::public_key &my_public_key);

bool decryptMetadata(const std::vector<uint8_t> &blob,
                     std::vector<uint8_t> &plaintext,
                     const monocypher::key_exchange::secret_key &my_secret_key,
                     const std::vector<unsigned char> &my_public_key);

bool decryptData(const std::vector<uint8_t> &blob,
                 std::vector<uint8_t> &plaintext,
                 const monocypher::key_exchange::secret_key &my_secret_key,
                 const monocypher::key_exchange::public_key &their_public_key);

bool decryptData(const std::vector<uint8_t> &blob,
                 std::vector<uint8_t> &plaintext,
                 const monocypher::key_exchange::secret_key &my_secret_key,
                 const std::vector<unsigned char> &their_public_key);

void encryptData(const std::vector<uint8_t> &plaintext,
                 std::vector<uint8_t> &blob,
                 const monocypher::key_exchange::secret_key &my_secret_key,
                 const monocypher::key_exchange::public_key &their_public_key);

void encryptData(const std::vector<uint8_t> &plaintext,
                 std::vector<uint8_t> &blob,
                 const monocypher::key_exchange::secret_key &my_secret_key,
                 const std::vector<unsigned char> &their_public_key);

std::string hex_string(const void *buf, size_t size);
}

#endif //MOONSHINE_SHARED_ENCRYPTION_H_
