#include "BytecodeEmitter.hpp"

namespace Linh
{
    BytecodeEmitter::BytecodeEmitter() {}

    void BytecodeEmitter::emit(const AST::StmtList &stmts)
    {
        chunk.clear();
        // --- Emit all function definitions first ---
        for (const auto &stmt : stmts)
        {
            if (auto *fn = dynamic_cast<AST::FunctionDeclStmt *>(stmt.get()))
            {
                // Prepare function info
                FunctionInfo finfo;
                // Save old var_table/next_var_index
                auto old_var_table = var_table;
                auto old_next_var_index = next_var_index;
                var_table.clear();
                next_var_index = 0;
                // Add parameters to var_table
                for (const auto &param : fn->params)
                {
                    finfo.param_names.push_back(param.name.lexeme);
                    get_var_index(param.name.lexeme);
                }
                // Emit function body into finfo.code
                if (fn->body)
                {
                    auto old_chunk = chunk;
                    chunk.clear();
                    for (const auto &s : fn->body->statements)
                        if (s)
                            s->accept(this);
                    emit_instr(OpCode::RET);
                    finfo.code = chunk;
                    chunk = old_chunk;
                }
                functions[fn->name.lexeme] = std::move(finfo);
                var_table = old_var_table;
                next_var_index = old_next_var_index;
            }
        }
        // --- Emit global code (non-function statements) ---
        for (const auto &stmt : stmts)
        {
            if (!dynamic_cast<AST::FunctionDeclStmt *>(stmt.get()))
            {
                if (stmt)
                    stmt->accept(this);
            }
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

    void BytecodeEmitter::visitBlockStmt(AST::BlockStmt *stmt)
    {
        for (const auto &s : stmt->statements)
        {
            if (s)
                s->accept(this);
        }
    }

    void BytecodeEmitter::visitIfStmt(AST::IfStmt *) {}
    void BytecodeEmitter::visitWhileStmt(AST::WhileStmt *stmt)
    {
        // while (cond) body
        // [cond]
        // JMP_IF_FALSE end
        // [body]
        // JMP cond
        // end:

        size_t cond_pos = chunk.size();
        if (stmt->condition)
            stmt->condition->accept(this);

        // Đặt chỗ cho JMP_IF_FALSE, sẽ sửa sau
        size_t jmp_if_false_pos = chunk.size();
        emit_instr(OpCode::JMP_IF_FALSE, int64_t(-1)); // placeholder

        if (stmt->body)
            stmt->body->accept(this);

        // Quay lại đầu vòng lặp
        emit_instr(OpCode::JMP, int64_t(cond_pos));

        // Sửa lại JMP_IF_FALSE để nhảy ra khỏi vòng lặp
        size_t end_pos = chunk.size();
        chunk[jmp_if_false_pos].operand = int64_t(end_pos);
    }

    void BytecodeEmitter::visitDoWhileStmt(AST::DoWhileStmt *stmt)
    {
        // do { body } while (cond);
        size_t loop_start = chunk.size();
        if (stmt->body)
            stmt->body->accept(this);
        if (stmt->condition)
            stmt->condition->accept(this);
        emit_instr(OpCode::JMP_IF_TRUE, int64_t(loop_start));
    }

    void BytecodeEmitter::visitFunctionDeclStmt(AST::FunctionDeclStmt *) {}
    void BytecodeEmitter::visitReturnStmt(AST::ReturnStmt *stmt)
    {
        if (stmt->value)
            stmt->value->accept(this);
        emit_instr(OpCode::RET);
    }
    void BytecodeEmitter::visitBreakStmt(AST::BreakStmt *) {}
    void BytecodeEmitter::visitContinueStmt(AST::ContinueStmt *) {}
    void BytecodeEmitter::visitSwitchStmt(AST::SwitchStmt *) {}
    void BytecodeEmitter::visitDeleteStmt(AST::DeleteStmt *) {}
    void BytecodeEmitter::visitThrowStmt(AST::ThrowStmt *) {}
    void BytecodeEmitter::visitTryStmt(AST::TryStmt *) {}

    std::any BytecodeEmitter::visitCallExpr(AST::CallExpr *expr)
    {
        // Special case: input(...) and type(...)
        if (auto id = dynamic_cast<AST::IdentifierExpr *>(expr->callee.get()))
        {
            if (id->name.lexeme == "input")
            {
                if (!expr->arguments.empty())
                    expr->arguments[0]->accept(this);
                else
                    emit_instr(OpCode::PUSH_STR, std::string(""));
                emit_instr(OpCode::INPUT);
                return {};
            }
            if (id->name.lexeme == "type")
            {
                if (!expr->arguments.empty())
                    expr->arguments[0]->accept(this);
                else
                    emit_instr(OpCode::PUSH_STR, std::string(""));
                emit_instr(OpCode::TYPEOF);
                return {};
            }
            // --- User-defined function call ---
            // Evaluate arguments (push in order)
            for (auto &arg : expr->arguments)
                if (arg)
                    arg->accept(this);
            emit_instr(OpCode::CALL, id->name.lexeme);
            return {};
        }
        // Not implemented yet for other calls
        return {};
    }

    std::any BytecodeEmitter::visitPostfixExpr(AST::PostfixExpr *expr)
    {
        // Hỗ trợ a++ và a--
        // Chỉ hỗ trợ cho IdentifierExpr
        auto id = dynamic_cast<AST::IdentifierExpr *>(expr->operand.get());
        if (!id)
            return {};
        int idx = get_var_index(id->name.lexeme);
        // LOAD_VAR idx
        emit_instr(OpCode::LOAD_VAR, idx);
        // PUSH_INT 1
        emit_instr(OpCode::PUSH_INT, 1);
        if (expr->op_token.type == TokenType::PLUS_PLUS)
            emit_instr(OpCode::ADD);
        else if (expr->op_token.type == TokenType::MINUS_MINUS)
            emit_instr(OpCode::SUB);
        // STORE_VAR idx
        emit_instr(OpCode::STORE_VAR, idx);
        // Optionally, load value back (for expression value)
        emit_instr(OpCode::LOAD_VAR, idx);
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
