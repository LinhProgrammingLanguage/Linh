# Cập nhật SmallString cho các file còn lại

## Các thay đổi cần thực hiện:

### 1. Thay thế std::holds_alternative<std::string> thành std::holds_alternative<SmallString>
### 2. Thay thế std::get<std::string> thành std::get<SmallString>.to_string()
### 3. Thay thế std::string trong Value variant thành SmallString

## Files cần cập nhật:
- LiVM/Math/Math.cpp
- LiVM/Loop.cpp  
- LiVM/LiVM.cpp (còn nhiều chỗ)
- LinhC/Parsing/Semantic/SemanticAnalyzer.cpp
- LinhC/Bytecode/BytecodeEmitter.cpp
- LinhC/Parsing/Parser/ParseExpression.cpp
- LinhC/Parsing/AST/ASTPrinter.cpp

## Pattern thay thế:
- `std::holds_alternative<std::string>(val)` → `std::holds_alternative<SmallString>(val)`
- `std::get<std::string>(val)` → `std::get<SmallString>(val).to_string()`
- `std::string` trong Value variant → `SmallString` 