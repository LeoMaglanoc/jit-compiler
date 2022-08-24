#include "1_code_management.hpp"
#include "3_syntax_analysis.hpp"
#include "4_semantic_analysis.hpp"
#include <string_view>
#include <charconv>
#include <iostream>
#include <optional>
//--------------------------------------------------------------
namespace semantic_analysis {

/// downcasts Arithmetic to UnaryOperator
const UnaryOperator* getUnaryOperator(const Arithmetic* node) {
    switch(node->type) {
        case Arithmetic::Type::UnaryOperator:
            return static_cast<const UnaryOperator*>(node);
        default:
            return nullptr;
    }
}

/// downcasts Arithmetic to BinaryOperator
const BinaryOperator* getBinaryOperator(const Arithmetic* node) {
    switch(node->type) {
        case Arithmetic::Type::BinaryOperator:
            return static_cast<const BinaryOperator*>(node);
        default:
            return nullptr;
    }
}


/// downcasts Arithmetic to Literal
const semantic_analysis::Literal* getLiteral(const Arithmetic* node) {
    switch(node->type) {
        case Arithmetic::Type::Literal:
            return static_cast<const semantic_analysis::Literal*>(node);
        default:
            return nullptr;
    }
}


/// downcasts Arithmetic to IdentifierNode
const IdentifierNode* getIdentifier(const Arithmetic* node) {
    switch(node->type) {
        case Arithmetic::Type::Identifier:
            return static_cast<const IdentifierNode*>(node);
        default:
            return nullptr;
    }
}


/// downcasts Statement to NormalStatement
const NormalStatement* getNormalStatement(const Statement* node) {
    switch(node->type) {
        case Statement::Type::NormalStatement:
            return static_cast<const NormalStatement*>(node);
        default:
            return nullptr;
    }
}

/// downcasts Statement to ReturnStatement
const ReturnStatement* getReturnStatement(const Statement* node) {
    switch(node->type) {
        case Statement::Type::ReturnStatement:
            return static_cast<const ReturnStatement*>(node);
        default:
            return nullptr;
    }
}



/// converts string to int64_t
int64_t stringToInt(std::string_view numString) {
    int64_t number = 0;
    auto result = std::from_chars(numString.data(), numString.data() + numString.size(), number);
    if(result.ec == std::errc::invalid_argument) {
        std::cout << "is not a number" << std::endl;
    }
    return number;
}

/// constructor for Literal
Literal::Literal(std::string_view numString) : Arithmetic(Arithmetic::Type::Literal), number(stringToInt(numString)) {}

/// returns false, if identifier with same name already exists
bool SymbolTable::addIdentifier(code_management::CodeFragment codeFragment, Identifier::Type type) {
    return addIdentifier(codeFragment, type, 0);
}

bool SymbolTable::addIdentifier(code_management::CodeFragment codeFragment, Identifier::Type type, int64_t value) {
    if(hashTable.contains(codeFragment.getString())) {
        return false;
    } else {
        hashTable[codeFragment.getString()] = {codeFragment, type, value, static_cast<unsigned int>(hashTable.size())};
        return true;
    }
}

/// returns nullptr if identifier doesnt exist
const Identifier* SymbolTable::getIdentifier(std::string_view name) {
    if(hashTable.contains(name)) {
        return &hashTable[name];
    } else {
        return nullptr;
    }
}

/// returns root of AST
std::unique_ptr<Function> SemanticAnalyser::analyseFunction() {
    /// goes through all names of declarations and adds them to the symbol table
    if(syntaxTree.parameters.has_value()) {
        const syntax_analysis::DeclaratorList& declaratorListPar = syntaxTree.parameters.value().declaratorList;
        if(!symbolTable.addIdentifier(declaratorListPar.identifier.codeFragment, Identifier::Type::Parameter)) {
            declaratorListPar.identifier.codeFragment.print("error: identifier with the same name already exists!");
            throw "Failure!";
        };
        count_parameters++;
        auto repeating = declaratorListPar.repeating.get();
        while(repeating) {
            if(!symbolTable.addIdentifier(repeating->identifier.codeFragment, Identifier::Type::Parameter)) {
                repeating->identifier.codeFragment.print("error: identifier with the same name already exists!");
                throw "Failure!";
            };
            count_parameters++;
            repeating = repeating->next.get();
        }
    }

    if(syntaxTree.variables.has_value()) {
        const syntax_analysis::DeclaratorList& declaratorListVar = syntaxTree.variables.value().declaratorList;
        if(!symbolTable.addIdentifier(declaratorListVar.identifier.codeFragment, Identifier::Type::Variable)) {
            declaratorListVar.identifier.codeFragment.print("error: identifier with the same name already exists!");
            throw "Failure!";
        };
        auto repeating = declaratorListVar.repeating.get();
        while(repeating) {
            if(!symbolTable.addIdentifier(repeating->identifier.codeFragment, Identifier::Type::Variable)) {
                repeating->identifier.codeFragment.print("error: identifier with the same name already exists!");
                throw "Failure!";
            };
            repeating = repeating->next.get();
        }
    }

    if(syntaxTree.constants.has_value()) {
        const syntax_analysis::InitDeclaratorList& declaratorListConst = syntaxTree.constants.value().initDeclaratorList;
        if(!symbolTable.addIdentifier(declaratorListConst.initDeclarator.identifier.codeFragment, Identifier::Type::Constant, stringToInt(declaratorListConst.initDeclarator.literal.codeFragment.getString()))) {
            declaratorListConst.initDeclarator.identifier.codeFragment.print("error: identifier with the same name already exists!");
            throw "Failure!";
        };
        auto repeating = declaratorListConst.repeating.get();
        while(repeating) {
            if(!symbolTable.addIdentifier(repeating->initDeclarator.identifier.codeFragment, Identifier::Type::Constant)) {
                repeating->initDeclarator.identifier.codeFragment.print("error: identifier with the same name already exists!");
                throw "Failure!";
            };
            repeating = repeating->next.get();
        }
    }

    return std::make_unique<Function>(analyseStatement(syntaxTree.compoundStatement.statementList));
}

/// depending on if there is a next statement, it has to either be a ReturnStatement or a NormalStatement
std::unique_ptr<Statement> SemanticAnalyser::analyseStatement(const syntax_analysis::StatementList& statementList) {
    std::unique_ptr<Statement> next = analyseStatement(statementList.repeating.get());
    if(next != nullptr) {
        std::unique_ptr<AssignmentExpression> expression = analyseAssignmentExpression(statementList.statement.assignmentExpression.value());
        return std::make_unique<NormalStatement>(std::move(expression), std::move(next));
    } else {
        std::unique_ptr<Arithmetic> arithmetic = analyseArithmeticExpression(statementList.statement.returnAdditive->additiveExpression);
        return std::make_unique<ReturnStatement>(std::move(arithmetic));
    }
}

/// parses repeating part of Statement
std::unique_ptr<Statement> SemanticAnalyser::analyseStatement(const syntax_analysis::StatementList::Repeating* repeating) {
    if(!repeating) {
        return nullptr;
    } else {
        std::unique_ptr<Statement> next = analyseStatement(repeating->next.get());
        if(next == nullptr) {
            return std::make_unique<ReturnStatement>(analyseArithmeticExpression(repeating->statement.returnAdditive.value().additiveExpression));
        } else {
            std::unique_ptr<AssignmentExpression> expression = analyseAssignmentExpression(repeating->statement.assignmentExpression.value());
            return std::make_unique<NormalStatement>(std::move(expression), std::move(next));
        }
    }
}

std::unique_ptr<AssignmentExpression> SemanticAnalyser::analyseAssignmentExpression(const syntax_analysis::AssignmentExpression& assignmentExpression) {
    /// check if identifier is in symbolTable + is not const
    const Identifier* identifier = symbolTable.getIdentifier(assignmentExpression.identifier.codeFragment.getString());
    if(!identifier) {
        assignmentExpression.identifier.codeFragment.print("error: identifier not defined!");
        throw "Failure!";
    } else {
        if(identifier->type == Identifier::Type::Constant) {
            assignmentExpression.identifier.codeFragment.print("error: identifier is constant!");
            throw "Failure!";
        } else {
            return std::make_unique<AssignmentExpression>(identifier->id, assignmentExpression.identifier.codeFragment.getString(), analyseArithmeticExpression(assignmentExpression.additiveExpression));
        }
    }
}

std::unique_ptr<Arithmetic> SemanticAnalyser::analyseArithmeticExpression(const syntax_analysis::AdditiveExpression& additiveExpression) {
   std::unique_ptr<Arithmetic> left = analyseArithmeticExpression(additiveExpression.multiplicativeExpression);
   if(additiveExpression.additiveExpression == nullptr) {
       /// no right branch
       return left;
   } else if(additiveExpression.whichOperator == syntax_analysis::AdditiveExpression::WhichOperator::Plus) {
       /// right branch with Plus as BinaryOperator
       return std::make_unique<BinaryOperator>(BinaryOperator::Type::Plus, std::move(left), analyseArithmeticExpression(*additiveExpression.additiveExpression));
   } else {
       /// right branch with Minus as BinaryOperator
       return std::make_unique<BinaryOperator>(BinaryOperator::Type::Minus, std::move(left), analyseArithmeticExpression(*additiveExpression.additiveExpression));
   }
}

std::unique_ptr<Arithmetic> SemanticAnalyser::analyseArithmeticExpression(const syntax_analysis::MultiplicativeExpression& multiplicativeExpression) {
    std::unique_ptr<Arithmetic> left = analyseArithmeticExpression(multiplicativeExpression.unaryExpression);
    if(multiplicativeExpression.multiplicativeExpression == nullptr) {
        /// no right branch
        return left;
    } else if(multiplicativeExpression.whichOperator == syntax_analysis::MultiplicativeExpression::WhichOperator::Multiplication) {
        /// right branch with Multiplication as BinaryOperator
        return std::make_unique<BinaryOperator>(BinaryOperator::Type::Multiplication, std::move(left), analyseArithmeticExpression(*multiplicativeExpression.multiplicativeExpression));
    } else {
        /// right branch with Division as BinaryOperator
        return std::make_unique<BinaryOperator>(BinaryOperator::Type::Division, std::move(left), analyseArithmeticExpression(*multiplicativeExpression.multiplicativeExpression));
    }
}

std::unique_ptr<Arithmetic> SemanticAnalyser::analyseArithmeticExpression(const syntax_analysis::UnaryExpression& unaryExpression) {
    std::unique_ptr<Arithmetic> next = analyseArithmeticExpression(unaryExpression.primaryExpression);
    if(unaryExpression.whichOperator == syntax_analysis::UnaryExpression::WhichOperator::Plus) {
        /// UnaryNode with Plus
        return std::make_unique<UnaryOperator>(UnaryOperator::Type::Plus, std::move(next));
    } else if(unaryExpression.whichOperator == syntax_analysis::UnaryExpression::WhichOperator::Minus) {
        /// UnaryNode with Minus
        return std::make_unique<UnaryOperator>(UnaryOperator::Type::Minus, std::move(next));
    } else {
        /// no extra unary operator
        return next;
    }
}

std::unique_ptr<Arithmetic> SemanticAnalyser::analyseArithmeticExpression(const syntax_analysis::PrimaryExpression& primaryExpression) {
    if(primaryExpression.whichAlternative == syntax_analysis::PrimaryExpression::WhichAlternative::Identifier) {
        /// its an identifier
        /// check if identifier is in symbolTable
        const Identifier* identifier = symbolTable.getIdentifier(primaryExpression.terminalNode.value().codeFragment.getString());
        if(!identifier) {
            /// identifier is not in symbolTable
            primaryExpression.terminalNode.value().codeFragment.print("error: identifier is not defined!");
            throw "Failure!";
        } else if(identifier->type == Identifier::Type::Constant) {
            return std::make_unique<Literal>(identifier->value);
        } else {
            return std::make_unique<IdentifierNode>(identifier->id, identifier->name);
        }
    } else if(primaryExpression.whichAlternative == syntax_analysis::PrimaryExpression::WhichAlternative::Literal) {
        /// its a literal
        return std::make_unique<Literal>(primaryExpression.terminalNode.value().codeFragment.getString());
    } else {
        /// its an AdditiveExpression
        return analyseArithmeticExpression(*primaryExpression.additiveExpressionBrackets.value().additiveExpression);
    }
}

void Print::visit(const Function& node) {
    std::cout << maxId++ << " [label = \"function\"]" << std::endl;
    const NormalStatement* normalStatement = getNormalStatement(node.statement.get());
    // its a normalStatement
    if(normalStatement != nullptr) {
        std::cout << maxId++ << " [label = \"normalStatement\"]" << std::endl;
    // its a returnStatement
    } else {
        std::cout << maxId++ << " [label = \"returnStatement\"]" << std::endl;
    }
    std::cout << maxId - 2 << " -> " << maxId - 1 << std::endl;
    visit(*node.statement, maxId - 1);
}

void Print::visit(const Arithmetic& node, unsigned thisId) {
    const
}

void Print::visit(const BinaryOperator& node, unsigned thisId) {

}

void Print::visit(const UnaryOperator& node, unsigned thisId) {

}

void Print::visit(const Literal& node, unsigned thisId) {

}

void Print::visit(const IdentifierNode& node, unsigned thisId) {

}

void Print::visit(const Statement& node, unsigned thisId) {

}

void Print::visit(const NormalStatement& node, unsigned thisId) {

}

void Print::visit(const ReturnStatement& node, unsigned thisId) {

}

void Print::visit(const MultiplicativeExpression& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"unaryExpression\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.unaryExpression, maxId - 1);
    if(node.whichOperator == MultiplicativeExpression::WhichOperator::Multiplication) {
        std::cout << maxId++ << " [label = \"*\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"multiplicativeExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(*node.multiplicativeExpression, maxId - 1);
    } else if(node.whichOperator == MultiplicativeExpression::WhichOperator::Divide) {
        std::cout << maxId++ << " [label = \"/\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"multiplicativeExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(*node.multiplicativeExpression, maxId - 1);
    }
}


} // namespace semantic_analysis
//--------------------------------------------------------------