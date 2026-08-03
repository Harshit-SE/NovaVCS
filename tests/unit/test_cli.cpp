#include <gtest/gtest.h>
#include "nova/cli/parser.hpp"

TEST(CLITest, DispatchCommandCorrectly) {
    nova::cli::Parser parser;
    bool command_called = false;
    
    parser.registerCommand("test_cmd", [&](const std::vector<std::string>& args) {
        command_called = true;
        EXPECT_EQ(args.size(), 1);
        EXPECT_EQ(args[0], "arg1");
        return 0;
    }, "Test command");

    const char* argv[] = {"nova", "test_cmd", "arg1"};
    int ret = parser.parseAndRun(3, const_cast<char**>(argv));
    
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(command_called);
}
