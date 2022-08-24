#include "6_lib_interface.hpp"
//-------------------------------------------------------------------------------------------------
namespace interface {

int64_t Function::operator()(std::initializer_list<int64_t> list) {
    return evaluation.evaluateFunction(list);
}


/// TODO: wont work because function is stored as local variable and not in member variable
Handle Pljit::registerFunction(std::string_view code) {
    std::vector<std::string_view> sourceCode = parseLines(code);
    Function function(sourceCode);
    return {&function};
}

Function Pljit::registerFunctionAlternative(std::string_view code) {
    std::vector<std::string_view> sourceCode = parseLines(code);
    return {sourceCode};
}

/// parse code into vector of string_view where each element is one line
std::vector<std::string_view> parseLines(std::string_view code) {
    std::vector<std::string_view> vector;
    std::string::size_type begin = 0;
    std::string::size_type length = 0;
    for (std::string::size_type i = 0; i < code.size(); i++) {
        if(code[i] == '\n') {
            vector.push_back(code.substr(begin, length));
            length = 0;
            begin = i + 1;
        } else if(i != code.size() - 1){
            length++;
        } else {
            vector.push_back(code.substr(begin, length + 1));
        }
    }
    return vector;
}


} // namespace interface
//-------------------------------------------------------------------------------------------------