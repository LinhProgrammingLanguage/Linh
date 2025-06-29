#pragma once
#include <string>

namespace Linh
{
    enum class ErrorStage
    {
        Lexer,
        Parser,
        Semantic,
        Bytecode,
        Runtime
    };

    struct Error
    {
        ErrorStage stage;
        std::string type; // e.g. "SyntaxError", "SemanticError", etc.
        std::string message;
        int line = 0;
        int column = 0;
    };
}
