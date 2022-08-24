#include <gtest/gtest.h>
#include "pljit/1_code_management.hpp"
#include "pljit/2_lexer.hpp"
#include "pljit/3_syntax_analysis.hpp"

using namespace syntax_analysis;

TEST(Syntax, simple) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("RETURN 5");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    SyntaxAnalyser syntaxAnalyser(codeManager);
    FunctionDefinition program = syntaxAnalyser.parseFunctionDefinition();

    // all optionals empty
    assert(!program.constants.has_value());
    assert(!program.variables.has_value());
    assert(!program.parameters.has_value());

    // no repeating statements
    CompoundStatement& compoundStatement = program.compoundStatement;
    StatementList& statementList = compoundStatement.statementList;
    assert(statementList.repeating.get() == nullptr);

    // correct optional
    Statement& statement = statementList.statement;
    assert(statement.whichAlternative == Statement::WhichAlternative::ReturnAdditive);
    Statement::ReturnAdditive& returnAdditive = statement.returnAdditive.value();

    // no operators and no additional additive expressions
    AdditiveExpression& additiveExpression = returnAdditive.additiveExpression;
    assert(additiveExpression.whichOperator == AdditiveExpression::WhichOperator::None);
    assert(!additiveExpression.plusOrMinus.has_value());
    assert(additiveExpression.additiveExpression.get() == nullptr);

    // no operators and no additional multiplicative expressions
    MultiplicativeExpression& multiplicativeExpression = additiveExpression.multiplicativeExpression;
    assert(multiplicativeExpression.whichOperator == MultiplicativeExpression::WhichOperator::None);
    assert(!multiplicativeExpression.timesOrDivide.has_value());
    assert(multiplicativeExpression.multiplicativeExpression.get() == nullptr);

    // no operators
    UnaryExpression& unaryExpression = multiplicativeExpression.unaryExpression;
    assert(unaryExpression.whichOperator == UnaryExpression::WhichOperator::NoOperator);
    assert(!unaryExpression.usedOperator.has_value());

    // correct optional as in not the additive expression with brackets
    PrimaryExpression& primaryExpression = unaryExpression.primaryExpression;
    assert(primaryExpression.whichAlternative == PrimaryExpression::WhichAlternative::Literal);
    assert(!primaryExpression.additiveExpressionBrackets.has_value());
    assert(primaryExpression.terminalNode.has_value());

    // correct TerminalNode
    TerminalNode& terminalNode = primaryExpression.terminalNode.value();
    assert(terminalNode.type == TerminalNode::Type::Literal);
    assert(terminalNode.codeFragment.getString() == "5");

}

TEST(Syntax, bigTest) {
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
    SyntaxAnalyser syntaxAnalyser(codeManager);
    FunctionDefinition program = syntaxAnalyser.parseFunctionDefinition();

    // all optionals not empty
    assert(program.constants.has_value());
    assert(program.variables.has_value());
    assert(program.parameters.has_value());

    // correct DeclaratorList of Parameters
    ParameterDeclarations& parameterDeclarations = program.parameters.value();
    DeclaratorList& declaratorListParam = parameterDeclarations.declaratorList;
    assert(declaratorListParam.identifier.codeFragment.getString() == "width");
    assert(declaratorListParam.repeating->identifier.codeFragment.getString() == "height");
    assert(declaratorListParam.repeating->next->identifier.codeFragment.getString() == "depth");
    assert(declaratorListParam.repeating->next->next.get() == nullptr);

    // correct Variable Declarations
    VariableDeclarations& variableDeclarations = program.variables.value();
    assert(variableDeclarations.declaratorList.identifier.codeFragment.getString() == "volume");
    assert(variableDeclarations.declaratorList.repeating.get() == nullptr);

    // correct Constant Declarations
    ConstantDeclarations& constantDeclarations = program.constants.value();
    InitDeclaratorList& initDeclaratorList = constantDeclarations.initDeclaratorList;
    assert(initDeclaratorList.repeating.get() == nullptr);
    assert(initDeclaratorList.initDeclarator.identifier.codeFragment.getString() == "density");
    assert(initDeclaratorList.initDeclarator.literal.codeFragment.getString() == "2400");

    // correct assignment expression
    CompoundStatement& compoundStatement = program.compoundStatement;
    AssignmentExpression& assignmentExpression = compoundStatement.statementList.statement.assignmentExpression.value();
    assert(assignmentExpression.identifier.codeFragment.getString() == "volume");

    // correct multiplicative expression in assignment expression
    MultiplicativeExpression& multiplicativeExpression1 = assignmentExpression.additiveExpression.multiplicativeExpression;
    assert(multiplicativeExpression1.multiplicativeExpression->multiplicativeExpression->multiplicativeExpression.get() == nullptr);
    assert(multiplicativeExpression1.unaryExpression.primaryExpression.whichAlternative == PrimaryExpression::WhichAlternative::Identifier);
    assert(multiplicativeExpression1.unaryExpression.primaryExpression.terminalNode.value().codeFragment.getString() == "width");
    assert(multiplicativeExpression1.whichOperator == MultiplicativeExpression::WhichOperator::Multiplication);
    assert(multiplicativeExpression1.multiplicativeExpression->unaryExpression.primaryExpression.whichAlternative == PrimaryExpression::WhichAlternative::Identifier);
    assert(multiplicativeExpression1.multiplicativeExpression->unaryExpression.primaryExpression.terminalNode.value().codeFragment.getString() == "height");
    assert(multiplicativeExpression1.multiplicativeExpression->whichOperator == MultiplicativeExpression::WhichOperator::Multiplication);
    assert(multiplicativeExpression1.multiplicativeExpression->multiplicativeExpression->unaryExpression.primaryExpression.terminalNode.value().codeFragment.getString() == "depth");

    // correct multiplicative expression in RETURN expression
    MultiplicativeExpression& multiplicativeExpression2 = compoundStatement.statementList.repeating->statement.returnAdditive.value().additiveExpression.multiplicativeExpression;
    assert(multiplicativeExpression2.whichOperator == MultiplicativeExpression::WhichOperator::Multiplication);
    assert(multiplicativeExpression2.unaryExpression.primaryExpression.whichAlternative == PrimaryExpression::WhichAlternative::Identifier);
    assert(multiplicativeExpression2.unaryExpression.primaryExpression.terminalNode.value().codeFragment.getString() == "density");
    assert(multiplicativeExpression2.multiplicativeExpression->unaryExpression.primaryExpression.whichAlternative == PrimaryExpression::WhichAlternative::Identifier);
    assert(multiplicativeExpression2.multiplicativeExpression->unaryExpression.primaryExpression.terminalNode.value().codeFragment.getString() == "volume");
}
