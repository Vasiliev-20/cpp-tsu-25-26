#include <iostream>

using namespace std;

int main() {
    string temp;
    int cost, cnt, itemsCount;
    long long totalCost = 0, totalItems = 0;
    
    cin >> temp >> temp >> temp >> itemsCount;
    
    for (int i = 0; i < itemsCount; i++) {
        while (cin >> temp) {
            if (temp == "за") {
                cin >> cost;
                totalCost += cost;
                totalItems++;
                break;
            }
            if (temp == "по") {
                cin >> cost >> temp >> cnt;
                totalCost += cost * cnt;
                totalItems += cnt;
                break;
            }
        }
    }
    
    if (totalItems == 1) {
        cout << "Куплен товар за " << totalCost << " рублей" << endl;
    } else {
        cout << "Куплено " << totalItems << " штук товара за " << totalCost << " рублей" << endl;
    }
    
    return 0;
}
