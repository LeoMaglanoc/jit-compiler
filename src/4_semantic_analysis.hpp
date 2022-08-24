#ifndef H_4_semantic_analysis
#define H_4_semantic_analysis
#include "1_code_management.hpp"
#include "2_lexer.hpp"
#include "3_syntax_analysis.hpp"
#include <unordered_map>
//--------------------------------------------------------------
namespace semantic_analysis {

/*
 * most important class is SemanticAnalyser, which provides methods to create ASTNodes
 */

/// saves information about Identifier
class Identifier {
    public:
    enum Type {
        Parameter,
        Variable,
        Constant
    };
    std::string_view name;
    code_management::CodeFragment codeFragment;
    Type type;
    int64_t value = 0;
    unsigned id;
    /// default constructor
    Identifier() = default;
    /// constructor
    Identifier(code_management::CodeFragment codeFragment, Type type, unsigned id)
        : name(codeFragment.getString()), codeFragment(codeFragment), type(type), id(id) {}
    Identifier(code_management::CodeFragment codeFragment, Type type, int64_t value, unsigned id)
        : name(codeFragment.getString()), codeFragment(codeFragment), type(type), value(value), id(id) {}
};

/// SymbolTable which saves all identifiers + information
class SymbolTable {
    public:
    std::unordered_map<std::string_view, Identifier> hashTable;
    /// constructor
    SymbolTable() = default;
    /// methods
    /// returns false, if identifier with same name already exists
    bool addIdentifier(code_management::CodeFragment codeFragment, Identifier::Type type);
    bool addIdentifier(code_management::CodeFragment codeFragment, Identifier::Type type, int64_t value);
    /// returns nullptr if identifier doesnt exist
    const Identifier* getIdentifier(std::string_view name);
};

/// virtual class of ASTNode
class ASTNode {
    public:
    /// possible derived classes
    enum class Type {
        Function, // function has one statement
        Statement, // statement can either have expression + next statement or return + expression
        AssignmentExpression, // identifier + arithmetic
        Arithmetic // leftArithmetic + Operator + rightArithmetic or plus/minus + Arithmetic or literal or identifier
    };
    Type type;
    /// constructor
    explicit ASTNode(Type type) : type(type) {}
    /// to make class virtual
    virtual ~ASTNode() = default;
};

/// virtual arithmetic node
class Arithmetic : public ASTNode {
    public:
    /// possible derived classes
    enum class Type {
        BinaryOperator,
        UnaryOperator,
        Literal,
        Identifier
    };
    Type type;
    /// constructor
    explicit Arithmetic(Type type) : ASTNode(ASTNode::Type::Arithmetic), type(type) {}
    /// destructor
    virtual ~Arithmetic() = default;
};

/// BinaryOperator
class BinaryOperator : public Arithmetic {
    public:
    /// which operators
    enum class Type {
        Plus,
        Minus,
        Multiplication,
        Division
    };
    Type type;
    /// left node
    std::unique_ptr<Arithmetic> left;
    /// right node
    std::unique_ptr<Arithmetic> right;
    /// constructor
    BinaryOperator(Type type, std::unique_ptr<Arithmetic> left, std::unique_ptr<Arithmetic> right)
        : Arithmetic(Arithmetic::Type::BinaryOperator), type(type), left(std::move(left)), right(std::move(right)) {}
    /// destructor
    ~BinaryOperator() override = default;
};

/// UnaryOperator
class UnaryOperator : public Arithmetic {
    public:
    /// which operators
    enum class Type {
        Plus,
        Minus
    };
    Type type;
    /// next arithmetic
    std::unique_ptr<Arithmetic> next;
    /// constructor
    UnaryOperator(Type type, std::unique_ptr<Arithmetic> next)
        : Arithmetic(Arithmetic::Type::UnaryOperator), type(type), next(std::move(next)) {}
    /// destructor
    ~UnaryOperator() override = default;
};

/// Literal
class Literal : public Arithmetic {
    public:
    int64_t number;
    /// constructor
    explicit Literal(std::string_view numString);
    explicit Literal(int64_t number) : Arithmetic(Arithmetic::Type::Literal), number(number) {}
    /// destructor
    ~Literal() override = default;
};

/// Identifier
class IdentifierNode : public Arithmetic {
    public:
    unsigned id;
    std::string_view name;
    /// constructor
    explicit IdentifierNode(unsigned id, std::string_view name)
        : Arithmetic(Arithmetic::Type::Identifier), id(id), name(name) {}
    /// destructor
    ~IdentifierNode() override = default;
};

/// assignmentExpression node
class AssignmentExpression : public ASTNode {
    public:
    unsigned id;
    std::string_view identifier;
    std::unique_ptr<Arithmetic> arithmetic;
    /// constructor
    AssignmentExpression(unsigned id, std::string_view identifier, std::unique_ptr<Arithmetic> arithmetic)
        : ASTNode(ASTNode::Type::AssignmentExpression), id(id), identifier(identifier), arithmetic(std::move(arithmetic)) {}
    /// destructor
    ~AssignmentExpression() override = default;
};

/// statement node
class Statement : public ASTNode {
    public:
    enum class Type {
        NormalStatement, // contains statement + next statement
        ReturnStatement // only contains arithmetic
    };
    Type type;
    /// constructor
    explicit Statement(Type type) : ASTNode(ASTNode::Type::Statement), type(type) {}
    /// destructor
    virtual ~Statement() = default;
};

/// NormalStatement node
class NormalStatement : public Statement {
    public:
    std::unique_ptr<AssignmentExpression> expression;
    std::unique_ptr<Statement> nextStatement;
    /// constructor
    NormalStatement(std::unique_ptr<AssignmentExpression> expression, std::unique_ptr<Statement> nextStatement)
        : Statement(Statement::Type::NormalStatement), expression(std::move(expression)), nextStatement(std::move(nextStatement)) {}
    /// destructor
    ~NormalStatement() override = default;
};

/// ReturnStatement node
class ReturnStatement : public Statement {
    public:
    std::unique_ptr<Arithmetic> arithmetic;
    /// constructor
    explicit ReturnStatement(std::unique_ptr<Arithmetic> arithmetic)
        : Statement(Statement::Type::ReturnStatement), arithmetic(std::move(arithmetic)) {}
    /// destructor
    ~ReturnStatement() override = default;
};

/// function node
class Function : public ASTNode {
    public:
    std::unique_ptr<Statement> statement;
    /// constructor
    explicit Function(std::unique_ptr<Statement> statement) : ASTNode(ASTNode::Type::Function), statement(std::move(statement)) {}
    /// destructor
    ~Function() override = default;
};

/// analyses semantics of program
class SemanticAnalyser {
    private:
    syntax_analysis::SyntaxAnalyser syntaxAnalyser;
    const syntax_analysis::FunctionDefinition syntaxTree;
    public:
    /// how many parameters
    unsigned count_parameters = 0;
    /// saves all symbols
    SymbolTable symbolTable;
    explicit SemanticAnalyser(CodeManager& codeManager) : syntaxAnalyser(codeManager), syntaxTree(syntaxAnalyser.parseFunctionDefinition()) {}
    /// analyse methods: for each ASTNode type, there is an analyse-method which takes a reference
    /// for the corresponding parse node and returns an ASTNode
    std::unique_ptr<Function> analyseFunction();
    private:
    std::unique_ptr<Statement> analyseStatement(const syntax_analysis::StatementList& statementList);
    std::unique_ptr<Statement> analyseStatement(const syntax_analysis::StatementList::Repeating* repeating);
    std::unique_ptr<AssignmentExpression> analyseAssignmentExpression(const syntax_analysis::AssignmentExpression& assignmentExpression);
    std::unique_ptr<Arithmetic> analyseArithmeticExpression(const syntax_analysis::AdditiveExpression& additiveExpression);
    std::unique_ptr<Arithmetic> analyseArithmeticExpression(const syntax_analysis::MultiplicativeExpression& multiplicativeExpression);
    std::unique_ptr<Arithmetic> analyseArithmeticExpression(const syntax_analysis::UnaryExpression& unaryExpression);
    std::unique_ptr<Arithmetic> analyseArithmeticExpression(const syntax_analysis::PrimaryExpression& primaryExpression);
};

/// downcasts Arithmetic to UnaryOperator
const UnaryOperator* getUnaryOperator(const Arithmetic* node);
/// downcasts Arithmetic to BinaryOperator
const BinaryOperator* getBinaryOperator(const Arithmetic* node);
/// downcasts Arithmetic to Literal
const Literal* getLiteral(const Arithmetic* node);
/// downcasts Arithmetic to IdentifierNode
const IdentifierNode* getIdentifier(const Arithmetic* node);
/// downcasts Statement to NormalStatement
const NormalStatement* getNormalStatement(const Statement* node);
/// downcasts Statement to ReturnStatement
const ReturnStatement* getReturnStatement(const Statement* node);
/// converts string to int64_t
int64_t stringToInt(std::string_view numString);

/// implements printing of ASTNode
class Print {
    private:
    /// unique id for nodes
    unsigned maxId = 0;
    public:
    /// visit methods for all node types
    void visit(const Function& node);
    void visit(const Arithmetic& node, unsigned thisId);
    void visit(const BinaryOperator& node, unsigned thisId);
    void visit(const UnaryOperator& node, unsigned thisId);
    void visit(const Literal& node, unsigned thisId);
    void visit(const IdentifierNode& node, unsigned thisId);
    void visit(const Statement& node, unsigned thisId);
    void visit(const NormalStatement& node, unsigned thisId);
    void visit(const ReturnStatement& node, unsigned thisId);
};

} // namespace semantic_analysis
//--------------------------------------------------------------
#endif