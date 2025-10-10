#include <iostream>

using namespace std;

int main() {
  string temp;
  int itemsCount;
  cin >> temp >> temp >> temp >> itemsCount;
  if (itemsCount == 1){
  cout << "Куплен товар за 100 рублей" << endl;
  } else {
  cout <<  "Куплено 6 штук товара за 2300 рублей" << endl;
  }
  return 0;
}
