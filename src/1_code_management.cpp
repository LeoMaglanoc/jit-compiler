#include "1_code_management.hpp"
#include <iostream>
#include <sstream>
#include <string>
//------------------------------------------------------------------
namespace code_management {

/// helper function to print error messages
void CodeFragment::print(std::string_view message) const {
    std::cout << codeMarker.line << ":" << codeMarker.charBegin << ": " << message << "\n"
              << "\t" << lineOfCode << "\n"
              << "\t" << consoleMarking() << std::endl;
}

/// marks the code fragment in the console
std::string CodeFragment::consoleMarking() const {
    std::stringstream ss;
    for(unsigned i = 0; i < codeMarker.charBegin; i++) {
        ss << " ";
    }
    ss << "^";
    unsigned length = codeMarker.charEnd - codeMarker.charBegin;
    for (unsigned i = 1; i <= length; i++) {
        ss << "~";
    }
    return ss.str();
}

/// comparison operator
bool CodeMarker::operator==(const CodeMarker& other) const {
    return line == other.line && charBegin == other.charBegin && charEnd == other.charEnd;
}

/// get code string
std::string_view CodeFragment::getString() const {
    return lineOfCode.substr(codeMarker.charBegin, codeMarker.charEnd - codeMarker.charBegin + 1);
}

/// gives back CodeMarker
CodeMarker CodeFragment::resolve() const {
    return codeMarker;
}


/// gives back CodeFragment
CodeFragment CodeManager::createCodeFragment(size_t line, size_t charBegin, size_t charEnd) {
    return {line, charBegin, charEnd, sourceCode[line]};
}

/// gives back CodeFragment
CodeFragment CodeManager::createCodeFragment(size_t line, size_t charBegin) {
    return {line, charBegin, charBegin, sourceCode[line]};
}

/// copy assignment
CodeFragment& CodeFragment::operator=(const CodeFragment& other) {
    if(this != &other) {
        codeMarker = other.codeMarker;
        lineOfCode = other.lineOfCode;
    }
    return *this;
}

} // namespace code_management
//-------------------------------------------------------------------------------------------------
