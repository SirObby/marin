#include <iostream>

extern void test_i18n();

int main() {
    std::cout << "Running Tests...\n";
    test_i18n();
    std::cout << "All tests passed.\n";
    return 0;
}
