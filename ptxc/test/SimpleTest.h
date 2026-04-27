#ifndef PTXC_TEST_SIMPLE_TEST_H
#define PTXC_TEST_SIMPLE_TEST_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>

namespace ptxc {
namespace test {

struct TestResult {
    std::string suite;
    std::string name;
    bool passed;
    std::string message;
};

class TestRunner {
public:
    static TestRunner& instance() {
        static TestRunner runner;
        return runner;
    }

    void addTest(const std::string& suite, const std::string& name, std::function<void()> func) {
        tests_.push_back({suite, name, func});
    }

    int runAll() {
        int failed = 0;
        for (const auto& test : tests_) {
            currentSuite_ = test.suite;
            currentName_ = test.name;
            currentPassed_ = true;
            
            try {
                test.func();
            } catch (const std::exception& e) {
                currentPassed_ = false;
                std::cerr << "[ EXCEPTION ] " << test.suite << "." << test.name << ": " << e.what() << std::endl;
            } catch (...) {
                currentPassed_ = false;
                std::cerr << "[ EXCEPTION ] " << test.suite << "." << test.name << ": Unknown exception" << std::endl;
            }

            if (currentPassed_) {
                std::cout << "[       OK ] " << test.suite << "." << test.name << std::endl;
            } else {
                std::cout << "[  FAILED  ] " << test.suite << "." << test.name << std::endl;
                failed++;
            }
        }
        
        std::cout << "\n[==========] " << tests_.size() << " tests ran." << std::endl;
        std::cout << "[  PASSED  ] " << (tests_.size() - failed) << " tests." << std::endl;
        if (failed > 0) {
            std::cout << "[  FAILED  ] " << failed << " tests." << std::endl;
        }
        return failed == 0 ? 0 : 1;
    }

    void fail(const std::string& msg) {
        currentPassed_ = false;
        std::cerr << "  " << currentSuite_ << "." << currentName_ << " failed: " << msg << std::endl;
    }

private:
    struct TestEntry {
        std::string suite;
        std::string name;
        std::function<void()> func;
    };
    std::vector<TestEntry> tests_;
    std::string currentSuite_;
    std::string currentName_;
    bool currentPassed_;
};

struct TestRegistrar {
    TestRegistrar(const std::string& suite, const std::string& name, std::function<void()> func) {
        TestRunner::instance().addTest(suite, name, func);
    }
};

} // namespace test
} // namespace ptxc

#define TEST(suite, name) \
    void suite##_##name##_func(); \
    static ptxc::test::TestRegistrar suite##_##name##_registrar(#suite, #name, suite##_##name##_func); \
    void suite##_##name##_func()

#define EXPECT_EQ(val1, val2) \
    if (!((val1) == (val2))) { \
        std::ostringstream ss; \
        ss << "EXPECT_EQ(" #val1 ", " #val2 ") failed: [" \
           << (val1) << "] != [" << (val2) << "]"; \
        ptxc::test::TestRunner::instance().fail(ss.str()); \
    }

#define EXPECT_TRUE(val) \
    if (!(val)) { \
        ptxc::test::TestRunner::instance().fail("EXPECT_TRUE(" #val ") failed"); \
    }

#define EXPECT_FALSE(val) \
    if (val) { \
        ptxc::test::TestRunner::instance().fail("EXPECT_FALSE(" #val ") failed"); \
    }

#define EXPECT_GE(val1, val2) \
    if (!((val1) >= (val2))) { \
        ptxc::test::TestRunner::instance().fail("EXPECT_GE(" #val1 ", " #val2 ") failed"); \
    }

#define EXPECT_GT(val1, val2) \
    if (!((val1) > (val2))) { \
        ptxc::test::TestRunner::instance().fail("EXPECT_GT(" #val1 ", " #val2 ") failed"); \
    }

#define ASSERT_GE(val1, val2) EXPECT_GE(val1, val2)

#define GTEST_SKIP() return

#endif // PTXC_TEST_SIMPLE_TEST_H
