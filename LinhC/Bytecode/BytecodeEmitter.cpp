#include "BytecodeEmitter.hpp"

namespace Linh
{
    BytecodeEmitter::BytecodeEmitter() {}

    void BytecodeEmitter::emit(const AST::StmtList &stmts)
    {
        for (const auto &stmt : stmts)
        {
            if (stmt)
                stmt->accept(this);
        }
        emit_instr(OpCode::HALT);
    }

    void BytecodeEmitter::emit_instr(OpCode op, BytecodeValue val)
    {
        chunk.emplace_back(op, val);
    }

    int BytecodeEmitter::get_var_index(const std::string &name)
    {
        auto it = var_table.find(name);
        if (it != var_table.end())
            return it->second;
        int idx = next_var_index++;
        var_table[name] = idx;
        return idx;
    }

    // --- ExprVisitor ---
    std::any BytecodeEmitter::visitLiteralExpr(AST::LiteralExpr *expr)
    {
        if (std::holds_alternative<int64_t>(expr->value))
            emit_instr(OpCode::PUSH_INT, std::get<int64_t>(expr->value));
        else if (std::holds_alternative<double>(expr->value))
            emit_instr(OpCode::PUSH_FLOAT, std::get<double>(expr->value));
        else if (std::holds_alternative<std::string>(expr->value))
            emit_instr(OpCode::PUSH_STR, std::get<std::string>(expr->value));
        else if (std::holds_alternative<bool>(expr->value))
            emit_instr(OpCode::PUSH_BOOL, std::get<bool>(expr->value));
        return {};
    }

    std::any BytecodeEmitter::visitIdentifierExpr(AST::IdentifierExpr *expr)
    {
        emit_instr(OpCode::LOAD_VAR, get_var_index(expr->name.lexeme));
        return {};
    }

    void BytecodeEmitter::visitPrintStmt(AST::PrintStmt *stmt)
    {
        if (stmt->expression)
            stmt->expression->accept(this);
        emit_instr(OpCode::PRINT);
    }

    void BytecodeEmitter::visitExpressionStmt(AST::ExpressionStmt *stmt)
    {
        if (stmt->expression)
            stmt->expression->accept(this);
        emit_instr(OpCode::POP);
    }

    void BytecodeEmitter::visitVarDeclStmt(AST::VarDeclStmt *stmt)
    {
        int idx = get_var_index(stmt->name.lexeme);
        if (stmt->initializer)
            stmt->initializer->accept(this);
        else
            emit_instr(OpCode::PUSH_INT, 0); // default 0
        emit_instr(OpCode::STORE_VAR, idx);
    }

    // ...Các visit* khác để trống hoặc TODO...
    std::any BytecodeEmitter::visitBinaryExpr(AST::BinaryExpr *) { return {}; }
    std::any BytecodeEmitter::visitUnaryExpr(AST::UnaryExpr *) { return {}; }
    std::any BytecodeEmitter::visitGroupingExpr(AST::GroupingExpr *) { return {}; }
    std::any BytecodeEmitter::visitAssignmentExpr(AST::AssignmentExpr *) { return {}; }
    std::any BytecodeEmitter::visitLogicalExpr(AST::LogicalExpr *) { return {}; }
    std::any BytecodeEmitter::visitCallExpr(AST::CallExpr *) { return {}; }
    std::any BytecodeEmitter::visitPostfixExpr(AST::PostfixExpr *) { return {}; }
    std::any BytecodeEmitter::visitUninitLiteralExpr(AST::UninitLiteralExpr *) { return {}; }
    std::any BytecodeEmitter::visitNewExpr(AST::NewExpr *) { return {}; }
    std::any BytecodeEmitter::visitThisExpr(AST::ThisExpr *) { return {}; }
    std::any BytecodeEmitter::visitArrayLiteralExpr(AST::ArrayLiteralExpr *) { return {}; }
    std::any BytecodeEmitter::visitMapLiteralExpr(AST::MapLiteralExpr *) { return {}; }
    std::any BytecodeEmitter::visitSubscriptExpr(AST::SubscriptExpr *) { return {}; }
    std::any BytecodeEmitter::visitInterpolatedStringExpr(AST::InterpolatedStringExpr *) { return {}; }

    void BytecodeEmitter::visitBlockStmt(AST::BlockStmt *) {}
    void BytecodeEmitter::visitIfStmt(AST::IfStmt *) {}
    void BytecodeEmitter::visitWhileStmt(AST::WhileStmt *) {}
    void BytecodeEmitter::visitDoWhileStmt(AST::DoWhileStmt *) {}
    void BytecodeEmitter::visitFunctionDeclStmt(AST::FunctionDeclStmt *) {}
    void BytecodeEmitter::visitReturnStmt(AST::ReturnStmt *) {}
    void BytecodeEmitter::visitBreakStmt(AST::BreakStmt *) {}
    void BytecodeEmitter::visitContinueStmt(AST::ContinueStmt *) {}
    void BytecodeEmitter::visitSwitchStmt(AST::SwitchStmt *) {}
    void BytecodeEmitter::visitDeleteStmt(AST::DeleteStmt *) {}
    void BytecodeEmitter::visitThrowStmt(AST::ThrowStmt *) {}
    void BytecodeEmitter::visitTryStmt(AST::TryStmt *) {}
}
