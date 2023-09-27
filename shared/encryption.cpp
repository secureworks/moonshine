#include "encryption.h"

#include <hydrogen.h>
#include <monocypher.h>

#include "util/debug.hpp"
#include "util/string.hpp"

#define DEBUG_EXTRA 0

#define PACKET_SIZE hydro_kx_N_PACKET1BYTES
#define KEY_SIZE 32
#define NONCE_SIZE 24
#define MAC_SIZE 16

void encryption::init() {
  if (hydro_init() != 0) {
    abort();
  }
}

bool encryption::encryptMetadata(const std::vector<uint8_t> &plaintext,
                                 std::vector<uint8_t> &blob,
                                 const std::vector<unsigned char> &their_public_key) {
  bool success = true;

  hydro_kx_session_keypair session_keypair;
  uint8_t packet[PACKET_SIZE];
  if (hydro_kx_n_1(&session_keypair, packet, nullptr, their_public_key.data()) == 0) {
    auto buffer = new uint8_t[plaintext.size()];

    monocypher::session::key key(session_keypair.tx, 32);
    monocypher::session::nonce nonce;   // random nonce
    monocypher::session::mac mac = key.lock(nonce, plaintext.data(), plaintext.size(), &buffer[0]);

    blob.clear();
    blob.insert(std::end(blob), std::begin(packet), std::end(packet));
    blob.insert(std::end(blob), std::begin(nonce), std::end(nonce));
    blob.insert(std::end(blob), std::begin(mac), std::end(mac));
    blob.insert(std::end(blob), &buffer[0], &buffer[plaintext.size()]);

    delete[] buffer;
  } else {
    ERROR_PRINT("Error creating packet containing ephemeral public key\n");
    success = false;
  }
  return success;
}

bool encryption::encryptMetadata(const std::vector<uint8_t> &plaintext,
                     std::vector<uint8_t> &blob,
                     const monocypher::key_exchange::public_key &their_public_key) {

  //std::vector<unsigned char> public_key(&their_public_key[0], &their_public_key[0] + their_public_key.size());
  //std::vector<unsigned char> public_key;
  //public_key.assign(their_public_key.begin(), their_public_key.end());
  std::vector<unsigned char> public_key(their_public_key.begin(), their_public_key.end());
  return encryption::encryptMetadata(plaintext, blob, public_key);
}

bool encryption::decryptMetadata(const std::vector<uint8_t> &blob,
                                 std::vector<uint8_t> &plaintext,
                                 const monocypher::key_exchange::secret_key &my_secret_key,
                                 const std::vector<unsigned char> &my_public_key) {
  bool success = true;

  hydro_kx_keypair server_static_kp;
  std::copy(my_public_key.begin(), my_public_key.end(), server_static_kp.pk);
  std::copy(my_secret_key.begin(), my_secret_key.end(), server_static_kp.sk);

  uint8_t packet[PACKET_SIZE];
  std::copy(blob.begin(), blob.begin() + PACKET_SIZE, packet);
  hydro_kx_session_keypair session_keypair;

  if (hydro_kx_n_2(&session_keypair, packet, nullptr, &server_static_kp) == 0) {
    if (blob.size() > PACKET_SIZE + NONCE_SIZE + MAC_SIZE) {
      monocypher::session::key key(session_keypair.rx, KEY_SIZE);
      monocypher::session::nonce nonce;
      nonce.fillWith(&blob[PACKET_SIZE], NONCE_SIZE);
      monocypher::session::mac mac;
      mac.fillWith(&blob[PACKET_SIZE + NONCE_SIZE], MAC_SIZE);
      std::vector<uint8_t> ciphertext(blob.begin() + PACKET_SIZE + NONCE_SIZE + MAC_SIZE, blob.end());
      auto buffer = new uint8_t[ciphertext.size()];
      if (key.unlock(nonce, mac, ciphertext.data(), ciphertext.size(), &buffer[0])) {
        plaintext.clear();
        plaintext.insert(plaintext.end(), &buffer[0], &buffer[ciphertext.size()]);
      } else {
        ERROR_PUTS("Error decrypting metadata\n");
        success = false;
      }
      crypto_wipe(buffer, ciphertext.size());
      delete[] buffer;
    } else {
      ERROR_PRINT("Invalid metadata\n");
      success = false;
    }
  } else {
    ERROR_PRINT("Invalid packet\n");
    success = false;
  }

  crypto_wipe(server_static_kp.pk, KEY_SIZE);
  return success;
}

