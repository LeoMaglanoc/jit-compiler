#include "6_lib_interface.hpp"
#include "5_execution.hpp"
#include "3_syntax_analysis.hpp"
#include <iostream>
#include <vector>
#include <sstream>
//---------------------------------------------------------------------------
using namespace interface;
//---------------------------------------------------------------------------

int main() {


    // Test for Milestone 6
    std::stringstream ss;
    std::vector<std::string> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;\n");
    sourceCode.emplace_back("VAR volume;\n");
    sourceCode.emplace_back("CONST density = 2400;\n");
    sourceCode.emplace_back("BEGIN\n");
    sourceCode.emplace_back("\tvolume := width * height * depth;\n");
    sourceCode.emplace_back("\tRETURN density * volume\n");
    sourceCode.emplace_back("END.");
    for(const std::string& line : sourceCode) {
        ss << line;
    }
    std::string s = ss.str();
    Pljit jit;
    Function function = jit.registerFunctionAlternative(s);
    std::cout << function({1,2,3}) << std::endl;

    /*
    // Test for Milestone 3
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;");
    sourceCode.emplace_back("VAR volume;");
    sourceCode.emplace_back("CONST density = 2400;");
    //sourceCode.emplace_back("");
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("\tvolume := width * height * depth;");
    sourceCode.emplace_back("\tRETURN density * volume");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    syntax_analysis::SyntaxAnalyser syntaxAnalyser(codeManager);
    syntax_analysis::FunctionDefinition program = syntaxAnalyser.parseFunctionDefinition();
    syntax_analysis::Print print;
    print.visit(program);
    */

    /*
    // Test for Milestone 3
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("\tRETURN 5 * 10 * 6");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    syntax_analysis::SyntaxAnalyser syntaxAnalyser(codeManager);
    syntax_analysis::FunctionDefinition program = syntaxAnalyser.parseFunctionDefinition();
    syntax_analysis::Print print;
    print.visit(program);
    */

    /*
    // Tests for Syntax Analysis
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;");
    sourceCode.emplace_back("VAR volume;");
    sourceCode.emplace_back("CONST density = 2400;");
    //sourceCode.emplace_back("");
    sourceCode.emplace_back("BEGIN");
    sourceCode.emplace_back("\tvolume := width * height * depth;");
    sourceCode.emplace_back("\tRETURN density * volume");
    sourceCode.emplace_back("END.");
    CodeManager codeManager(sourceCode);
    SyntaxAnalyser syntaxAnalyser(codeManager);
    FunctionDefinition program = syntaxAnalyser.parseFunctionDefinition();
    */

    /*
    /// Tests for Code Management
    std::vector<std::string_view> vec;
    vec.emplace_back("Hello World!");
    code_management::CodeManager test = code_management::CodeManager(vec);
    code_management::CodeFragment fragment = test.createCodeFragment(0, 0, 4);
    fragment.print("error");
    */

    /*
    /// Tests for Lexer
    std::vector<std::string_view> sourceCode;
    sourceCode.emplace_back("PARAM width, height, depth;");
    sourceCode.emplace_back("END");
    code_management::CodeManager codeManager(sourceCode);
    lexical_analysis::Lexer lexer(codeManager);
    for(unsigned i = 0; i < 7; ++i) {
        auto token = lexer.next().value();
        token.codeFragment.print("test:");
    }

    std::vector<std::string_view> source;
    source.emplace_back("Hello");
    CodeManager codeManager = CodeManager(source);
    CodeFragment codeFragment = codeManager.createCodeFragment(0, 1);
    Keyword token = Keyword(codeFragment, Keyword::Keywords::PARAM);
    Token* keyword1 = &token;
    const Keyword* keyword2 = getKeywordDynamic(keyword1);
    std::cout << keyword2->codeFragment.getString() << std::endl;
    */

}
//---------------------------------------------------------------------------
