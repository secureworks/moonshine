#ifndef MOONSHINE_SHARED_UTIL_STRING_H_
#define MOONSHINE_SHARED_UTIL_STRING_H_

#include <string>
#include <random>
#include <algorithm>
#include <regex>

namespace util {

namespace string {

inline std::string random_alpha_numeric(int length = 10) {
  std::string str("0123456789abcdefghijklmnopqrstuvwxyz");
  std::random_device rd;
  std::mt19937 generator(rd());
  std::shuffle(str.begin(), str.end(), generator);
  return str.substr(0, length);
}

inline std::string convert(const std::vector<unsigned char> &data) {
  if (data.empty())
    return {};
  return std::string{reinterpret_cast<const char *>(&data[0]), data.size()};;
}

inline std::string convert(const void *data, size_t length) {
  if (data == nullptr)
    return {};
  const auto *raw_memory = reinterpret_cast<const char *>(data);
  return {raw_memory, raw_memory + length};
}

inline std::vector<unsigned char> convert(const std::string &str) {
  if (str.empty())
    return {};
  const auto *raw_memory = reinterpret_cast<const unsigned char *>(str.data());
  return {raw_memory, raw_memory + str.size()};
}

inline std::vector<unsigned char> convert(const char *str, size_t length) {
  if (str == nullptr)
    return {};
  const auto *raw_memory = reinterpret_cast<const unsigned char *>(str);
  return {raw_memory, raw_memory + length};
}

inline bool char_compare_i(char a, char b) {
  return (toupper(a) == toupper(b));
}

inline bool ends_with(std::string_view str, std::string_view ending, bool case_sensitive = true) {
  if (ending.size() > str.size())
    return false;
  if (case_sensitive)
    return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
  return std::equal(ending.rbegin(), ending.rend(), str.rbegin(), char_compare_i);
}

inline bool starts_with(std::string_view str, std::string_view start, bool case_sensitive = true) {
  if (start.size() > str.size())
    return false;
  if (case_sensitive)
    return std::equal(start.begin(), start.end(), str.begin());
  return std::equal(start.begin(), start.end(), str.begin(), char_compare_i);
}

inline bool compare_insensitive(std::string_view str1, std::string_view str2) {
  return ((str1.size() == str2.size()) && std::equal(str1.begin(), str1.end(), str2.begin(), char_compare_i));
}

inline std::string ltrim(const std::string &str) {
  return std::regex_replace(str, std::regex("^\\s+"), std::string(""));
}

inline std::string rtrim(const std::string &str) {
  return std::regex_replace(str, std::regex("\\s+$"), std::string(""));
}

inline std::string trim(const std::string &str) {
  return ltrim(rtrim(str));
}

template<typename ... Args>
inline std::string string_format(const std::string &format, Args ... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
  if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
  auto size = static_cast<size_t>(size_s);
  auto buf = std::make_unique<char[]>(size);
  std::snprintf(buf.get(), size, format.c_str(), args ...);
  return {buf.get(), buf.get() + size - 1}; // We don't want the '\0' inside
}

inline std::string to_hex(const void *buf, size_t size) {
  auto hex = std::make_unique<char[]>(2*size + size/4 + 1);
  char *dst = hex.get();
  for (size_t i = 0; i < size; i++) {
    if (i > 0 && (i % 4) == 0) *dst++ = ' ';
    dst += sprintf(dst, "%02X", ((const uint8_t*)buf)[i]);
  }
  return {hex.get()};
}

inline std::string join(const std::vector<std::string>& v, char c) {

  std::string s;
  for (auto p = v.begin(); p != v.end(); ++p) {
    s += *p;
    if (p != v.end() - 1)
      s += c;
  }
  return s;
}

inline std::vector<std::string> split(const std::string& s, const std::string& delimiter, const bool& removeEmptyEntries = false)
{
  std::vector<std::string> tokens;

  for (size_t start = 0, end; start < s.length(); start = end + delimiter.length()) {
    size_t position = s.find(delimiter, start);
    end = position != std::string::npos ? position : s.length();

    std::string token = s.substr(start, end - start);
    if (!removeEmptyEntries || !token.empty()) {
      tokens.push_back(token);
    }
  }

  if (!removeEmptyEntries &&  (s.empty() || ends_with(s, delimiter))) {
    tokens.emplace_back("");
  }

  return tokens;
}

inline std::string to_lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c){ return std::tolower(c); }
  );
  return s;
}

