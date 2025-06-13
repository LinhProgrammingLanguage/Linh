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

    std::any BytecodeEmitter::visitBinaryExpr(AST::BinaryExpr *expr)
    {
        // Evaluate left and right
        if (expr->left)
            expr->left->accept(this);
        if (expr->right)
            expr->right->accept(this);

        // Map TokenType to OpCode
        switch (expr->op.type)
        {
        case TokenType::PLUS:
            emit_instr(OpCode::ADD);
            break;
        case TokenType::MINUS:
            emit_instr(OpCode::SUB);
            break;
        case TokenType::STAR:
            emit_instr(OpCode::MUL);
            break;
        case TokenType::SLASH:
            emit_instr(OpCode::DIV);
            break;
        case TokenType::PERCENT:
            emit_instr(OpCode::MOD);
            break;
        case TokenType::EQ_EQ:
            emit_instr(OpCode::EQ);
            break;
        case TokenType::NOT_EQ:
            emit_instr(OpCode::NEQ);
            break;
        case TokenType::LT:
            emit_instr(OpCode::LT);
            break;
        case TokenType::GT:
            emit_instr(OpCode::GT);
            break;
        case TokenType::LT_EQ:
            emit_instr(OpCode::LTE);
            break;
        case TokenType::GT_EQ:
            emit_instr(OpCode::GTE);
            break;
        case TokenType::AND_LOGIC:
        case TokenType::AND_KW:
            emit_instr(OpCode::AND);
            break;
        case TokenType::OR_LOGIC:
        case TokenType::OR_KW:
            emit_instr(OpCode::OR);
            break;
        default:
            // Not implemented
            break;
        }
        return {};
    }

    std::any BytecodeEmitter::visitUnaryExpr(AST::UnaryExpr *expr)
    {
        if (expr->right)
            expr->right->accept(this);
        switch (expr->op.type)
        {
        case TokenType::MINUS:
            emit_instr(OpCode::PUSH_INT, 0);
            emit_instr(OpCode::SWAP); // Not defined, but you may need to implement SWAP for correct order
            emit_instr(OpCode::SUB);
            break;
        case TokenType::NOT:
        case TokenType::NOT_KW:
            emit_instr(OpCode::NOT);
            break;
        default:
            break;
        }
        return {};
    }

    std::any BytecodeEmitter::visitGroupingExpr(AST::GroupingExpr *expr)
    {
        if (expr->expression)
            expr->expression->accept(this);
        return {};
    }

    std::any BytecodeEmitter::visitAssignmentExpr(AST::AssignmentExpr *expr)
    {
        // Evaluate value and store to variable
        if (expr->value)
            expr->value->accept(this);
        int idx = get_var_index(expr->name.lexeme);
        emit_instr(OpCode::STORE_VAR, idx);
        // Optionally, load the value back (for chaining)
        emit_instr(OpCode::LOAD_VAR, idx);
        return {};
    }

    std::any BytecodeEmitter::visitLogicalExpr(AST::LogicalExpr *expr)
    {
        // Short-circuit logic not implemented, fallback to eager evaluation
        if (expr->left)
            expr->left->accept(this);
        if (expr->right)
            expr->right->accept(this);
        switch (expr->op.type)
        {
        case TokenType::AND_LOGIC:
        case TokenType::AND_KW:
            emit_instr(OpCode::AND);
            break;
        case TokenType::OR_LOGIC:
        case TokenType::OR_KW:
            emit_instr(OpCode::OR);
            break;
        default:
            break;
        }
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

    std::any BytecodeEmitter::visitCallExpr(AST::CallExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitPostfixExpr(AST::PostfixExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitUninitLiteralExpr(AST::UninitLiteralExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitNewExpr(AST::NewExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitThisExpr(AST::ThisExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitArrayLiteralExpr(AST::ArrayLiteralExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitMapLiteralExpr(AST::MapLiteralExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitSubscriptExpr(AST::SubscriptExpr *expr)
    {
        // Not implemented yet
        return {};
    }

    std::any BytecodeEmitter::visitInterpolatedStringExpr(AST::InterpolatedStringExpr *expr)
    {
        // Not implemented yet
        return {};
    }
}
