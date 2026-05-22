#include <iostream>
#include <fstream>
#include "Translator.h"

int main() {
    std::ifstream input("my.minic");

    if (!input.is_open()) {
        std::cerr << "Error: cannot open file my.minic\n";
        return 1;
    }

    try {
        Translator translator(input);
        if (!translator.translate()) return 1;
        std::cout << "Syntax OK" << std::endl;
        translator.printAtoms(std::cout);
        /*translator.getSymbolTable().print(std::cout);
        std::cout << std::endl;
        std::cout << translator.getStringTable();
        std::cout << std::endl;*/
        translator.generateCode(std::cout);
    }
    catch (const TranslatorException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

//int fact(int a) {
//    int i, res = 1;
//    for (i = 1; i <= a; ++i) {
//        res = res * i;
//    }
//    return res;
//}
//
//
//int main() {
//    int a = 0, i, res;
//    in a;
//    for (i = 0; i < a; ++i) {
//        if (i == 5) {
//            out "5 is there";
//        }
//        else {
//            switch (i) {
//            case 1:
//                out "1 is here";
//            case 2:
//                out "2 is here";
//            default:
//                out "nothing";
//            }
//        }
//    }
//    res = fact(5);
//    out res;
//}
