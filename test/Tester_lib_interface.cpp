#include <gtest/gtest.h>
#include "pljit/1_code_management.hpp"
#include "pljit/6_lib_interface.hpp"

using namespace interface;

TEST(Interface, simple) {
    std::string code = "BEGIN\nRETURN 5\nEND.";
    Pljit jit;
    Function function = jit.registerFunctionAlternative(code);
    int64_t result = function();
    assert(result == 5);
}

TEST(Interface, bigTest1) {
    std::stringstream ss;
    std::vector<std::string> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;\n");
    sourceCode.emplace_back("VAR volume;\n");
    sourceCode.emplace_back("CONST density = 2400;\n");
    sourceCode.emplace_back("BEGIN\n");
    sourceCode.emplace_back("\tvolume := width * height * depth;\n");
    sourceCode.emplace_back("\tRETURN density * volume\n");
    sourceCode.emplace_back("END.");
    for(std::string line : sourceCode) {
        ss << line;
    }
    std::string s = ss.str();
    Pljit jit;
    Function function = jit.registerFunctionAlternative(s);
    int64_t result = function({1,2,3});
    assert(result == (1*2*3*2400));
}

TEST(Interface, bigTest2) {
    std::stringstream ss;
    std::vector<std::string> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;\n");
    sourceCode.emplace_back("VAR volume;\n");
    sourceCode.emplace_back("CONST density = 3;\n");
    sourceCode.emplace_back("BEGIN\n");
    sourceCode.emplace_back("\tvolume := width * (height / depth);\n");
    sourceCode.emplace_back("\tRETURN density * volume\n");
    sourceCode.emplace_back("END.");
    // combine all strings to one string
    for(std::string line : sourceCode) {
        ss << line;
    }
    std::string s = ss.str();
    Pljit jit;
    Function function = jit.registerFunctionAlternative(s);
    int64_t result = function({-5,2,3});
    assert(result == (-5 * (2 / 3) * 3));
}