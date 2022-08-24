#include <gtest/gtest.h>
#include "pljit/1_code_management.hpp"
#include "pljit/2_lexer.hpp"
#include "pljit/4_semantic_analysis.hpp"

using namespace semantic_analysis;

TEST(Semantics, simple) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("RETURN 5");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    SemanticAnalyser semanticAnalyser(codeManager);
    std::unique_ptr<Function> function = semanticAnalyser.analyseFunction();
    const ReturnStatement* returnStatement = getReturnStatement(function->statement.get());
    assert(returnStatement != nullptr);
    const semantic_analysis::Literal* literal = getLiteral(returnStatement->arithmetic.get());
    assert(literal != nullptr);
    assert(literal->number == 5);
}

TEST(Semantics, bigTest) {
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
    SemanticAnalyser semanticAnalyser(codeManager);
    std::unique_ptr<Function> function = semanticAnalyser.analyseFunction();

    /// Normal Statement has one AssignmentExpression and one ReturnStatement
    const NormalStatement* normalStatement = getNormalStatement(function->statement.get());
    assert(normalStatement != nullptr);

    /// AssignmentExpression
    const AssignmentExpression* assignmentExpression = normalStatement->expression.get();
    assert(assignmentExpression != nullptr);
    assert(assignmentExpression->identifier == "volume");
    const BinaryOperator* binaryOperator1 = getBinaryOperator(assignmentExpression->arithmetic.get());
    assert(binaryOperator1 != nullptr);
    assert(binaryOperator1->type == BinaryOperator::Type::Multiplication);
    const IdentifierNode* identifier2 = getIdentifier(binaryOperator1->left.get());
    assert(identifier2 != nullptr);
    assert(identifier2->name == "width");
    const BinaryOperator* binaryOperator2 = getBinaryOperator(binaryOperator1->right.get());
    assert(binaryOperator2->type == BinaryOperator::Type::Multiplication);
    const IdentifierNode* identifier3 = getIdentifier(binaryOperator2->left.get());
    assert(identifier3->name == "height");
    const IdentifierNode* identifier4 = getIdentifier(binaryOperator2->right.get());
    assert(identifier4->name == "depth");

    /// Return Statement
    const ReturnStatement* returnStatement = getReturnStatement(normalStatement->nextStatement.get());
    assert(returnStatement != nullptr);
    const BinaryOperator* binaryOperator3 = getBinaryOperator(returnStatement->arithmetic.get());
    assert(binaryOperator3->type == BinaryOperator::Type::Multiplication);
    const semantic_analysis::Literal* literal = getLiteral(binaryOperator3->left.get());
    assert(literal != nullptr);
    assert(literal->number == 2400);
    const IdentifierNode* identifier6 = getIdentifier(binaryOperator3->right.get());
    assert(identifier6 != nullptr);
    assert(identifier6->name == "volume");


}