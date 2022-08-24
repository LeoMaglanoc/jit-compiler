#include "1_code_management.hpp"
#include "4_semantic_analysis.hpp"
#include "5_execution.hpp"
//--------------------------------------------------------------
namespace execution {

/// saves all variables in an array with id as index
void Evaluation::evaluateSymbols(std::initializer_list<int64_t> list) {
    auto& hashtable = semanticAnalyser.symbolTable.hashTable;
    identifiers.reserve(hashtable.size());
    for(auto pair : hashtable) {
        auto& identifier = pair.second;
        identifiers[identifier.id] = identifier;
    }
    unsigned index = 0;
    for(int64_t element : list) {
        identifiers[index++].value = element;
    }
}

//--------------------------------------------------------------
// Begin: evaluation functions

int64_t Evaluation::evaluateFunction(std::initializer_list<int64_t> list) {
    evaluateSymbols(list);
    return evaluateStatement(function->statement.get());
}

int64_t Evaluation::evaluateStatement(const semantic_analysis::Statement* statement) {
    // normal statement
    if(statement->type == semantic_analysis::Statement::Type::NormalStatement) {
        return evaluateNormalStatement(semantic_analysis::getNormalStatement(statement));
    // return statement
    } else {
        return evaluateReturnStatement(semantic_analysis::getReturnStatement(statement));
    }
}

int64_t Evaluation::evaluateReturnStatement(const semantic_analysis::ReturnStatement* returnStatement) {
    return evaluateArithmetic(returnStatement->arithmetic.get());
}

int64_t Evaluation::evaluateNormalStatement(const semantic_analysis::NormalStatement* normalStatement) {
    evaluateAssignmentExpression(normalStatement->expression.get());
    return evaluateStatement(normalStatement->nextStatement.get());

}

int64_t Evaluation::evaluateAssignmentExpression(const semantic_analysis::AssignmentExpression* assignmentExpression) {
    int64_t result = evaluateArithmetic(assignmentExpression->arithmetic.get());
    identifiers[assignmentExpression->id].value = result;
    return result;
}

int64_t Evaluation::evaluateArithmetic(const semantic_analysis::Arithmetic* arithmetic) {
    // downcast depending of type of arithmetic
    if(arithmetic->type == semantic_analysis::Arithmetic::Type::BinaryOperator) {
        return evaluateBinaryOperator(semantic_analysis::getBinaryOperator(arithmetic));
    } else if(arithmetic->type == semantic_analysis::Arithmetic::Type::Literal) {
        return evaluateLiteral(semantic_analysis::getLiteral(arithmetic));
    } else if(arithmetic->type == semantic_analysis::Arithmetic::Type::UnaryOperator) {
        return evaluateUnaryOperator(semantic_analysis::getUnaryOperator(arithmetic));
    } else {
        // Identifier
        return evaluateIdentifierNode(semantic_analysis::getIdentifier(arithmetic));
    }
}

int64_t Evaluation::evaluateBinaryOperator(const semantic_analysis::BinaryOperator* binaryOperator) {
    // correct operator depending on type of BinaryOperator
    if(binaryOperator->type == semantic_analysis::BinaryOperator::Type::Division) {
        return evaluateArithmetic(binaryOperator->left.get()) / evaluateArithmetic(binaryOperator->right.get());
    } else if(binaryOperator->type == semantic_analysis::BinaryOperator::Type::Minus) {
        return evaluateArithmetic(binaryOperator->left.get()) - evaluateArithmetic(binaryOperator->right.get());
    } else if(binaryOperator->type == semantic_analysis::BinaryOperator::Type::Multiplication) {
        return evaluateArithmetic(binaryOperator->left.get()) * evaluateArithmetic(binaryOperator->right.get());
    } else {
        /// Plus
        return evaluateArithmetic(binaryOperator->left.get()) + evaluateArithmetic(binaryOperator->right.get());
    }
}

int64_t Evaluation::evaluateUnaryOperator(const semantic_analysis::UnaryOperator* unaryOperator) {
    // correct operator depending on type of UnaryOperator
    if(unaryOperator->type == semantic_analysis::UnaryOperator::Type::Plus) {
        return evaluateArithmetic(unaryOperator->next.get());
    } else {
        /// Minus
        return -evaluateArithmetic(unaryOperator->next.get());
    }
}

int64_t Evaluation::evaluateLiteral(const semantic_analysis::Literal* literal) {
    return literal->number;
}

int64_t Evaluation::evaluateIdentifierNode(const semantic_analysis::IdentifierNode* identifier) {
    return identifiers[identifier->id].value;
}

// End: evaluation functions
//--------------------------------------------------------------

//--------------------------------------------------------------
// Begin: optimize functions

void ConstantPropagation::optimize(std::unique_ptr<semantic_analysis::Function>& function) {
    const semantic_analysis::Statement* statement = function->statement.get();
    const semantic_analysis::NormalStatement* normalStatement = semantic_analysis::getNormalStatement(statement);
    // while-loop to iterate through all statements
    while(normalStatement != nullptr) {
        optimize(normalStatement->expression->arithmetic);
        statement = normalStatement->nextStatement.get();
        normalStatement = semantic_analysis::getNormalStatement(statement);
    }
    semantic_analysis::ReturnStatement* returnStatement = const_cast<semantic_analysis::ReturnStatement*>(semantic_analysis::getReturnStatement(statement));
    optimize(returnStatement->arithmetic);
}

void ConstantPropagation::optimize(std::unique_ptr<semantic_analysis::Arithmetic>& arithmetic) {
    // if arithmetic is BinaryOperator
    if(arithmetic->type == semantic_analysis::Arithmetic::Type::BinaryOperator) {
        semantic_analysis::BinaryOperator* binaryOperator = const_cast<semantic_analysis::BinaryOperator*>(semantic_analysis::getBinaryOperator(arithmetic.get()));
        // optimize childs first
        optimize(binaryOperator->left);
        optimize(binaryOperator->right);
        // then if both childs are literal, combine then to a new literal depending on type of operator
        if(binaryOperator->left->type == semantic_analysis::Arithmetic::Type::Literal && binaryOperator->right->type == semantic_analysis::Arithmetic::Type::Literal) {
            semantic_analysis::Literal* left = const_cast<semantic_analysis::Literal*>(semantic_analysis::getLiteral(binaryOperator->left.get()));
            semantic_analysis::Literal* right = const_cast<semantic_analysis::Literal*>(semantic_analysis::getLiteral(binaryOperator->right.get()));
            std::unique_ptr<semantic_analysis::Literal> literal;
            switch(binaryOperator->type) {
                case semantic_analysis::BinaryOperator::Type::Plus:
                    literal = std::make_unique<semantic_analysis::Literal>(left->number + right->number);
                    break;
                case semantic_analysis::BinaryOperator::Type::Minus:
                    literal = std::make_unique<semantic_analysis::Literal>(left->number - right->number);
                    break;
                case semantic_analysis::BinaryOperator::Type::Division:
                    literal = std::make_unique<semantic_analysis::Literal>(left->number / right->number);
                    break;
                case semantic_analysis::BinaryOperator::Type::Multiplication:
                    literal = std::make_unique<semantic_analysis::Literal>(left->number * right->number);
                    break;
            }
            arithmetic = std::move(literal);
        }
    // if arithmetic is UnaryOperator
    } else if(arithmetic->type == semantic_analysis::Arithmetic::Type::UnaryOperator) {
        semantic_analysis::UnaryOperator* unaryOperator = const_cast<semantic_analysis::UnaryOperator*>(semantic_analysis::getUnaryOperator(arithmetic.get()));
        // optimize child first
        optimize(unaryOperator->next);
        // if child is literal, create new literal depending on type of UnaryOperator
        if(unaryOperator->next->type == semantic_analysis::Arithmetic::Type::Literal) {
            semantic_analysis::Literal* next = const_cast<semantic_analysis::Literal*>(semantic_analysis::getLiteral(unaryOperator->next.get()));
            std::unique_ptr<semantic_analysis::Literal> literal;
            switch(unaryOperator->type) {
                case semantic_analysis::UnaryOperator::Type::Plus:
                    literal = std::make_unique<semantic_analysis::Literal>(next->number);
                    break;
                case semantic_analysis::UnaryOperator::Type::Minus:
                    literal = std::make_unique<semantic_analysis::Literal>(-next->number);
                    break;
            }
            arithmetic = std::move(literal);
        }
    }
}

// End: optimize functions
//--------------------------------------------------------------

/// constructor calls optimization methods after creating function
Evaluation::Evaluation(CodeManager& codeManager) : semanticAnalyser(codeManager), function(semanticAnalyser.analyseFunction()) {
    ConstantPropagation constantPropagation;
    constantPropagation.optimize(function);
}

} // namespace execution
//--------------------------------------------------------------