#include <gtest/gtest.h>
#include "pljit/5_execution.hpp"

using namespace execution;

TEST(Execution, simple) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("RETURN 5");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    Evaluation evaluation(codeManager);
    int64_t result = evaluation.evaluateFunction({});
    assert(result == 5);
}

TEST(Execution, bigTest) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;");
    sourceCode.emplace_back("VAR volume;");
    sourceCode.emplace_back("CONST density = 2400;");
    //sourceCode.emplace_back("");
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("\tvolume := width * height * depth;");
    sourceCode.emplace_back("\tRETURN density * volume");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    Evaluation evaluation(codeManager);
    int64_t result = evaluation.evaluateFunction({1,2,3});
    assert(result == (1*2*3*2400));
}


TEST(Execution, constantPropagation1) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("\tRETURN 5 * 10 * 6");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    Evaluation evaluation(codeManager);
    int64_t result = evaluation.evaluateFunction({});
    assert(result == 300);
    auto& function = evaluation.function;
    const semantic_analysis::ReturnStatement* returnStatement = semantic_analysis::getReturnStatement(function->statement.get());
    assert(returnStatement != nullptr);
    const semantic_analysis::Literal* literal = semantic_analysis::getLiteral(returnStatement->arithmetic.get());
    assert(literal != nullptr);
    assert(literal->number == 300);
}

TEST(Execution, constantPropagation2) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("CONST var = 5;");
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("\tRETURN 1 * 2 * var");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    Evaluation evaluation(codeManager);
    int64_t result = evaluation.evaluateFunction({});
    assert(result == 10);
    auto& function = evaluation.function;
    const semantic_analysis::ReturnStatement* returnStatement = semantic_analysis::getReturnStatement(function->statement.get());
    assert(returnStatement != nullptr);
    const semantic_analysis::Literal* literal = semantic_analysis::getLiteral(returnStatement->arithmetic.get());
    assert(literal != nullptr);
    assert(literal->number == 10);
}