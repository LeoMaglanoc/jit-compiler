#include "3_syntax_analysis.hpp"
#include "iostream"
//--------------------------------------------------------------
namespace syntax_analysis {

/// downcasts Token to Keyword
static Keyword* getKeyword(Token* token) {
    switch(token->type) {
        case Token::Type::Keyword:
            return static_cast<Keyword*>(token);
        default:
            return nullptr;
    }
}

/// downcasts Token to Separator
static Separator* getSeparator(Token* token) {
    switch(token->type) {
        case Token::Type::Separator:
            return static_cast<Separator*>(token);
        default:
            return nullptr;
    }
}

/// downcasts Token to Operator
static Operator* getOperator(Token* token) {
    switch(token->type) {
        case Token::Type::Operator:
            return static_cast<Operator*>(token);
        default:
            return nullptr;
    }
}

/// downcasts Token to Literal
static Literal* getLiteral(Token* token) {
    switch(token->type) {
        case Token::Type::Literal:
            return static_cast<Literal*>(token);
        default:
            return nullptr;
    }
}

/// get new token if nullpointer
void SyntaxAnalyser::getToken() {
    if (!token) {
        token_ptr = lexer.next();
        token = token_ptr.get();
    }
}

/// delete token if used up
void SyntaxAnalyser::deleteToken() {
    token = token_ptr.release();
    token = nullptr;
}

/// parse function-definition
FunctionDefinition SyntaxAnalyser::parseFunctionDefinition() {
    auto parameters = parseParameterDeclarations();
    auto variables = parseVariableDeclarations();
    auto constants = parseConstantDeclarations();
    auto compoundStatement = parseCompoundStatement();
    // parse dot
    getToken();
    if(token == nullptr) {
        throw "failure!";
    }
    Separator* separator = getSeparator(token);
    // check if token is a point
    if(separator && separator->separators == Separator::Separators::point) {
        TerminalNode dot(separator->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        return {std::move(parameters), std::move(variables), std::move(constants), std::move(compoundStatement), std::move(dot)};
    }
    token->codeFragment.print("error: point missing");
    throw "failure!";
}

/// parse parameter-declarations
std::optional<ParameterDeclarations> SyntaxAnalyser::parseParameterDeclarations() {
    getToken();
    const Keyword* keyword = getKeyword(token);
    // check if token is PARAM
    if(keyword && keyword->keywords == Keyword::Keywords::PARAM) {
        TerminalNode PARAM(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto declaratorList = parseDeclaratorList();
        getToken();
        Separator* separator = getSeparator(token);
        // check if token is semicolon
        if(separator && separator->separators == Separator::Separators::semicolon) {
            TerminalNode semiColon(token->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            return std::make_optional<ParameterDeclarations>(std::move(PARAM), std::move(declaratorList), std::move(semiColon));
        } else {
            token->codeFragment.print("error: semicolon missing");
        }
    } else {
        return std::nullopt;
    }
    throw "failure!";
}

/// parse variable-declarations
std::optional<VariableDeclarations> SyntaxAnalyser::parseVariableDeclarations() {
    getToken();
    const Keyword* keyword = getKeyword(token);
    // check if token is VAR
    if(keyword && keyword->keywords == Keyword::Keywords::VAR) {
        TerminalNode VAR(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto declaratorList = parseDeclaratorList();
        getToken();
        Separator* separator = getSeparator(token);
        // check if token is semicolon
        if(separator && separator->separators == Separator::Separators::semicolon) {
            TerminalNode semiColon(token->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            return std::make_optional<VariableDeclarations>(VAR, declaratorList, semiColon);
        } else {
            token->codeFragment.print("error: semicolon missing");
        }
    } else {
        return std::nullopt;
    }
    throw "failure!";
}

/// parse constant-declarations
std::optional<ConstantDeclarations> SyntaxAnalyser::parseConstantDeclarations() {
    getToken();
    const Keyword* keyword = getKeyword(token);
    // check if token is CONST
    if(keyword && keyword->keywords == Keyword::Keywords::CONST) {
        TerminalNode CONST(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto initDeclaratorList = parseInitDeclaratorList();
        getToken();
        Separator* separator = getSeparator(token);
        // check if token is semicolon
        if(separator && separator->separators == Separator::Separators::semicolon) {
            TerminalNode semiColon(token->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            return std::make_optional<ConstantDeclarations>(std::move(CONST), std::move(initDeclaratorList), std::move(semiColon));
        } else {
            token->codeFragment.print("error: semicolon missing");
        }
    } else {
        return std::nullopt;
    }
    throw "failure!";
}

/// parse declarator-list
DeclaratorList SyntaxAnalyser::parseDeclaratorList() {
    getToken();
    // check if token is identifier
    if(token->type == Token::Type::Identifier) {
        TerminalNode terminalNode(token->codeFragment, TerminalNode::Type::Identifier);
        deleteToken();
        auto repeating = parseRepeatingDeclaratorList();
        return {terminalNode, std::move(repeating)};
    } else {
        token->codeFragment.print("error: expected identifier");
        throw "failure!";
    }

}

/// parse repeating part of DeclaratorList
std::unique_ptr<DeclaratorList::Repeating> SyntaxAnalyser::parseRepeatingDeclaratorList() {
    getToken();
    Separator* separator = getSeparator(token);
    // check if token is comma
    if(separator && separator->separators == Separator::Separators::comma) {
        TerminalNode colon(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        getToken();
        // check if token is identifier
        if (token->type == Token::Type::Identifier) {
            TerminalNode identifier(token->codeFragment, TerminalNode::Type::Identifier);
            deleteToken();
            auto repeating = parseRepeatingDeclaratorList();
            return std::make_unique<DeclaratorList::Repeating>(colon, identifier, std::move(repeating));
        } else {
            token->codeFragment.print("error: identifier missing");
            return {nullptr};
        }
    } else {
        return {nullptr};
    }
}

/// parse init-declarator-list
InitDeclaratorList SyntaxAnalyser::parseInitDeclaratorList() {
    auto initDeclarator = parseInitDeclarator();
    auto repeating = parseRepeatingInitDeclaratorList();
    return {initDeclarator, std::move(repeating)};
}

/// parse repeating part of InitDeclaratorList
std::unique_ptr<InitDeclaratorList::Repeating> SyntaxAnalyser::parseRepeatingInitDeclaratorList() {
    getToken();
    Separator* separator = getSeparator(token);
    // check is token is comma
    if(separator && separator->separators == Separator::Separators::comma) {
        TerminalNode colon(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto initDeclarator = parseInitDeclarator();
        auto repeating = parseRepeatingInitDeclaratorList();
        return std::make_unique<InitDeclaratorList::Repeating>(colon, initDeclarator, std::move(repeating));
    } else {
        return {nullptr};
    }
}

/// parse init-declarator
InitDeclarator SyntaxAnalyser::parseInitDeclarator() {
    getToken();
    // check if token is identifier
    if(token->type == Token::Type::Identifier) {
        TerminalNode identifier(token->codeFragment, TerminalNode::Type::Identifier);
        deleteToken();
        getToken();
        Operator* tokenOperator = getOperator(token);
        // check if token is =
        if(tokenOperator && tokenOperator->operators == Operator::Operators::EqualsInit) {
            TerminalNode equals(token->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            getToken();
            Literal* tokenLiteral = getLiteral(token);
            // check if token is valid literal
            if(tokenLiteral) {
                TerminalNode literal(token->codeFragment, TerminalNode::Type::Literal);
                deleteToken();
                return {identifier, equals, literal};
            } else {
                token->codeFragment.print("error: expected valid literal");
            }
        } else {
            token->codeFragment.print("error: expected \"=\"");
        }
    } else {
        token->codeFragment.print("error: expected valid identifier");
    }
    throw "failure!";
}

/// parse compound-statement
CompoundStatement SyntaxAnalyser::parseCompoundStatement() {
    getToken();
    Keyword* keyword1 = getKeyword(token);
    // check if token is BEGIN
    if(keyword1 && keyword1->keywords == Keyword::Keywords::BEGIN) {
        TerminalNode BEGIN(keyword1->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto statementList = parseStatementList();
        getToken();
        Keyword* keyword2 = getKeyword(token);
        // check if token is END
        if(keyword2 && keyword2->keywords == Keyword::Keywords::END) {
            TerminalNode END(keyword2->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            return {BEGIN, std::move(statementList), END};
        } else {
            token->codeFragment.print("error: expected \"END\"");
        }
    } else {
        token->codeFragment.print("error: expected \"BEGIN\"");
    }
    throw "failure!";
}

/// parse statement-list
StatementList SyntaxAnalyser::parseStatementList() {
    auto statement = parseStatement();
    auto repeating = parseRepeatingStatementList();
    return {std::move(statement), std::move(repeating)};
}

/// parse repeating part of StatementList
std::unique_ptr<StatementList::Repeating> SyntaxAnalyser::parseRepeatingStatementList() {
    getToken();
    Separator* separator = getSeparator(token);
    // check if token is semicolon
    if(separator && separator->separators == Separator::Separators::semicolon) {
        TerminalNode semiColon(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto statement = parseStatement();
        auto repeating = parseRepeatingStatementList();
        return std::make_unique<StatementList::Repeating>(semiColon, std::move(statement), std::move(repeating));
    } else {
        return {nullptr};
    }
}

/// parse statement
Statement SyntaxAnalyser::parseStatement() {
    getToken();
    Keyword* keyword = getKeyword(token);
    // check if token is RETURN, if yes then its a return statement, else it's a normal statement
    if(keyword && keyword->keywords == Keyword::Keywords::RETURN) {
        TerminalNode RETURN(keyword->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto additiveExpression = parseAdditiveExpression().value();
        auto returnAdditive = std::make_optional<Statement::ReturnAdditive>(RETURN, additiveExpression);
        return {Statement::WhichAlternative::ReturnAdditive, std::nullopt, std::move(returnAdditive)};
    } else {
        auto assignmentExpression = std::make_optional<AssignmentExpression>(parseAssignmentExpression());
        return {Statement::WhichAlternative::AssignmentExpression, std::move(assignmentExpression), std::nullopt};
    }
}

/// parse assignment-expression
AssignmentExpression SyntaxAnalyser::parseAssignmentExpression() {
    getToken();
    // check if token is identifier
    if(token->type == Token::Type::Identifier) {
        TerminalNode identifier(token->codeFragment, TerminalNode::Type::Identifier);
        deleteToken();
        getToken();
        Operator* operatorToken = getOperator(token);
        // check if token is :=
        if(operatorToken && operatorToken->operators == Operator::Operators::EqualsAssignment) {
            TerminalNode equalsAssignment(operatorToken->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            auto additiveExpression = parseAdditiveExpression().value();
            return {identifier, equalsAssignment, std::move(additiveExpression)};
        }
    } else {
        token->codeFragment.print("error: expected identifier");
    }
    throw "failure!";
}

/// parse additive-expression
std::optional<AdditiveExpression> SyntaxAnalyser::parseAdditiveExpression() {
    auto multiplicativeExpression = parseMultiplicativeExpression();
    if(!multiplicativeExpression.has_value()) {
        return std::nullopt;
    }
    getToken();
    auto operator1 = getOperator(token);
    // check if token is +
    if(operator1 && operator1->operators == Operator::Operators::Plus) {
        TerminalNode plus(operator1->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto additiveExpression = parseAdditiveExpression();
        if(!additiveExpression.has_value()) {
            token->codeFragment.print("error: additiveExpression missing");
            throw "Failure!";
        }
        auto additiveExpression_ptr = std::make_unique<AdditiveExpression>(std::move(additiveExpression.value()));
        return std::make_optional<AdditiveExpression>(
            AdditiveExpression::WhichOperator::Plus,
            std::move(multiplicativeExpression.value()),
            std::move(plus),
            std::move(additiveExpression_ptr));
    // check if token is -
    } else if (operator1 && operator1->operators == Operator::Operators::Minus) {
        TerminalNode minus(operator1->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto additiveExpression = parseAdditiveExpression();
        if(!additiveExpression.has_value()) {
            token->codeFragment.print("error: additiveExpression missing");
            throw "Failure!";
        }
        auto additiveExpression_ptr = std::make_unique<AdditiveExpression>(std::move(additiveExpression.value()));
        return std::make_optional<AdditiveExpression>(
            AdditiveExpression::WhichOperator::Minus,
            std::move(multiplicativeExpression.value()),
            std::move(minus),
            std::move(additiveExpression_ptr));
    // no operator
    } else {
        auto additiveExpression = parseAdditiveExpression();
        if(!additiveExpression.has_value()) {
            return std::make_optional<AdditiveExpression>(
                AdditiveExpression::WhichOperator::None,
                std::move(multiplicativeExpression.value()),
                std::nullopt,
                nullptr);
        }
        auto additiveExpression_ptr = std::make_unique<AdditiveExpression>(std::move(additiveExpression.value()));
        return std::make_optional<AdditiveExpression>(
            AdditiveExpression::WhichOperator::None,
            std::move(multiplicativeExpression.value()),
            std::nullopt,
            std::move(additiveExpression_ptr));
    }
}

/// parse multiplicative-expression
std::optional<MultiplicativeExpression> SyntaxAnalyser::parseMultiplicativeExpression() {
    auto unaryExpression = parseUnaryExpression();
    if(!unaryExpression.has_value()) {
        return std::nullopt;
    }
    getToken();
    auto operator1 = getOperator(token);
    // check if token is *
    if(operator1 && operator1->operators == Operator::Operators::Multiplication) {
        TerminalNode multiply(operator1->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto multiplicativeExpression = parseMultiplicativeExpression();
        if(!multiplicativeExpression.has_value()) {
            token->codeFragment.print("error: multiplicativeExpression missing");
            throw "failure!";
        }
        auto multiplicativeExpression_ptr = std::make_unique<MultiplicativeExpression>(std::move(multiplicativeExpression.value()));
        return std::make_optional<MultiplicativeExpression>(
            MultiplicativeExpression::WhichOperator::Multiplication,
            std::move(unaryExpression.value()),
            std::move(multiply),
            std::move(multiplicativeExpression_ptr));
    // check if token is /
    } else if(operator1 && operator1->operators == Operator::Operators::Division) {
        TerminalNode divide(operator1->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto multiplicativeExpression = parseMultiplicativeExpression();
        if(!multiplicativeExpression.has_value()) {
            token->codeFragment.print("error: multiplicativeExpression missing");
            throw "failure!";
        }
        auto multiplicativeExpression_ptr = std::make_unique<MultiplicativeExpression>(std::move(multiplicativeExpression.value()));
        return std::make_optional<MultiplicativeExpression>(
            MultiplicativeExpression::WhichOperator::Divide,
            std::move(unaryExpression.value()),
            std::move(divide),
            std::move(multiplicativeExpression_ptr));
    // no operator
    } else {
        auto multiplicativeExpression = parseMultiplicativeExpression();
        if(!multiplicativeExpression.has_value()) {
            return std::make_optional<MultiplicativeExpression>(
                MultiplicativeExpression::WhichOperator::None,
                std::move(unaryExpression.value()),
                std::nullopt,
                nullptr);
        }
        auto multiplicativeExpression_ptr = std::make_unique<MultiplicativeExpression>(std::move(multiplicativeExpression.value()));
        return std::make_optional<MultiplicativeExpression>(
            MultiplicativeExpression::WhichOperator::None,
            std::move(unaryExpression.value()),
            std::nullopt,
            std::move(multiplicativeExpression_ptr));
    }
}

/// parse unary-expression
std::optional<UnaryExpression> SyntaxAnalyser::parseUnaryExpression() {
    getToken();
    if(token->codeFragment.getString() == "+") {
        std::optional<TerminalNode> plus = std::make_optional<TerminalNode>(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto primaryExpression = parsePrimaryExpression();
        if(!primaryExpression.has_value()) {
            plus.value().codeFragment.print("error: primaryExpression missing");
            throw "failure!";
        }
        return std::make_optional<UnaryExpression>(UnaryExpression::WhichOperator::Plus, plus, std::move(primaryExpression.value()));
    } else if(token->codeFragment.getString() == "-") {
        std::optional<TerminalNode> minus = std::make_optional<TerminalNode>(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        auto primaryExpression = parsePrimaryExpression();
        if (!primaryExpression.has_value()) {
            minus.value().codeFragment.print("error: primaryExpression missing");
            throw "failure!";
        }
        return std::make_optional<UnaryExpression>(UnaryExpression::WhichOperator::Minus, minus, std::move(primaryExpression.value()));
    // no operator
    } else {
        auto primaryExpression = parsePrimaryExpression();
        if(!primaryExpression.has_value()) {
            return std::nullopt;
        }
        return std::make_optional<UnaryExpression>(UnaryExpression::WhichOperator::NoOperator, std::nullopt, std::move(primaryExpression.value()));
    }


}

/// parse primary-expression
std::optional<PrimaryExpression> SyntaxAnalyser::parsePrimaryExpression() {
    getToken();
    if(token->type == Token::Type::Identifier) {
        // create TerminalNode
        TerminalNode terminalNode(token->codeFragment, TerminalNode::Type::Identifier);
        // insert TerminalNode + WhichAlternative into PrimaryExpression
        deleteToken();
        return std::make_optional<PrimaryExpression>(PrimaryExpression::WhichAlternative::Identifier, terminalNode);

    } else if(token->type == Token::Type::Literal) {
        // create TerminalNode
        TerminalNode terminalNode(token->codeFragment, TerminalNode::Type::Literal);
        // insert TerminalNode + WhichAlternative into PrimaryExpression
        deleteToken();
        return std::make_optional<PrimaryExpression>(PrimaryExpression::WhichAlternative::Literal, terminalNode);

    } else if(token->codeFragment.getString() == "(") {
        // create TerminalNodes
        TerminalNode terminalNodeLeft(token->codeFragment, TerminalNode::Type::Generic);
        deleteToken();
        AdditiveExpression additiveExpression = parseAdditiveExpression().value();
        std::unique_ptr<AdditiveExpression> ptr = std::make_unique<AdditiveExpression>(std::move(additiveExpression));
        getToken();
        if(token->codeFragment.getString() == ")") {
            TerminalNode terminalNodeRight(token->codeFragment, TerminalNode::Type::Generic);
            deleteToken();
            // create AdditiveExpressionBrackets
            PrimaryExpression::AdditiveExpressionBrackets additiveExpressionBrackets(terminalNodeLeft, std::move(ptr), terminalNodeRight);
            // insert AdditiveExpressionBrackets + WhichAlternative into PrimaryExpression
            return std::make_optional<PrimaryExpression>(PrimaryExpression::WhichAlternative::AdditiveExpression, std::move(additiveExpressionBrackets));

        } else {
            token->codeFragment.print("error: missing \")\"");
        }
    }
    return std::nullopt;

}

//--------------------------------------------------------------------
// Begin: visit Methods for printing SyntaxTree

void Print::visit(const FunctionDefinition& node) {
    maxId = 0;
    std::cout << "digraph {" << std::endl;
    std::cout << maxId++ << " [label = \"function\"]" << std::endl;
    std::cout << maxId++ << " [label = \"parameters\"]" << std::endl;
    std::cout << maxId - 2 << " -> " << maxId - 1 << std::endl;
    std::cout << maxId++ << " [label = \"variables\"]" << std::endl;
    std::cout << maxId - 3 << " -> " << maxId - 1 << std::endl;
    std::cout << maxId++ << " [label = \"constants\"]" << std::endl;
    std::cout << maxId - 4 << " -> " << maxId - 1 << std::endl;
    std::cout << maxId++ << " [label = \"compoundStatement\"]" << std::endl;
    std::cout << maxId - 5 << " -> " << maxId - 1 << std::endl;
    unsigned parameters = maxId - 4;
    unsigned variables = maxId - 3;
    unsigned constants = maxId - 2;
    unsigned compoundStatement = maxId - 1;
    if(node.parameters.has_value()) {
        visit(node.parameters.value(), parameters);
    }
    if(node.variables.has_value()) {
        visit(node.variables.value(), variables);
    }
    if(node.constants.has_value()) {
        visit(node.constants.value(), constants);
    }
    visit(node.compoundStatement, compoundStatement);
    std::cout << "}" << std::endl;
}

void Print::visit(const ParameterDeclarations& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"declaratorList\"]" << std::endl;
    std::cout << thisId << " -> \"PARAM\"" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.declaratorList, maxId - 1);
}

void Print::visit(const VariableDeclarations& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"declaratorList\"]" << std::endl;
    std::cout << thisId << " -> \"VAR\"" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.declaratorList, maxId - 1);
}

void Print::visit(const ConstantDeclarations& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"initDeclaratorList\"]" << std::endl;
    std::cout << thisId << " -> \"CONST\"" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.initDeclaratorList, maxId - 1);
}

void Print::visit(const DeclaratorList& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"" << node.identifier.codeFragment.getString() << "\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.repeating.get(), thisId);
}

void Print::visit(const DeclaratorList::Repeating* node, unsigned thisId) {
    if(node != nullptr) {
        std::cout << maxId++ << " [label = \"" << node->identifier.codeFragment.getString() << "\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(node->next.get(), thisId);
    }
}

void Print::visit(const InitDeclaratorList& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"initDeclarator\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.initDeclarator, maxId - 1);
    visit(node.repeating.get(), thisId);
}

void Print::visit(const InitDeclaratorList::Repeating* node, unsigned thisId) {
    if(node != nullptr) {
        std::cout << maxId++ << " [label = \"initDeclarator\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(node->next.get(), thisId);
    }
}

void Print::visit(const InitDeclarator& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"" << node.identifier.codeFragment.getString() << "\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    std::cout << maxId++ << " [label = \"" << node.literal.codeFragment.getString() << "\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
}

void Print::visit(const CompoundStatement& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"statementList\"]" << std::endl;
    std::cout << thisId << " -> " << "\"BEGIN\"" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    std::cout << thisId << " -> " << "\"END\"" << std::endl;
    visit(node.statementList, maxId - 1);
}

void Print::visit(const StatementList& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"statement0\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.statement, maxId - 1);
    visit(node.repeating.get(), thisId, 1);
}

void Print::visit(const StatementList::Repeating* node, unsigned thisId, unsigned statementId) {
    if(node != nullptr) {
        std::cout << maxId++ << " [label = \"statement" << statementId << "\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(node->statement, maxId - 1);
        visit(node->next.get(), thisId, statementId + 1);
    }
}

void Print::visit(const Statement& node, unsigned thisId) {
    if(node.whichAlternative == Statement::WhichAlternative::AssignmentExpression) {
        std::cout << maxId++ << " [label = \"assignmentExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(node.assignmentExpression.value(), maxId - 1);
    } else {
        // ReturnAdditive
        std::cout << maxId++ << " [label = \"additiveExpression\"]" << std::endl;
        std::cout << thisId << " -> \"RETURN\"" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(node.returnAdditive.value().additiveExpression, maxId - 1);
    }
}

void Print::visit(const AssignmentExpression& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"" << node.identifier.codeFragment.getString() << "\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    std::cout << maxId++ << " [label = \"additiveExpression\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.additiveExpression, maxId - 1);
}

void Print::visit(const AdditiveExpression& node, unsigned thisId) {
    std::cout << maxId++ << " [label = \"multiplicativeExpression\"]" << std::endl;
    std::cout << thisId << " -> " << maxId - 1 << std::endl;
    visit(node.multiplicativeExpression, maxId - 1);
    if(node.whichOperator == AdditiveExpression::WhichOperator::Plus) {
        std::cout << maxId++ << " [label = \"+\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"additiveExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(*node.additiveExpression, maxId - 1);
    } else if(node.whichOperator == AdditiveExpression::WhichOperator::Minus) {
        std::cout << maxId++ << " [label = \"-\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"additiveExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(*node.additiveExpression, maxId - 1);
    }
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

void Print::visit(const UnaryExpression& node, unsigned thisId) {
    if(node.whichOperator == UnaryExpression::WhichOperator::Plus) {
        std::cout << maxId++ << " [label = \"+\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"primaryExpression\"]" << std::endl;
        std::cout << maxId - 2 << " -> " << maxId - 1 << std::endl;
    } else if(node.whichOperator == UnaryExpression::WhichOperator::Minus) {
        std::cout << maxId++ << " [label = \"-\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"primaryExpression\"]" << std::endl;
        std::cout << maxId - 2 << " -> " << maxId - 1 << std::endl;
    } else {
        // no operator
        std::cout << maxId++ << " [label = \"primaryExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
    }
    visit(node.primaryExpression, maxId - 1);
}

void Print::visit(const PrimaryExpression& node, unsigned thisId) {
    if(node.whichAlternative == PrimaryExpression::WhichAlternative::Literal
        || node.whichAlternative == PrimaryExpression::WhichAlternative::Identifier) {
        std::cout << maxId++ << " [label = \"" << node.terminalNode.value().codeFragment.getString() << "\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
    } else {
        // additiveExpression
        std::cout << maxId++ << " [label = \"(\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \"additiveExpression\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        std::cout << maxId++ << " [label = \")\"]" << std::endl;
        std::cout << thisId << " -> " << maxId - 1 << std::endl;
        visit(*node.additiveExpressionBrackets.value().additiveExpression, maxId - 2);
    }
}

// End: visit Methods for printing SyntaxTree
//--------------------------------------------------------------------


} // namespace syntax_analysis
//--------------------------------------------------------------
