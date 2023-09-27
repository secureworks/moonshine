#include "x509.h"

#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>

#include <spdlog/spdlog.h>

namespace x509 {

namespace {

#if defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

int add_ext(X509 *cert, int nid, char *value)
{
  X509_EXTENSION *ex;
  X509V3_CTX ctx;
  /* This sets the 'context' of the extensions. */
  /* No configuration database */
  X509V3_set_ctx_nodb(&ctx);
  /* Issuer and subject certs: both the target since it is self signed, no request and no CRL */
  X509V3_set_ctx(&ctx, cert, cert, NULL, NULL, 0);
  ex = X509V3_EXT_conf_nid(NULL, &ctx, nid, value);
  if (!ex)
    return 0;

  X509_add_ext(cert,ex,-1);
  X509_EXTENSION_free(ex);
  return 1;
}

/* Generates a 2048-bit RSA key. */
EVP_PKEY *generate_key() {
  /* Allocate memory for the EVP_PKEY structure. */
  EVP_PKEY *pkey = EVP_PKEY_new();
  if (!pkey) {
    throw std::runtime_error("Unable to create EVP_PKEY structure.");
  }

  /* Generate the RSA key and assign it to pkey. */
  BIGNUM *bne = BN_new();
  int ret = BN_set_word(bne, RSA_F4);
  if(ret != 1){
    throw std::runtime_error("Unable to assign RSA_F4 value.");
  }
  RSA *rsa = RSA_new();
  ret = RSA_generate_key_ex(rsa, 2048, bne, NULL);
  if(ret != 1){
    throw std::runtime_error("Unable to generate RSA key pair.");
  }

  if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
    EVP_PKEY_free(pkey);
    throw std::runtime_error("Unable to generate 2048-bit RSA key.");
  }

  /* The key has been generated, return it. */
  return pkey;
}

/* Generates a self-signed x509 certificate. */
X509 *generate_x509(EVP_PKEY *pkey, const std::string& c, const std::string& o, const std::string& cn) {
  /* Allocate memory for the X509 structure. */
  X509 *x509 = X509_new();
  if (!x509) {
    throw std::runtime_error("Unable to create X509 structure.");
  }

  X509_set_version(x509,2);

  /* Set the serial number. */
  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

  /* This certificate is valid from now until exactly one year from now. */
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

  /* Set the public key for our certificate. */
  X509_set_pubkey(x509, pkey);

  /* We want to copy the subject name to the issuer name. */
  X509_name_st *name = X509_get_subject_name(x509);

  /* Set the country code and common name. */
  X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *) c.c_str(), -1, -1, 0); // CountryName
  X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *) o.c_str(), -1, -1, 0); // Organization
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *) cn.c_str(), -1, -1, 0); // CommonName

  /* Now set the issuer name. */
  X509_set_issuer_name(x509, name);

  /* Add various extensions: standard extensions */
  add_ext(x509, NID_basic_constraints, (char*) "critical,CA:TRUE");
  //add_ext(x509, NID_key_usage, "critical,keyCertSign,cRLSign,digitalSignature,keyEncipherment");
  add_ext(x509, NID_subject_key_identifier, (char*) "hash");

  /* Some Netscape specific extensions */
  add_ext(x509, NID_netscape_cert_type, (char*) "sslCA");

  /* This is a typical use for request extensions: requesting a value for
   * subject alternative name. */
  add_ext(x509, NID_subject_alt_name, (char*) "DNS:localhost, IP:127.0.0.1");

  /* Actually sign the certificate with our key. */
  if (!X509_sign(x509, pkey, EVP_sha256())) {
    X509_free(x509);
    throw std::runtime_error("Error signing certificate.");
  }

  return x509;
}

void write_to_disk(const std::string& name, const std::string& path, EVP_PKEY *pkey, X509 *x509) {
  /* Open the PEM file for writing the key to disk. */
  auto key_filepath = std::string(path + PATH_SEPARATOR + name + ".pem");
  FILE *pkey_file = fopen(key_filepath.c_str(), "wb");
  if (!pkey_file) {
    throw std::runtime_error("Unable to open \"" + key_filepath + "\" for writing.");
  }

  /* Write the key to disk. */
  bool ret = PEM_write_PrivateKey(pkey_file, pkey, NULL, NULL, 0, NULL, NULL);
  fclose(pkey_file);

  if (!ret) {
    throw std::runtime_error("Unable to write private key to disk.");
  }

  /* Open the PEM file for writing the certificate to disk. */
  auto crt_filepath = std::string(path + PATH_SEPARATOR + name + ".crt");
  FILE *x509_file = fopen(crt_filepath.c_str(), "wb");
  if (!x509_file) {
    throw std::runtime_error("Unable to open \"" + crt_filepath + "\" for writing.");
  }

  /* Write the certificate to disk. */
  ret = PEM_write_X509(x509_file, x509);
  fclose(x509_file);

  if (!ret) {
    throw std::runtime_error("Unable to write certificate to disk.");
  }
}

}

void generate_self_signed_certificate(const std::string &filename_prefix, const std::string &path, const std::string& country, const std::string& organization, const std::string& common_name) {
  spdlog::info("Generating RSA key...");
  EVP_PKEY *pkey = generate_key();

  spdlog::info("Generating x509 certificate...");
  X509 *x509 = generate_x509(pkey, country, organization, common_name);

  spdlog::info("Writing certificate and key to disk...");
  write_to_disk(filename_prefix, path, pkey, x509);
  EVP_PKEY_free(pkey);
  X509_free(x509);
}

void generate_dhparam(const std::string& file_path) {
  DH * dh = DH_new();
  if (!dh) {
    throw std::runtime_error("Unable to create DH structure.");
  }

  int dh_bits = 2048;
  spdlog::warn("Generating DH parameters ({} bits) - this is going to take a long time!", dh_bits);
  if(!DH_generate_parameters_ex(dh, dh_bits, DH_GENERATOR_2, NULL)) {
    DH_free(dh);
    throw std::runtime_error("Unable to generate DH parameters");
  }

  spdlog::info("Writing DH parameters to disk...");

  FILE *dh_file = fopen(file_path.c_str(), "wb");
  if (!dh_file) {
    DH_free(dh);
    throw std::runtime_error("Unable to open \"" + file_path + "\" for writing.");
  }

  /* Write the certificate to disk. */
  bool ret = PEM_write_DHparams(dh_file, dh);
  fclose(dh_file);

  DH_free(dh);

  if (!ret) {
    throw std::runtime_error("Unable to write DH parameters to disk.");
  }
}

} // x509
