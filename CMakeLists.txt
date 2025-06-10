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
)
target_link_libraries(LinhApp PRIVATE
    LinhLexerLib
    LinhASTLib
    LinhParserLib
    LinhSemanticLib
)

include(FetchContent)
FetchContent_Declare(
  CPM
  GIT_REPOSITORY https://github.com/cpm-cmake/CPM.cmake.git
  GIT_TAG origin/master
)
FetchContent_MakeAvailable(CPM)

# --- CPM: Boost.Multiprecision (chỉ riêng multiprecision) ---
CPMAddPackage(
  NAME boost_multiprecision
  GITHUB_REPOSITORY boostorg/multiprecision
  GIT_TAG boost-1.85.0
)
if(boost_multiprecision_ADDED)
    message(STATUS "Boost.Multiprecision added via CPM")
endif()

# --- CPM: utf8-cpp ---
CPMAddPackage(
  NAME utf8cpp
  GITHUB_REPOSITORY nemtrif/utfcpp
  GIT_TAG v3.2.1
  DOWNLOAD_ONLY ON
)
if(utf8cpp_ADDED)
    message(STATUS "utf8-cpp added via CPM (download only)")
endif()

target_include_directories(LinhApp PRIVATE ${utf8cpp_SOURCE_DIR}/source)

target_link_libraries(LinhApp PRIVATE
    LinhLexerLib
    LinhASTLib
    LinhParserLib
    LinhSemanticLib
    boost_multiprecision
    # Không cần utf8cpp vì chỉ là header-only, đã include ở trên
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

add_custom_command(TARGET LinhApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Built LinhApp: $<TARGET_FILE:LinhApp>"
)