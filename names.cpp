
int main() {}

long long int foo() { return 42; }
long long int foo(long long int) { return 100; }
long long int foo(double, double) { return 100500; }

void bar() {}

extern "C" {
void bar(long long int) {}

// ошибка компиляции
// void bar() {}
}
