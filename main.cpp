#include <iostream>
#include "delegate.h"

class TestClass {
public:
    void foo(const char *str) {
        std::cout << "TestClass::foo(\"" << str << "\");\n";
    }
};

void foo(const char *str) {
    std::cout << "foo(\"" << str << "\");\n";
}

int main() {
    TestClass obj;

    MulticastDelegate<void, const char *> dele = make_delegate(foo);

    dele += make_delegate(obj, &TestClass::foo);

    dele("salam");

    return 0;
}