#ifndef H_2_lexical_analysis
#define H_2_lexical_analysis
#include <utility>
#include <vector>
#include <string_view>
#include <optional>
#include <memory>
#include "1_code_management.hpp"
//--------------------------------------------------------------
using namespace code_management;

namespace lexical_analysis {

/*
 * most important class is Lexer, which provides methods for lexing text
 */

class Token;

/// class which does the lexing
class Lexer {
    private:
    /// manages code fragments
    CodeManager codeManager;
    /// member variables to save where the lexer is in the code
    size_t lineNum;
    size_t charNum;
    /// bool if it has lexed everything
    bool lexedAll;
    /// advances position by one char
    bool advanceOneChar();
    /// advance postion until first non-whitespace character
    void firstAdvance();

    public:
    /// constructor
    Lexer(CodeManager& codeManager) : codeManager(codeManager), lineNum(0), charNum(0), lexedAll(false) {}
    /// advances Lexer-Algo and determines next token
    std::unique_ptr<Token> next();
};


/// abstract class which represents one token
/// Terminal Symbols grouped in Tokens
class Token {
    public:
    /// CodeFragment
    CodeFragment codeFragment;
    /// all possible types of Tokens
    enum class Type {
        Identifier,
        Literal,
        Operator,
        Keyword,
        Separator
    };
    /// save derived class
    Type type;
    /// normal constructor
    Token(CodeFragment& codeFragment, Type type) : codeFragment(codeFragment), type(type) {}
    /// default constructor
    Token() {}
    /// destructor
    virtual ~Token() = default;
};

/// literal token (64-bit numbers)
class Literal : public Token {
    public:
    int64_t number;
    Literal(CodeFragment& codeFragment);
    /// destructor
    ~Literal() override = default;
};

/// operators token (+, -, *, /, =, :=, Klammer auf und zu)
class Operator : public Token {
    public:
    /// possible operators
    enum class Operators {
        Plus,
        Minus,
        Multiplication,
        Division,
        EqualsInit,
        EqualsAssignment,
        BracketsOpen,
        BracketsClosed
    };
    /// constructor
    Operator(CodeFragment& codeFragment, Operators operators) : Token(codeFragment, Token::Type::Operator), operators(operators) {}
    /// type of operator
    Operators operators;
    /// destructor
    ~Operator() override = default;
};

/// keyword token (PARAM, VAR, CONST, BEGIN, END., RETURN)
class Keyword : public Token {
    public:
    /// possible keywords
    enum class Keywords {
        PARAM,
        VAR,
        CONST,
        BEGIN,
        END,
        RETURN
    };
    /// type of Keyword
    Keywords keywords;
    /// Constructor
    Keyword(CodeFragment& codeFragment, Keywords keywords) : Token(codeFragment, Token::Type::Keyword), keywords(keywords) {}
    /// destructor
    ~Keyword() override = default;
};

/// separator token (, ;)
class Separator : public Token {
    public:
    /// possible separators
    enum class Separators {
        comma,
        semicolon,
        point
    };
    /// type of separator
    Separators separators;
    /// constructor
    Separator(CodeFragment& codeFragment, Separators separators) : Token(codeFragment, Type::Separator), separators(separators) {}
    /// destructor
    ~Separator() override = default;
};


/// identifier token (names which are not keywords)
class Identifier : public Token {
    public:
    Identifier(CodeFragment& codeFragment) : Token(codeFragment, Type::Identifier) {}
    /// destructor
    ~Identifier() override = default;
};

} // namespace lexical_analysis
//--------------------------------------------------------------
#endif