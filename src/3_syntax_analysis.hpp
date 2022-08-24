#ifndef H_3_syntax_analysis
#define H_3_syntax_analysis
#include "1_code_management.hpp"
#include "2_lexer.hpp"
#include <optional>
#include <vector>
#include <memory>
//--------------------------------------------------------------

using namespace code_management;
using namespace lexical_analysis;

namespace syntax_analysis {

/*
 * most important class is SyntaxAnalyser, which provides methods for creating SyntaxTree
 */

/* Node types:
 * Terminal Symbols with no children: Identifier, Literal, Generic (Operator, Keyword, Separator)
 * Non-Terminal Symbols with children:
 * if alternation then also store which alternative it represents as additional data member --> optional + enum which says which alternative gets used
 * if optional --> optional-data-struct []
 * if repeating --> linked list of struct for repeating part {}
 */

/// each grammar component has a special class
// Beginning: Grammar Components
//--------------------------------------------------------------------


/// Terminal Node with no children
class TerminalNode {
    public:
    CodeFragment codeFragment;
    /// type of terminal node
    enum class Type {
        Identifier,
        Literal,
        Generic // (Operator, Keyword, Separator)
    };
    Type type;
    /// default constructor
    TerminalNode() : type(Type::Generic) {}
    /// Constructor
    explicit TerminalNode(CodeFragment& codeFragment, Type type) : codeFragment(codeFragment), type(type) {}
};

class AdditiveExpression;

class PrimaryExpression {
    public:
    enum class WhichAlternative {
        Identifier,
        Literal,
        AdditiveExpression
    };
    /// struct which saves brackets + AdditiveExpression
    struct AdditiveExpressionBrackets {
        TerminalNode leftBracket;
        std::unique_ptr<AdditiveExpression> additiveExpression;
        TerminalNode rightBracket;
        /// constructor
        AdditiveExpressionBrackets(TerminalNode& leftBracket, std::unique_ptr<AdditiveExpression> additiveExpression, TerminalNode& rightBracket)
            : leftBracket(leftBracket), additiveExpression(std::move(additiveExpression)), rightBracket(rightBracket) {}
    };
    /// says which alternative is used
    WhichAlternative whichAlternative;
    std::optional<TerminalNode> terminalNode;
    std::optional<AdditiveExpressionBrackets> additiveExpressionBrackets;
    /// constructor with TerminalNode
    PrimaryExpression(WhichAlternative whichAlternative, TerminalNode terminalNode)
        : whichAlternative(whichAlternative), terminalNode(std::make_optional<TerminalNode>(terminalNode)), additiveExpressionBrackets(std::nullopt) {}
    /// constructor with AdditiveExpressionBrackets
    PrimaryExpression(WhichAlternative whichAlternative, AdditiveExpressionBrackets additiveExpressionBrackets)
        : whichAlternative(whichAlternative), terminalNode(std::nullopt), additiveExpressionBrackets(std::move(additiveExpressionBrackets)) {}
};

class UnaryExpression {
    public:
    enum class WhichOperator {
        Plus,
        Minus,
        NoOperator
    };
    /// says which operator was used
    WhichOperator whichOperator;
    std::optional<TerminalNode> usedOperator;
    PrimaryExpression primaryExpression;
    UnaryExpression(WhichOperator whichOperator,
                    std::optional<TerminalNode> usedOperator,
                    PrimaryExpression primaryExpression)
        : whichOperator(whichOperator), usedOperator(std::move(usedOperator)), primaryExpression(std::move(primaryExpression)) {}
};

class MultiplicativeExpression {
    public:
    enum class WhichOperator {
        Multiplication,
        Divide,
        None
    };
    /// says which operator is used
    WhichOperator whichOperator;
    UnaryExpression unaryExpression;
    std::optional<TerminalNode> timesOrDivide;
    std::unique_ptr<MultiplicativeExpression> multiplicativeExpression;
    /// constructor
    MultiplicativeExpression(WhichOperator whichOperator,
                             UnaryExpression unaryExpression,
                             std::optional<TerminalNode> timesOrDivide,
                             std::unique_ptr<MultiplicativeExpression> multiplicativeExpression)
        : whichOperator(whichOperator), unaryExpression(std::move(unaryExpression)),
          timesOrDivide(std::move(timesOrDivide)), multiplicativeExpression(std::move(multiplicativeExpression)) {}
};

class AdditiveExpression  {
    public:
    enum class WhichOperator {
        Plus,
        Minus,
        None
    };
    /// says which operator is used
    WhichOperator whichOperator;
    MultiplicativeExpression multiplicativeExpression;
    std::optional<TerminalNode> plusOrMinus;
    std::unique_ptr<AdditiveExpression> additiveExpression;
    /// constructor
    AdditiveExpression(WhichOperator whichOperator,
                       MultiplicativeExpression multiplicativeExpression,
                       std::optional<TerminalNode> plusOrMinus,
                       std::unique_ptr<AdditiveExpression> additiveExpression)
        : whichOperator(whichOperator), multiplicativeExpression(std::move(multiplicativeExpression)),
          plusOrMinus(std::move(plusOrMinus)), additiveExpression(std::move(additiveExpression)) {}
};

class AssignmentExpression  {
    public:
    TerminalNode identifier;
    TerminalNode equalsAssignment;
    AdditiveExpression additiveExpression;
    /// constructor
    AssignmentExpression(TerminalNode identifier,
                         TerminalNode equalsAssignment,
                         AdditiveExpression additiveExpression)
        : identifier(std::move(identifier)), equalsAssignment(std::move(equalsAssignment)), additiveExpression(std::move(additiveExpression)) {}
};

class Statement {
    public:
    /// struct for Return Terminal Node + AdditiveExpression
    struct ReturnAdditive {
        TerminalNode RETURN;
        AdditiveExpression additiveExpression;
        /// constructor
        ReturnAdditive(TerminalNode& RETURN,
                       AdditiveExpression& additiveExpression)
            : RETURN(RETURN), additiveExpression(std::move(additiveExpression)) {}
    };
    enum class WhichAlternative {
        AssignmentExpression,
        ReturnAdditive
    };
    /// says which alternative is used
    WhichAlternative whichAlternative;
    std::optional<AssignmentExpression> assignmentExpression;
    std::optional<ReturnAdditive> returnAdditive;
    /// constructor
    Statement(WhichAlternative whichAlternative,
              std::optional<AssignmentExpression> assignmentExpression,
              std::optional<ReturnAdditive> returnAdditive)
        : whichAlternative(whichAlternative), assignmentExpression(std::move(assignmentExpression)), returnAdditive(std::move(returnAdditive)) {}
};

class StatementList {
    public:
    /// struct to save repeating part of StatementList via linked list
    struct Repeating {
        TerminalNode semiColon;
        Statement statement;
        std::unique_ptr<Repeating> next;
        /// constructor
        Repeating(TerminalNode semiColon,
                  Statement statement,
                  std::unique_ptr<Repeating> next)
            : semiColon(std::move(semiColon)), statement(std::move(statement)), next(std::move(next)) {}
    };
    Statement statement;
    std::unique_ptr<Repeating> repeating;
    /// constructor
    StatementList(Statement statement, std::unique_ptr<Repeating> repeating)
        : statement(std::move(statement)), repeating(std::move(repeating)) {}
};

class CompoundStatement {
    public:
    TerminalNode BEGIN;
    StatementList statementList;
    TerminalNode END;
    /// constructor
    CompoundStatement(TerminalNode BEGIN,
                      StatementList statementList,
                      TerminalNode END)
        : BEGIN(std::move(BEGIN)), statementList(std::move(statementList)), END(std::move(END)) {}
};

class InitDeclarator {
    public:
    TerminalNode identifier;
    TerminalNode equals;
    TerminalNode literal;
    /// constructor
    InitDeclarator(TerminalNode identifier, TerminalNode equals, TerminalNode literal)
        : identifier(std::move(identifier)), equals(std::move(equals)), literal(std::move(literal)) {}
};

class InitDeclaratorList {
    public:
    /// struct to save repeating part of InitDeclaratorList via linked list
    struct Repeating {
        TerminalNode colon;
        InitDeclarator initDeclarator;
        std::unique_ptr<Repeating> next;
        /// constructor
        Repeating(TerminalNode colon, InitDeclarator initDeclarator, std::unique_ptr<Repeating> next)
            : colon(std::move(colon)), initDeclarator(std::move(initDeclarator)), next(std::move(next)) {}
    };
    InitDeclarator initDeclarator;
    std::unique_ptr<Repeating> repeating;
    /// constructor
    InitDeclaratorList(InitDeclarator initDeclarator, std::unique_ptr<Repeating> repeating)
        : initDeclarator(std::move(initDeclarator)), repeating(std::move(repeating)) {}
};

class DeclaratorList {
    public:
    /// struct to save repeating part of DeclaratorList via linked list
    struct Repeating {
        public:
        TerminalNode colon;
        TerminalNode identifier;
        std::unique_ptr<Repeating> next;
        /// constructor
        Repeating(TerminalNode colon, TerminalNode identifier, std::unique_ptr<Repeating> next)
            : colon(std::move(colon)), identifier(std::move(identifier)), next(std::move(next)) {}

    };
    TerminalNode identifier;
    std::unique_ptr<Repeating> repeating;
    /// constructor
    DeclaratorList(TerminalNode identifier, std::unique_ptr<Repeating> repeating)
        : identifier(std::move(identifier)), repeating(std::move(repeating)) {}
};

class ConstantDeclarations {
    public:
    TerminalNode CONST;
    InitDeclaratorList initDeclaratorList;
    TerminalNode semiColon;
    /// normal constructor
    ConstantDeclarations(TerminalNode CONST,
                         InitDeclaratorList initDeclaratorList,
                         TerminalNode semiColon)
        : CONST(std::move(CONST)), initDeclaratorList(std::move(initDeclaratorList)), semiColon(std::move(semiColon)) {}
};

class VariableDeclarations {
    public:
    TerminalNode VAR;
    DeclaratorList declaratorList;
    TerminalNode semiColon;
    /// normal constructor
    VariableDeclarations(TerminalNode& VAR,
                          DeclaratorList& declaratorList,
                          TerminalNode& semiColon)
        : VAR(std::move(VAR)), declaratorList(std::move(declaratorList)), semiColon(std::move(semiColon)) {}
};

class ParameterDeclarations {
    public:
    TerminalNode PARAM;
    DeclaratorList declaratorList;
    TerminalNode semiColon;
    /// normal constructor
    ParameterDeclarations(TerminalNode PARAM,
                          DeclaratorList declaratorList,
                          TerminalNode semiColon)
        : PARAM(std::move(PARAM)), declaratorList(std::move(declaratorList)), semiColon(std::move(semiColon)) {}
};

class FunctionDefinition {
    public:
    /// declarations can be optional, because they can be empty
    std::optional<ParameterDeclarations> parameters;
    std::optional<VariableDeclarations> variables;
    std::optional<ConstantDeclarations> constants;
    CompoundStatement compoundStatement;
    TerminalNode dot;
    /// normal constructor
    FunctionDefinition(std::optional<ParameterDeclarations> parameters,
                       std::optional<VariableDeclarations> variables,
                       std::optional<ConstantDeclarations> constants,
                       CompoundStatement compoundStatement,
                       TerminalNode dot)
        : parameters(std::move(parameters)), variables(std::move(variables)),
          constants(std::move(constants)), compoundStatement(std::move(compoundStatement)), dot(std::move(dot)) {}
};

//--------------------------------------------------------------------
// End: Grammar Components

/// creates Syntax Tree based on grammar
class SyntaxAnalyser {
    private:
    /// creates CodeFragments and saves ref to sourceCode
    CodeManager codeManager;
    Lexer lexer;
    std::unique_ptr<Token> token_ptr;
    Token* token;
    public:
    /// Constructor
    SyntaxAnalyser(CodeManager& codeManager) : codeManager(codeManager), lexer(codeManager), token_ptr(nullptr), token(nullptr) {}
    /// parse function-definition
    FunctionDefinition parseFunctionDefinition();
    private:
    /// get new token if nullpointer
    void getToken();
    /// delete token if used up
    void deleteToken();
    /// parse parameter-declarations
    std::optional<ParameterDeclarations> parseParameterDeclarations();
    /// parse variable-declarations
    std::optional<VariableDeclarations> parseVariableDeclarations();
    /// parse constant-declarations
    std::optional<ConstantDeclarations> parseConstantDeclarations();
    /// parse declarator-list
    DeclaratorList parseDeclaratorList();
    /// parse repeating part of DeclaratorList
    std::unique_ptr<DeclaratorList::Repeating> parseRepeatingDeclaratorList();
    /// parse init-declarator-list
    InitDeclaratorList parseInitDeclaratorList();
    /// parse repeating part of InitDeclaratorList
    std::unique_ptr<InitDeclaratorList::Repeating> parseRepeatingInitDeclaratorList();
    /// parse init-declarator
    InitDeclarator parseInitDeclarator();
    /// parse compound-statement
    CompoundStatement parseCompoundStatement();
    /// parse statement-list
    StatementList parseStatementList();
    /// parse repeating part of StatementList
    std::unique_ptr<StatementList::Repeating> parseRepeatingStatementList();
    /// parse statement
    Statement parseStatement();
    /// parse assignment-expression
    AssignmentExpression parseAssignmentExpression();
    /// parse additive-expression
    std::optional<AdditiveExpression> parseAdditiveExpression();
    /// parse multiplicative-expression
    std::optional<MultiplicativeExpression> parseMultiplicativeExpression();
    /// parse unary-expression
    std::optional<UnaryExpression> parseUnaryExpression();
    /// parse primary-expression
    std::optional<PrimaryExpression> parsePrimaryExpression();

};


/// implements printing of SyntaxTree
class Print {
    private:
    /// unique id for nodes
    unsigned maxId = 0;
    public:
    /// visit methods for all node types
    void visit(const FunctionDefinition& node);
    void visit(const ParameterDeclarations& node, unsigned thisId);
    void visit(const VariableDeclarations& node, unsigned thisId);
    void visit(const ConstantDeclarations& node, unsigned thisId);
    void visit(const DeclaratorList& node, unsigned thisId);
    void visit(const DeclaratorList::Repeating* node, unsigned thisId);
    void visit(const InitDeclaratorList& node, unsigned thisId);
    void visit(const InitDeclaratorList::Repeating* node, unsigned thisId);
    void visit(const InitDeclarator& node, unsigned thisId);
    void visit(const CompoundStatement& node, unsigned thisId);
    void visit(const StatementList& node, unsigned thisId);
    void visit(const Statement& node, unsigned thisId);
    void visit(const StatementList::Repeating* node, unsigned thisId, unsigned statementId);
    void visit(const AssignmentExpression& node, unsigned thisId);
    void visit(const AdditiveExpression& node, unsigned thisId);
    void visit(const MultiplicativeExpression& node, unsigned thisId);
    void visit(const UnaryExpression& node, unsigned thisId);
    void visit(const PrimaryExpression& node, unsigned thisId);
};



//--------------------------------------------------------------------

} // namespace syntax_analysis
//--------------------------------------------------------------
#endif