#include <iostream>

using namespace std;

int main() {
  int n, quantity, price, summ;
  summ = 0;
  cin >> n;
  while (n > 0){
      cin >>  quantity >> price;
      summ += quantity * price;
      n -= 1;
  }
  cout << summ << endl;
  return 0;
}
