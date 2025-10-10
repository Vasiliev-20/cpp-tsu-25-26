#include <iostream>

using namespace std;

int main() {
    char symbol;
    while (cin >> symbol) {
        if (symbol == ':') break;
    }
    int item_count;
    cin >> item_count;
    
    int overall_quantity = 0;
    int overall_total = 0;
    
    for (int i = 0; i < item_count; i++) {
        string input_word;
        while (cin >> input_word) {
            if (input_word == "за" || input_word == "по") {
                break;
            }
        }
        
        int item_quantity, item_price;
        cin >> item_price;
        
        if (input_word == "за") {
            item_quantity = 1;
            cin >> input_word;
        } else {
            cin >> input_word;
            cin >> item_quantity;
            cin >> input_word;
        }
        
        overall_quantity += item_quantity;
        overall_total += item_quantity * item_price;
    }
    
    if (overall_quantity == 1) {
        cout << "Куплен товар за " << overall_total << " рублей";
    } else {
        cout << "Куплено " << overall_quantity << " штук товара за " << overall_total << " рублей";
    }
    
    return 0;
}
