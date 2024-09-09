#include "strings.h"

std::string moc::strrev(std::string _src) {
  char* c = new char[_src.size() + 1];
  int ptr = 0;
  for (int i = _src.size() - 1; i >= 0; i--)
    c[ptr++] = _src[i];
  c[ptr] = 0;
  std::string ret(c);
  delete[] c;
  return ret;
}

std::vector<std::string> 
moc::operator/(const std::string& _src, char _pattern) {
  int l = _src.size(), s = 0;
  std::vector<std::string> ret;
  char *newstr;
  for (int i = 0; i < l; i++) {
    if (_src[i] != _pattern) continue;
    newstr = new char[i-s+1];
    moc::strcpy(newstr, _src.c_str(), s, i-s);
    ret.push_back(newstr);
    delete[] newstr;
    s = i+1;
  }
  newstr = new char[l-s+1];
  moc::strcpy(newstr, _src.c_str(), s, l-s);
  ret.push_back(newstr);
  delete[] newstr;
  return ret;
}

void moc::strcpy(char* dst, const char* src, int ss, int l) {
  for(int i = 0; i < l; i++)
    dst[i] = src[ss+i];
  dst[l] = 0;
}
