#ifndef H_1_code_management
#define H_1_code_management
#include <string_view>
#include <span>
//-------------------------------------------------------------------------------------------------
namespace code_management {

/// struct for code marker
struct CodeMarker {
    /// which line
    size_t line;
    /// which char in the line
    size_t charBegin;
    /// if its just one char, its the same as charBegin, if its a range then its a range (inclusive beginning and end)
    size_t charEnd;
    CodeMarker() : line(-1), charBegin(-1), charEnd(-1) {}
    CodeMarker(size_t line, size_t charBegin) : line(line), charBegin(charBegin), charEnd(charBegin) {}
    CodeMarker(size_t line, size_t charBegin, size_t charEnd) : line(line), charBegin(charBegin), charEnd(charEnd) {}
    /// comparison operator
    bool operator==(const CodeMarker& other) const;
};

class CodeManager;

/// saves line range
class CodeFragment {
    /// marks the code fragment in the console
    std::string consoleMarking() const;
    public:
    /// member variables to save positions
    CodeMarker codeMarker;
    /// saves the line of code
    std::string_view lineOfCode;
    /// default constructor
    CodeFragment() = default;
    /// constructor for single position
    CodeFragment(size_t line, size_t charBegin, std::string_view lineOfCode)
        : codeMarker(line, charBegin), lineOfCode(lineOfCode) {}
    /// constructor for range
    CodeFragment(size_t line, size_t charBegin, size_t charEnd, std::string_view lineOfCode)
        : codeMarker(line, charBegin, charEnd), lineOfCode(lineOfCode) {}
    /// copy assignment
    CodeFragment& operator=(const CodeFragment& other);
    /// copy constructor
    CodeFragment(const CodeFragment& other) = default;
    //CodeFragment(const CodeFragment& other) : codeMarker(other.codeMarker), lineOfCode(other.lineOfCode) {}
    /// prints context of message with corresponding code fragment
    void print(std::string_view message) const;
    /// get code string
    std::string_view getString() const;
    /// gives back CodeMarker
    CodeMarker resolve() const;
};

/// saves ref to source code + creation of CodeFragments
class CodeManager {
    friend CodeFragment;
    public:
    /// ref to source code
    std::span<std::string_view> sourceCode;
    /// constructor
    CodeManager(std::span<std::string_view> sourceCode) : sourceCode(sourceCode) {}
    /// gives back CodeFragment
    CodeFragment createCodeFragment(size_t line, size_t charBegin, size_t charEnd);
    CodeFragment createCodeFragment(size_t line, size_t charBegin);
};


} // namespace code_management
//-------------------------------------------------------------------------------------------------
#endif