#include "2_lexer.hpp"
#include <string_view>
#include <charconv>
#include <iostream>
#include <optional>
#include <cctype>
//--------------------------------------------------------------
namespace lexical_analysis {

/// constructor for Literal
Literal::Literal(CodeFragment& codeFragment)
    : Token(codeFragment, Type::Literal) {
    std::string_view stringNumber = codeFragment.getString();
    auto result = std::from_chars(stringNumber.data(), stringNumber.data() + stringNumber.size(), number);
    if(result.ec == std::errc::invalid_argument) {
        std::cout << "is not a number" << std::endl;
    }
}

/// advances position by one char
/// returns true if it goes to a new line
bool Lexer::advanceOneChar() {
    ++charNum;
    if(charNum >= codeManager.sourceCode[lineNum].size()) {
        charNum = 0;
        ++lineNum;
        if(lineNum >= codeManager.sourceCode.size()) {
            lexedAll = true;
            return true;
        }
        return true;
    } else {
        return false;
    }
}

/// advance postion until first non-whitespace character
void Lexer::firstAdvance() {
    while(!lexedAll) {
        std::string_view currentChar = codeManager.sourceCode[lineNum].substr(charNum, 1);
        if(currentChar != " " && currentChar != "\n" && currentChar != "\t") {
            return;
        }
        advanceOneChar();
    }
}

/// advances Lexer-Algo and determines next token
std::unique_ptr<Token> Lexer::next() {
    firstAdvance();
    if(lexedAll) {
        return nullptr;
    }

    // based on first non-whitespace character, determine type of token

    // immediately parsing single character tokens

    std::string_view currentChar = codeManager.sourceCode[lineNum].substr(charNum, 1);
    CodeFragment codeFragment = codeManager.createCodeFragment(lineNum, charNum);
    if("," == currentChar) {
        advanceOneChar();
        return std::make_unique<Separator>(codeFragment, Separator::Separators::comma);
    } else if(";" == currentChar) {
        advanceOneChar();
        return std::make_unique<Separator>(codeFragment, Separator::Separators::semicolon);
    } else if("+" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::Plus);
    } else if("-" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::Minus);
    } else if("*" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::Multiplication);
    } else if("/" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::Division);
    } else if("=" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::EqualsInit);
    } else if("(" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::BracketsOpen);
    } else if(")" == currentChar) {
        advanceOneChar();
        return std::make_unique<Operator>(codeFragment, Operator::Operators::BracketsClosed);
    } else if("." == currentChar) {
        lexedAll = true;
        return std::make_unique<Separator>(codeFragment, Separator::Separators::point);

        // parsing token with two chars
    } else if(":" == currentChar) {
        advanceOneChar();
        currentChar = codeManager.sourceCode[lineNum].substr(charNum, 1);
        codeFragment = codeManager.createCodeFragment(lineNum, charNum - 1, charNum);
        if("=" == currentChar) {
            advanceOneChar();
            return std::make_unique<Operator>(codeFragment, Operator::Operators::EqualsAssignment);
        } else {
            codeFragment.print("error: should be \":=\"");
            advanceOneChar();
            return nullptr;
        }
    }

    // if literal
    if(isdigit(codeManager.sourceCode[lineNum][charNum])) {
        size_t beginningChar = charNum;
        bool wentOverLine = false;
        //advanceOneChar()
        while(isdigit(codeManager.sourceCode[lineNum][charNum])) {
            if(advanceOneChar()) {
                wentOverLine = true;
                break;
            }
        }
        if(!wentOverLine) {
            codeFragment = codeManager.createCodeFragment(lineNum, beginningChar, charNum - 1);
        } else {
            codeFragment = codeManager.createCodeFragment(lineNum - 1, beginningChar, codeManager.sourceCode[lineNum-1].size() - 1);
        }
        return std::make_unique<Literal>(codeFragment);
    }

    // if keyword or identifier
    if(isalpha(codeManager.sourceCode[lineNum][charNum])) {
        size_t beginningChar = charNum;
        bool wentOverLine = false;
        //advanceOneChar()
        while(isalpha(codeManager.sourceCode[lineNum][charNum])) {
            if(advanceOneChar()) {
                wentOverLine = true;
                break;
            }
        }
        if(!wentOverLine) {
            codeFragment = codeManager.createCodeFragment(lineNum, beginningChar, charNum - 1);
        } else {
            codeFragment = codeManager.createCodeFragment(lineNum - 1, beginningChar, codeManager.sourceCode[lineNum-1].size() - 1);
        }
        // parsing keywords
        std::string_view stringOperator = codeFragment.getString();
        if("PARAM" == stringOperator) {
            return std::make_unique<Keyword>(codeFragment, Keyword::Keywords::PARAM);
        } else if("VAR" == stringOperator) {
            return std::make_unique<Keyword>(codeFragment, Keyword::Keywords::VAR);
        } else if("CONST" == stringOperator) {
            return std::make_unique<Keyword>(codeFragment, Keyword::Keywords::CONST);
        } else if("BEGIN" == stringOperator) {
            return std::make_unique<Keyword>(codeFragment, Keyword::Keywords::BEGIN);
        } else if("END" == stringOperator) {
            return std::make_unique<Keyword>(codeFragment, Keyword::Keywords::END);
        } else if("RETURN" == stringOperator) {
            return std::make_unique<Keyword>(codeFragment, Keyword::Keywords::RETURN);
        } else {
            return std::make_unique<Identifier>(codeFragment);
        }
    }
    codeFragment.print("error: unknown character");
    return nullptr;
}


} // namespace lexical_analysis
//--------------------------------------------------------------