bool encryption::decryptMetadata(const std::vector<uint8_t> &blob,
                     std::vector<uint8_t> &plaintext,
                     const monocypher::key_exchange::secret_key &my_secret_key,
                     const monocypher::key_exchange::public_key &my_public_key) {

  //std::vector<unsigned char> public_key;
  //public_key.assign(my_public_key.begin(), my_public_key.end());
  std::vector<unsigned char> public_key(my_public_key.begin(), my_public_key.end());

  return encryption::decryptMetadata(blob, plaintext, my_secret_key, public_key);
}

bool encryption::decryptData(const std::vector<uint8_t> &blob,
                             std::vector<uint8_t> &plaintext,
                             const monocypher::key_exchange::secret_key &my_secret_key,
                             const monocypher::key_exchange::public_key &their_public_key) {
  bool success = false;

  if (blob.empty())
    return true;

  monocypher::key_exchange key_exchange(my_secret_key);

  monocypher::key_exchange::shared_secret shared_secret = key_exchange.get_shared_secret(their_public_key);
  monocypher::session::key key(shared_secret);
  monocypher::session::nonce nonce;
  nonce.fillWith(&blob[0], 24);
  monocypher::session::mac mac;
  mac.fillWith(&blob[24], 16);
  std::vector<uint8_t> ciphertext(blob.begin() + 40, blob.end());

#if DEBUG_EXTRA
  DEBUG_PUTS("++ decryptData ++\n");
  DEBUG_PRINT("shared secret = %s\n", util::string::to_hex(shared_secret.data(), shared_secret.size()));
  DEBUG_PRINT("        nonce = %s\n", util::string::to_hex(nonce.data(), nonce.size()));
  DEBUG_PRINT("          mac = %s\n", util::string::to_hex(mac.data(), mac.size()));
  DEBUG_PRINT("   ciphertext = %s\n", util::string::to_hex(ciphertext.data(), ciphertext.size()));
#endif

  auto buffer = new uint8_t[ciphertext.size()];
  if (key.unlock(nonce, mac, ciphertext.data(), ciphertext.size(), &buffer[0])) {
    plaintext.clear();
    plaintext.insert(plaintext.end(), &buffer[0], &buffer[ciphertext.size()]);
    success = true;
  }
  crypto_wipe(buffer, ciphertext.size());
  delete[] buffer;
  return success;
}

bool encryption::decryptData(const std::vector<uint8_t> &blob,
                             std::vector<uint8_t> &plaintext,
                             const monocypher::key_exchange::secret_key &my_secret_key,
                             const std::vector<unsigned char> &their_public_key) {
  monocypher::key_exchange::public_key client_pkey;
  client_pkey.fillWith(their_public_key.data(), their_public_key.size());

  return encryption::decryptData(blob, plaintext, my_secret_key, client_pkey);
}

void encryption::encryptData(const std::vector<uint8_t> &plaintext,
                             std::vector<uint8_t> &blob,
                             const monocypher::key_exchange::secret_key &my_secret_key,
                             const monocypher::key_exchange::public_key &their_public_key) {

  if (plaintext.empty())
    return;

  monocypher::key_exchange key_exchange(my_secret_key);

  monocypher::key_exchange::shared_secret shared_secret = key_exchange.get_shared_secret(their_public_key);
  auto ciphertext = new uint8_t[plaintext.size()];

  monocypher::session::key key(shared_secret);
  monocypher::session::nonce nonce;   // random nonce
  monocypher::session::mac mac = key.lock(nonce, plaintext.data(), plaintext.size(), &ciphertext[0]);

  blob.clear();
  blob.insert(std::end(blob), std::begin(nonce), std::end(nonce));
  blob.insert(std::end(blob), std::begin(mac), std::end(mac));
  blob.insert(std::end(blob), &ciphertext[0], &ciphertext[plaintext.size()]);

#if DEBUG_EXTRA
  DEBUG_PUTS("++ encryptData ++\n");
  DEBUG_PRINT("shared secret = %s\n", util::string::to_hex(shared_secret.data(), shared_secret.size()));
  DEBUG_PRINT("        nonce = %s\n", util::string::to_hex(nonce.data(), nonce.size()));
  DEBUG_PRINT("          mac = %s\n", hex_stutil::string::to_hexring(mac.data(), mac.size()));
  DEBUG_PRINT("   ciphertext = %s\n", util::string::to_hex(&ciphertext[0], plaintext.size()));
#endif

  delete[] ciphertext;
}

void encryption::encryptData(const std::vector<uint8_t> &plaintext,
                             std::vector<uint8_t> &blob,
                             const monocypher::key_exchange::secret_key &my_secret_key,
                             const std::vector<unsigned char> &their_public_key) {
  monocypher::key_exchange::public_key client_pkey;
  client_pkey.fillWith(their_public_key.data(), their_public_key.size());

  encryption::encryptData(plaintext, blob, my_secret_key, client_pkey);
}
