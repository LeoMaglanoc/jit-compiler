#ifndef H_6_library_interface
#define H_6_library_interface
#include <string_view>
#include <span>
#include "1_code_management.hpp"
#include "5_execution.hpp"
//-------------------------------------------------------------------------------------------------
namespace interface {

/*
 * Pljit class creates Function-Objects, and Function does the actual compilation & execution
 */

/// splits string by newlines
std::vector<std::string_view> parseLines(std::string_view code);

/// saves function
class Function {
    private:
    std::vector<std::string_view> sourceCode;
    code_management::CodeManager codeManager;
    execution::Evaluation evaluation;
    public:
    /// constructor
    Function(std::vector<std::string_view> sourceCode) : sourceCode(sourceCode), codeManager(sourceCode), evaluation(codeManager) {}
    int64_t operator()(std::initializer_list<int64_t> list);
    int64_t operator()() {return operator()({});};
};

/// lightweight handle
class Handle {
    private:
    Function* function;
    public:
    /// constructor
    Handle(Function* function) : function(function) {}
    int64_t operator()(std::initializer_list<int64_t> list) {return (*function)(list);}
    /// copy constructor
    Handle(const Handle&) = default;
    /// copy assignment
    Handle& operator=(const Handle&) = default;

};

/// creates new functions
class Pljit {
    private:
    std::vector<Function> list;
    public:
    /// default constructor
    Pljit() = default;
    /// gives back function
    Handle registerFunction(std::string_view code);
    Function registerFunctionAlternative(std::string_view code);
};


} // namespace interface
//-------------------------------------------------------------------------------------------------
#endif