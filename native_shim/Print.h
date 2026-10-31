// Host shim for Adafruit GFX: just enough of Arduino's Print/String.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string>

class __FlashStringHelper;

class String {
 public:
  String() {}
  String(const char *s) : s_(s) {}
  const char *c_str() const { return s_.c_str(); }
  size_t length() const { return s_.size(); }
  char charAt(size_t i) const { return s_[i]; }

 private:
  std::string s_;
};

class Print {
 public:
  virtual ~Print() {}
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *buf, size_t n) {
    size_t k = 0;
    while (n--) k += write(*buf++);
    return k;
  }
  size_t write(const char *s) { return write((const uint8_t *)s, strlen(s)); }
  size_t print(const char *s) { return write(s); }
  size_t print(const String &s) { return write(s.c_str()); }
  size_t println(const char *s) { size_t k = write(s); k += write((uint8_t)'\n'); return k; }
};
