#include "byte.h"
#include "iostream"

moc::bytes::bytes() : std::vector<byte>(), ptr(0) {}

moc::bytes::bytes(int _len) : std::vector<byte>(_len), ptr(0) {}

moc::bytes::bytes(std::string _src) : std::vector<byte>(_src.size()+1), ptr(0) {
  for (int i = 0; i < _src.size(); i++)
    this->operator[](i) = _src[i];
  this->operator[](_src.size()) = 0;
}

moc::bytes::bytes(const void* _addr, int _len) : std::vector<byte>(_len), ptr(0) {
  const byte* addr = (const byte*) _addr;
  for (int i = 0; i < _len; i++)
    this->operator[](i) = addr[i];
}

moc::bytes moc::bytes::range(int start, int end) {
  if (end <= start) return bytes();
  if (end > this->size()) return bytes();
  int len = end-start;
  bytes ret(len);
  for (int i = 0; i < len; i++)
    ret[i] = this->operator[](start+i);
  return ret;
}

std::string moc::bytes::to_string() {
  int end = this->size();
  for (int i = 0; i < this->size(); i++) {
    if (!this->operator[](i)) {
      end = i;
      break;
    }
  }
  char str[end+1];
  for (int i = 0; i < end; i++)
    str[i] = this->operator[](i);
  str[end] = 0;
  return std::string(str);
}

moc::bytes moc::bytes::operator+(const bytes& v) {
  bytes ret(this->size()+v.size());
  int ptr = 0;
  for (int i = 0; i < this->size(); i++)
    ret[ptr++] = this->operator[](i);
  for (int i = 0; i < v.size(); i++)
    ret[ptr++] = v[i];
  return ret;
}

#define min(a, b) ((a) < (b) ? (a) : (b))
void moc::bytes::operator+=(const bytes& other) {
  this->ptr = min(this->ptr, this->size());
  for (int i = 0; i < other.size(); i++)
    this->push_back(other[i]);
}

void moc::bytes::operator+=(byte other) {
  this->ptr = min(this->ptr, this->size());
  this->push_back(other);
}

void moc::bytes::operator+=(std::string& other) {
  this->operator+=(moc::bytes(other));
}

void moc::bytes::operator+=(long other) {
  for (int i = 0; i < 4; ++i)
    this->operator+=((moc::byte) (other >> (i*8)));
}

void moc::bytes::operator+=(std::map<std::string, moc::bytes> &m) {
  this->operator+=((long) m.size());
  for (const auto &it: m) {
    this->operator+=(it.first);
    this->operator+=((long) it.second.size());
    this->operator+=(it.second);
  }
}

std::string moc::bytes::to_hex_str() {
  std::string ret;
  ret.resize(2*(this->size()));
  const char map[17] = "0123456789ABCDEF";
  int ptr = 0;
  for (int i = 0; i < this->size(); i++) {
    unsigned char val = (unsigned char) this->operator[](i);
    ret[ptr++] = map[val>>4];
    ret[ptr++] = map[val&0x0F];
  }
  return ret;
}

void moc::bytes::print_as_hex() {
  std::cout << this->to_hex_str();
}

void moc::bytes::println_as_hex() {
  this->print_as_hex();
  std::cout << std::endl;
}

bool moc::bytes::has_next() {
  return this->ptr < this->size();
}

std::string moc::bytes::next_string() {
  if (!this->has_next()) return "";
  int start = this->ptr;
  while (this->ptr < this->size() && this->operator[](this->ptr))
    this->ptr++;
  return this->range(start, this->ptr++).to_string();
}

long moc::bytes::next_int32() {
  if (!this->has_next()) return 0;
  if (this->ptr + 3 >= this->size()) return 0;
  unsigned long ret = 0;
  for (int i = 0; i < 4; i++) {
    ret >>= 8;
    ret |= ((unsigned long) this->operator[](this->ptr++)) << 24;
  }
  return (long) ret;
}

std::map<std::string, moc::bytes> moc::bytes::next_map() {
  int size = this->next_int32();
  std::map<std::string, moc::bytes> ret;
  while (size--) {
    std::string key = this->next_string();
    int len = this->next_int32();
    ret[key] = this->range(this->ptr, this->ptr + len);
    this->ptr += len;
  }
  return ret;
}

void moc::bytes::to_mem(void* _addr, int _len) {
  if (_len > this->size())
    return;
  byte* addr = (byte*) _addr;
  for (int i = 0; i < _len; i++)
    addr[i] = this->operator[](i);
}
