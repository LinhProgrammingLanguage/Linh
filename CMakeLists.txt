cmake_minimum_required(VERSION 3.15)

project(TinhLinhLangProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Định nghĩa thư viện Lexer ---
add_library(LinhLexerLib STATIC
    LinhC/Parsing/Lexer/Lexer.cpp
)
target_include_directories(LinhLexerLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Định nghĩa thư viện AST (cho các node và printer) ---
# ASTNode.hpp là header-only, ASTPrinter.cpp cần được biên dịch
add_library(LinhASTLib STATIC
    LinhC/Parsing/AST/ASTPrinter.cpp
)
target_include_directories(LinhASTLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Để include ASTNode.hpp và Lexer.hpp (cho Token)
)

# --- Định nghĩa thư viện Parser ---
add_library(LinhParserLib STATIC
    LinhC/Parsing/Parser/ParserBase.cpp
    LinhC/Parsing/Parser/ParseDeclaration.cpp
    LinhC/Parsing/Parser/ParseStatement.cpp
    LinhC/Parsing/Parser/ParseExpression.cpp
    LinhC/Parsing/Parser/ParseType.cpp
)
target_include_directories(LinhParserLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR} # Để include Parser.hpp, ASTNode.hpp, Lexer.hpp
)

# --- Định nghĩa thư viện SemanticAnalyzer ---
add_library(LinhSemanticLib STATIC
    LinhC/Parsing/Semantic/SemanticAnalyzer.cpp
)
target_include_directories(LinhSemanticLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- LiVM VM ---
add_library(LiVMLib STATIC
    LiVM/LiVM.cpp
    LiVM/type.cpp
    LiVM/Loop.cpp
    LiVM/iostream/iostream.cpp # Thêm dòng này để build iostream
    LiVM/Math/Math.cpp
    LiVM/Value/Value.cpp
)
target_include_directories(LiVMLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- LiPM Package Manager ---
add_library(LiPMLib STATIC
    LiPM/LiPM.cpp
)
target_include_directories(LiPMLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- Định nghĩa file thực thi chính ---
add_executable(LinhApp
    Main.cpp
    LinhC/Parsing/Parser/ParseType.cpp
    LinhC/Parsing/Parser/ParseStatement.cpp
    LinhC/Parsing/Parser/ParseDeclaration.cpp
    LinhC/Parsing/Parser/ParserBase.cpp
    LinhC/Parsing/Semantic/SemanticAnalyzer.cpp
    LinhC/Parsing/AST/ASTPrinter.cpp
    LinhC/Bytecode/BytecodeEmitter.cpp
    REPL.cpp
)
target_link_libraries(LinhApp PRIVATE
    LinhLexerLib
    LinhASTLib
    LinhParserLib
    LinhSemanticLib
    LiVMLib
    LiPMLib
)

if(WIN32)
    set(RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
    set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Linh.ico")

    if(EXISTS "${RESOURCE_FILE}" AND EXISTS "${ICON_FILE}")
        message(STATUS "Found Windows resource file: ${RESOURCE_FILE} and icon: ${ICON_FILE}")
        target_sources(LinhApp PRIVATE "${RESOURCE_FILE}")
    else()
        message(WARNING "Windows resource file (appicon.rc) or icon (Linh.ico) not found. Skipping icon embedding.")
    endif()
endif()

# Tối ưu hóa cho Release build
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -DNDEBUG")
endif()

add_custom_command(TARGET LinhApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Built LinhApp: $<TARGET_FILE:LinhApp>"
)