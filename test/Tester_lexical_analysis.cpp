#include <gtest/gtest.h>
#include "pljit/1_code_management.hpp"
#include "pljit/2_lexer.hpp"

using namespace lexical_analysis;
using namespace code_management;

const Keyword* getKeyword(const Token* token) {
    switch(token->type) {
        case Token::Type::Keyword:
            return static_cast<const Keyword*>(token);
        default:
            return nullptr;
    }
}

const Separator* getSeparator(const Token* token) {
    switch(token->type) {
        case Token::Type::Separator:
            return static_cast<const Separator*>(token);
        default:
            return nullptr;
    }
}

const Operator* getOperator(const Token* token) {
    switch(token->type) {
        case Token::Type::Operator:
            return static_cast<const Operator*>(token);
        default:
            return nullptr;
    }
}

const Literal* getLiteral(const Token* token) {
    switch(token->type) {
        case Token::Type::Literal:
            return static_cast<const Literal*>(token);
        default:
            return nullptr;
    }
}

TEST(Lexer, simple) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    Lexer lexer(codeManager);
    std::unique_ptr<Token> token;
    Token* tokens;

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
}

TEST(Lexer, bigTest) {
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
    Lexer lexer(codeManager);
    std::unique_ptr<Token> token;
    Token* tokens;

    // getString von allen Tokens passt

    // first line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);
    ASSERT_TRUE(getKeyword(tokens)->keywords == Keyword::Keywords::PARAM);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "width");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::comma);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "height");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::comma);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "depth");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::semicolon);

    // second line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);
    ASSERT_TRUE(getKeyword(tokens)->keywords == Keyword::Keywords::VAR);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "volume");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::semicolon);

    // third line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);
    ASSERT_TRUE(getKeyword(tokens)->keywords == Keyword::Keywords::CONST);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "density");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Operator);
    ASSERT_TRUE(getOperator(tokens)->operators == Operator::Operators::EqualsInit);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Literal);
    ASSERT_TRUE(getLiteral(tokens)->number == 2400);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::semicolon);

    // fourth line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);
    ASSERT_TRUE(getKeyword(tokens)->keywords == Keyword::Keywords::BEGIN);

    // fifth line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "volume");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Operator);
    ASSERT_TRUE(getOperator(tokens)->operators == Operator::Operators::EqualsAssignment);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "width");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Operator);
    ASSERT_TRUE(getOperator(tokens)->operators == Operator::Operators::Multiplication);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "height");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Operator);
    ASSERT_TRUE(getOperator(tokens)->operators == Operator::Operators::Multiplication);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "depth");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::semicolon);

    // sixth line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == lexical_analysis::Token::Type::Keyword);
    ASSERT_TRUE(getKeyword(tokens)->keywords == Keyword::Keywords::RETURN);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "density");

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Operator);
    ASSERT_TRUE(getOperator(tokens)->operators == Operator::Operators::Multiplication);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Identifier);
    ASSERT_TRUE(tokens->codeFragment.getString() == "volume");

    // seventh line
    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Keyword);
    ASSERT_TRUE(getKeyword(tokens)->keywords == Keyword::Keywords::END);

    token = lexer.next();
    tokens = token.get();
    ASSERT_TRUE(tokens->type == Token::Type::Separator);
    ASSERT_TRUE(getSeparator(tokens)->separators == Separator::Separators::point);
}

TEST(Lexer, stops) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    Lexer lexer(codeManager);
    ASSERT_TRUE(lexer.next()->type == Token::Type::Keyword);
    ASSERT_TRUE(lexer.next()->type == Token::Type::Separator);
    ASSERT_TRUE(lexer.next() == nullptr);
}

TEST(Lexer, deadCodeElimination) {
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("END. BEGIN RETURN 5");
    CodeManager codeManager(sourceCode);
    Lexer lexer(codeManager);
    ASSERT_TRUE(lexer.next()->type == Token::Type::Keyword);
    ASSERT_TRUE(lexer.next()->type == Token::Type::Separator);
    ASSERT_TRUE(lexer.next() == nullptr);
}