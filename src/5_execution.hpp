#ifndef H_5_execution
#define H_5_execution
#include "4_semantic_analysis.hpp"
//--------------------------------------------------------------
namespace execution {

/*
 * most important class is evaluation, which provides method to execute a function
 */

class Evaluation {
    private:
    std::vector<semantic_analysis::Identifier> identifiers;
    semantic_analysis::SemanticAnalyser semanticAnalyser;
    public:
    std::unique_ptr<semantic_analysis::Function> function;
    /// constructor
    explicit Evaluation(CodeManager& codeManager);
    /// saves all symbols in an array with id as index
    void evaluateSymbols(std::initializer_list<int64_t> list);
    /// evaluation function for all AST-node types
    int64_t evaluateFunction(std::initializer_list<int64_t> list);
    private:
    int64_t evaluateStatement(const semantic_analysis::Statement* statement);
    int64_t evaluateReturnStatement(const semantic_analysis::ReturnStatement* returnStatement);
    int64_t evaluateNormalStatement(const semantic_analysis::NormalStatement* normalStatement);
    int64_t evaluateAssignmentExpression(const semantic_analysis::AssignmentExpression* assignmentExpression);
    int64_t evaluateArithmetic(const semantic_analysis::Arithmetic* arithmetic);
    int64_t evaluateBinaryOperator(const semantic_analysis::BinaryOperator* binaryOperator);
    int64_t evaluateUnaryOperator(const semantic_analysis::UnaryOperator* unaryOperator);
    int64_t evaluateLiteral(const semantic_analysis::Literal* literal);
    int64_t evaluateIdentifierNode(const semantic_analysis::IdentifierNode* identifier);
};

class Optimization {
    public:
    virtual void optimize(std::unique_ptr<semantic_analysis::Function>& function) = 0;
};

/// already implemented dead code elimination in Milestone 2, parser ignores everything after "."
class DeadCodeElimination {};

class ConstantPropagation : public Optimization {
    public:
    void optimize(std::unique_ptr<semantic_analysis::Function>& function) override;
    private:
    void optimize(std::unique_ptr<semantic_analysis::Arithmetic>& arithmetic);
};

} // namespace execution
//--------------------------------------------------------------
#endif