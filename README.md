g++ -std=c++17 -o translate_file translate_file.cpp Translator.cpp Scaner.cpp SymbolTable.cpp StringTable.cpp Atoms.cpp
./translate_file

g++ -o compiler compiler.cpp Scaner.cpp && ./compiler
