#include <pyrite/client.h>
#include<iostream>
using namespace std;



int main() {
  cout<<"client start"<<endl;
  prt::client c("127.0.0.1", 8080);
  c.async();
  std::string input;
  while (true) {
    std::cin >> input;
    std::cout << c.promise("print", prt::bytes(input)).to_string() << std::endl;
  }
  return 0;
}
