#include <iostream>

using namespace std;

int main() {

    char c;
    while (cin >> c) {
        if (c == ':') break;
    }
    int items;
    cin >> items;
    
    int total_count = 0;
    int total_amount = 0;
    
    for (int i = 0; i < items; i++) {

        string current_word;
        while (cin >> current_word) {
            if (current_word == "за" || current_word == "по") {
                break;
            }
        }
        
        int quantity, price;
        cin >> price;
        
        if (current_word == "за") {
            quantity = 1;

            cin >> current_word;
        } else {

            cin >> current_word;
            cin >> quantity;

            cin >> current_word;
        }
        
        total_count += quantity;
        total_amount += quantity * price;
    }
    
    if (total_count == 1) {
        cout << "Куплен товар за " << total_amount << " рублей" << endl;
    } else {
        cout << "Куплено " << total_count << " штук товара за " << total_amount << " рублей" << endl;
    }
    
    return 0;
}