inline std::string to_upper(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c){ return std::toupper(c); }
  );
  return s;
}

} // string

namespace wstring {

inline bool wchar_compare_i(wchar_t a, wchar_t b) {
  return (towupper(a) == towupper(b));
}

inline bool ends_with(std::wstring_view wstr, std::wstring_view ending, bool case_sensitive = true) {
  if (ending.size() > wstr.size())
    return false;
  if (case_sensitive)
    return std::equal(ending.rbegin(), ending.rend(), wstr.rbegin());
  return std::equal(ending.rbegin(), ending.rend(), wstr.rbegin(), wchar_compare_i);
}

inline bool starts_with(std::wstring_view wstr, std::wstring_view start, bool case_sensitive = true) {
  if (start.size() > wstr.size())
    return false;
  if (case_sensitive)
    return std::equal(start.begin(), start.end(), wstr.begin());
  return std::equal(start.begin(), start.end(), wstr.begin(), wchar_compare_i);
}

inline bool compare_insensitive(std::wstring_view wstr1, std::wstring_view wstr2) {
  return ((wstr1.size() == wstr2.size()) && std::equal(wstr1.begin(), wstr1.end(), wstr2.begin(), wchar_compare_i));
}

inline std::wstring ltrim(const std::wstring &wstr) {
  return std::regex_replace(wstr, std::wregex(L"^\\s+"), std::wstring(L""));
}

inline std::wstring rtrim(const std::wstring &wstr) {
  return std::regex_replace(wstr, std::wregex(L"\\s+$"), std::wstring(L""));
}

inline std::wstring trim(const std::wstring &wstr) {
  return ltrim(rtrim(wstr));
}

} // wstring

namespace base64 {

namespace {

typedef unsigned char uchar;
static const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=

} // anonymous

inline std::string encode(const void *in, std::size_t length) {
  std::string out;

  auto in_uchar = reinterpret_cast<const unsigned char *>(in);

  int val = 0, valb = -6;
  for (std::size_t i = 0; i < length; ++i) {
    val = (val << 8) + *(in_uchar + i);
    valb += 8;
    while (valb >= 0) {
      out.push_back(b[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6) out.push_back(b[((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4) out.push_back('=');
  return out;
}

inline std::string encode(const std::vector<unsigned char> &in) {
  return encode(in.data(), in.size());
}

inline std::string encode(const std::string &in) {
  return encode(in.data(), in.size());
}

inline std::vector<unsigned char> decode(const std::string &in) {

  std::vector<unsigned char> out;

  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++) T[b[i]] = i;

  unsigned val = 0;
  int valb = -8;
  for (uchar c : in) {
    if (T[c] == -1) break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

} // base64

namespace url {

inline unsigned char to_hex(unsigned char x) {
  return x + (x > 9 ? ('A' - 10) : '0');
}

inline unsigned char from_hex(unsigned char ch) {
  if (ch <= '9' && ch >= '0')
    ch -= '0';
  else if (ch <= 'f' && ch >= 'a')
    ch -= 'a' - 10;
  else if (ch <= 'F' && ch >= 'A')
    ch -= 'A' - 10;
  else
    ch = 0;
  return ch;
}

inline std::string encode(const std::string &s) {
  std::string result;
  result.reserve(s.size());

  for (char ch : s)
  {
    if (isalnum(ch) || (ch == '-') || (ch == '.') || (ch == '/') || (ch == '_') || (ch == '~'))
      result.push_back(ch);
    else if (ch == ' ')
      result.push_back('+');
    else
    {
      const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
      result.push_back('%');
      result.push_back(hex[(ch >> 4) & 0x0F]);
      result.push_back(hex[(ch >> 0) & 0x0F]);
    }
  }

  return result;
}

inline std::string decode(const std::string &str) {
  std::string result;
  std::string::size_type i;

  for (i = 0; i < str.size(); ++i) {
    if (str[i] == '+') {
      result += ' ';
    } else if (str[i] == '%' && str.size() > i + 2) {
      const unsigned char ch1 = from_hex(str[i + 1]);
      const unsigned char ch2 = from_hex(str[i + 2]);
      const unsigned char ch = (ch1 << 4) | ch2;
      result += ch;
      i += 2;
    } else {
      result += str[i];
    }
  }
  return result;
}

} // url

} // util

#endif //MOONSHINE_SHARED_UTIL_STRING_H_
