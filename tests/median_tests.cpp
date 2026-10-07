#include "median_device.h"

#include <exception>
#include <iostream>

namespace {
bool test_device_lifecycle() {
    MedianDevice device;
    return true;
}

struct TestCase {
    const char* name;
    bool (*run)();
};
} // namespace

int main() {
    // Add test functions above and register them here. Return false on failure.
    const TestCase tests[] = {
        {"device_lifecycle", test_device_lifecycle},
    };

    int total = 0;
    int failures = 0;
    for (const auto& test : tests) {
        ++total;
        std::cout << "[ RUN  ] " << test.name << std::endl;
        bool passed = false;
        try {
            passed = test.run();
        } catch (const std::exception& error) {
            std::cerr << test.name << ": " << error.what() << '\n';
        } catch (...) {
            std::cerr << test.name << ": unknown exception\n";
        }
        std::cout << (passed ? "[ PASS ] " : "[ FAIL ] ") << test.name << '\n';
        if (!passed) ++failures;
    }

    std::cout << total - failures << '/' << total << " tests passed\n";
    return failures == 0 ? 0 : 1;
}
