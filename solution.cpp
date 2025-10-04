#include <iostream>
#include <string>

using namespace std;

int main() {
    int total = 0;
    int totaltovars = 0;
    int n;
    string temp;

    // Читаем "куплен товар за N"
    cin >> temp >> temp >> temp >> n;

    while (n > 0) {
        string ind;
        cin >> ind;
        
        if (ind == "за") {
            int pricetovar;
            cin >> pricetovar;
            total += pricetovar;
            totaltovars++;
            n--;
        }
        else if (ind == "по") {
            int pricetovar, kolvo;
            cin >> pricetovar >> temp >> kolvo;
            total += pricetovar * kolvo;
            totaltovars += kolvo;
            n--;
        }
    }

    // Обязательные строки в конце
    if (totaltovars == 1) {
        std::cout << "Куплен товар за " << total << " рублей" << std::endl;
    } else {
        std::cout << "Куплено " << totaltovars << " штук товара за " << total << " рублей" << std::endl;
    }
    return 0;
}
