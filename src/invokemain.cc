#include <iostream>
using namespace std;
extern "C" void invokemain();

int main() {
  cout << "Inside invokemain.cc\n";
  invokemain();
}
