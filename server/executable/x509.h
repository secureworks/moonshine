#ifndef MOONSHINE_SERVER_EXECUTABLE_X509_H_
#define MOONSHINE_SERVER_EXECUTABLE_X509_H_

#include <string>

namespace x509 {

void generate_self_signed_certificate(const std::string &filename_prefix, const std::string &path, const std::string& country, const std::string& organization, const std::string& common_name);
void generate_dhparam(const std::string& file_path);

} // x509

#endif //MOONSHINE_SERVER_EXECUTABLE_X509_H_
