#include <pyrite/server.h>

prt::bytes default_handler(sockaddr_in client_addr, prt::bytes data) {
  return prt::bytes(std::string("server recv: ") + data.to_string());
}

prt::bytes version(sockaddr_in client_addr, prt::bytes data) {
  return prt::bytes(std::string("2.0.0"));
}

int main() {
  prt::server s(8080);
  s.set_handler("*", default_handler);
  s.set_handler("version", version);
  s.start();
  return 0;
}
