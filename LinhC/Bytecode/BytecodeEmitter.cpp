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

    void BytecodeEmitter::emit_instr(OpCode op, BytecodeValue val, int line, int col)
    {
        chunk.emplace_back(op, val, line, col);
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
        // int<8>, int<16>, int<32>, int<64>, uint<8>, ... đều là int64_t ở runtime
        // float<32>, float<64> đều là double ở runtime
        if (std::holds_alternative<int64_t>(expr->value))
            emit_instr(OpCode::PUSH_INT, std::get<int64_t>(expr->value), expr->getLine(), expr->getCol());
        else if (std::holds_alternative<double>(expr->value))
            emit_instr(OpCode::PUSH_FLOAT, std::get<double>(expr->value), expr->getLine(), expr->getCol());
        else if (std::holds_alternative<std::string>(expr->value))
            emit_instr(OpCode::PUSH_STR, std::get<std::string>(expr->value), expr->getLine(), expr->getCol());
        else if (std::holds_alternative<bool>(expr->value))
            emit_instr(OpCode::PUSH_BOOL, std::get<bool>(expr->value), expr->getLine(), expr->getCol());
        return {};
    }

    std::any BytecodeEmitter::visitIdentifierExpr(AST::IdentifierExpr *expr)
    {
        emit_instr(OpCode::LOAD_VAR, get_var_index(expr->name.lexeme), expr->getLine(), expr->getCol());
        return {};
    }

    std::any BytecodeEmitter::visitBinaryExpr(AST::BinaryExpr *expr)
    {
        // Evaluate left and right
        if (expr->left)
            expr->left->accept(this);
        if (expr->right)
            expr->right->accept(this);

        int line = expr->op.line;
        int col = expr->op.column_start;
        switch (expr->op.type)
        {
        case TokenType::PLUS:
            emit_instr(OpCode::ADD, {}, line, col);
            break;
        case TokenType::MINUS:
            emit_instr(OpCode::SUB, {}, line, col);
            break;
        case TokenType::STAR:
            emit_instr(OpCode::MUL, {}, line, col);
            break;
        case TokenType::SLASH:
            emit_instr(OpCode::DIV, {}, line, col);
            break;
        case TokenType::PERCENT:
            emit_instr(OpCode::MOD, {}, line, col);
            break;
        case TokenType::STAR_STAR:
            emit_instr(OpCode::CALL, std::string("pow"), line, col);
            break;
        case TokenType::EQ_EQ:
            emit_instr(OpCode::EQ, {}, line, col);
            break;
        case TokenType::NOT_EQ:
            emit_instr(OpCode::NEQ, {}, line, col);
            break;
        case TokenType::LT:
            emit_instr(OpCode::LT, {}, line, col);
            break;
        case TokenType::GT:
            emit_instr(OpCode::GT, {}, line, col);
            break;
        case TokenType::LT_EQ:
            emit_instr(OpCode::LTE, {}, line, col);
            break;
        case TokenType::GT_EQ:
            emit_instr(OpCode::GTE, {}, line, col);
            break;
        case TokenType::AND_LOGIC:
        case TokenType::AND_KW:
            emit_instr(OpCode::AND, {}, line, col);
            break;
        case TokenType::OR_LOGIC:
        case TokenType::OR_KW:
            emit_instr(OpCode::OR, {}, line, col);
            break;
        case TokenType::HASH:
            emit_instr(OpCode::HASH, {}, line, col);
            break;
        case TokenType::PIPE:
            emit_instr(OpCode::PIPE, {}, line, col);
            break;
        case TokenType::CARET:
            emit_instr(OpCode::CARET, {}, line, col);
            break;
        case TokenType::AMP:
            emit_instr(OpCode::AMP, {}, line, col);
            break;
        case TokenType::LT_LT:
            emit_instr(OpCode::LT_LT, {}, line, col);
            break;
        case TokenType::GT_GT:
            emit_instr(OpCode::GT_GT, {}, line, col);
            break;
        default:
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
            emit_instr(OpCode::PUSH_INT, 0, expr->getLine(), expr->getCol());
            emit_instr(OpCode::SWAP, {}, expr->getLine(), expr->getCol()); // Not defined, but you may need to implement SWAP for correct order
            emit_instr(OpCode::SUB, {}, expr->getLine(), expr->getCol());
            break;
        case TokenType::NOT:
        case TokenType::NOT_KW:
            emit_instr(OpCode::NOT, {}, expr->getLine(), expr->getCol());
            break;
        case TokenType::TILDE:
            emit_instr(OpCode::NOT); // <-- Thêm dòng này cho ~
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
        emit_instr(OpCode::STORE_VAR, idx, expr->getLine(), expr->getCol());
        // Optionally, load the value back (for chaining)
        emit_instr(OpCode::LOAD_VAR, idx, expr->getLine(), expr->getCol());
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
            emit_instr(OpCode::AND, {}, expr->getLine(), expr->getCol());
            break;
        case TokenType::OR_LOGIC:
        case TokenType::OR_KW:
            emit_instr(OpCode::OR, {}, expr->getLine(), expr->getCol());
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
        emit_instr(OpCode::PRINT, {}, stmt->getLine(), stmt->getCol());
    }

    void BytecodeEmitter::visitExpressionStmt(AST::ExpressionStmt *stmt)
    {
        if (stmt->expression)
            stmt->expression->accept(this);
        emit_instr(OpCode::POP, {}, stmt->getLine(), stmt->getCol());
    }

    void BytecodeEmitter::visitVarDeclStmt(AST::VarDeclStmt *stmt)
    {
        int idx = get_var_index(stmt->name.lexeme);
        if (stmt->initializer)
            stmt->initializer->accept(this);
        else
            emit_instr(OpCode::PUSH_INT, 0, stmt->getLine(), stmt->getCol()); // default 0
        emit_instr(OpCode::STORE_VAR, idx, stmt->getLine(), stmt->getCol());
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
        emit_instr(OpCode::JMP_IF_FALSE, int64_t(-1), stmt->getLine(), stmt->getCol()); // placeholder

        if (stmt->body)
            stmt->body->accept(this);

        // Quay lại đầu vòng lặp
        emit_instr(OpCode::JMP, int64_t(cond_pos), stmt->getLine(), stmt->getCol());

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
        emit_instr(OpCode::JMP_IF_TRUE, int64_t(loop_start), stmt->getLine(), stmt->getCol());
    }

    void BytecodeEmitter::visitFunctionDeclStmt(AST::FunctionDeclStmt *) {}
    void BytecodeEmitter::visitReturnStmt(AST::ReturnStmt *stmt)
    {
        if (stmt->value)
            stmt->value->accept(this);
        emit_instr(OpCode::RET, {}, stmt->getLine(), stmt->getCol());
    }
    void BytecodeEmitter::visitBreakStmt(AST::BreakStmt *) {}
    void BytecodeEmitter::visitContinueStmt(AST::ContinueStmt *) {}
    void BytecodeEmitter::visitSwitchStmt(AST::SwitchStmt *stmt)
    {
        // --- Sinh bytecode cho switch-case ---
        if (stmt->expression_to_switch_on)
            stmt->expression_to_switch_on->accept(this);

        size_t case_count = stmt->cases.size();
        std::vector<size_t> case_jump_addrs(case_count, 0);
        std::vector<size_t> case_body_addrs(case_count, 0);
        size_t default_case_idx = size_t(-1);

        // Để sửa JMP (break) sau khi biết địa chỉ kết thúc switch
        std::vector<size_t> break_jmp_addrs;

        // 2. Sinh code kiểm tra từng case
        for (size_t i = 0; i < case_count; ++i)
        {
            const auto &case_clause = stmt->cases[i];
            if (case_clause.is_default)
            {
                default_case_idx = i;
                continue;
            }
            emit_instr(OpCode::DUP);
            if (case_clause.case_value.has_value() && case_clause.case_value.value())
                case_clause.case_value.value()->accept(this);
            else
                emit_instr(OpCode::PUSH_INT, 0);

            emit_instr(OpCode::EQ);
            size_t jmp_if_true_addr = chunk.size();
            emit_instr(OpCode::JMP_IF_TRUE, int64_t(-1));
            case_jump_addrs[i] = jmp_if_true_addr;
            // Không sinh POP ở đây!
        }

        size_t jmp_default_addr = chunk.size();
        emit_instr(OpCode::JMP, int64_t(-1));

        // 3. Sinh code cho từng case body
        for (size_t i = 0; i < case_count; ++i)
        {
            case_body_addrs[i] = chunk.size();
            if (!stmt->cases[i].is_default)
                chunk[case_jump_addrs[i]].operand = int64_t(case_body_addrs[i]);
            // Chỉ pop switch_value khi thực sự vào case body
            emit_instr(OpCode::POP);

            // Sinh code cho các statement trong case
            for (const auto &s : stmt->cases[i].statements)
            {
                if (s)
                {
                    // Nếu là BreakStmt thì sinh JMP và lưu lại vị trí để sửa sau
                    if (dynamic_cast<AST::BreakStmt *>(s.get()))
                    {
                        size_t break_jmp_addr = chunk.size();
                        emit_instr(OpCode::JMP, int64_t(-1));
                        break_jmp_addrs.push_back(break_jmp_addr);
                        // Sau break thì không sinh code cho các statement tiếp theo trong case
                        break;
                    }
                    s->accept(this);
                }
            }
        }

        // 4. Default case
        size_t default_addr = chunk.size();
        if (default_case_idx != size_t(-1))
        {
            chunk[jmp_default_addr].operand = int64_t(default_addr);
            // Khi vào default, pop switch_value
            emit_instr(OpCode::POP);
            for (const auto &s : stmt->cases[default_case_idx].statements)
            {
                if (s)
                {
                    if (dynamic_cast<AST::BreakStmt *>(s.get()))
                    {
                        size_t break_jmp_addr = chunk.size();
                        emit_instr(OpCode::JMP, int64_t(-1));
                        break_jmp_addrs.push_back(break_jmp_addr);
                        break;
                    }
                    s->accept(this);
                }
            }
        }
        else
        {
            chunk[jmp_default_addr].operand = int64_t(default_addr);
            emit_instr(OpCode::POP);
        }

        // Địa chỉ kết thúc switch
        size_t end_switch_addr = chunk.size();

        // Sửa lại tất cả JMP (break) để nhảy ra ngoài switch
        for (size_t addr : break_jmp_addrs)
        {
            chunk[addr].operand = int64_t(end_switch_addr);
        }
    }

    void BytecodeEmitter::visitDeleteStmt(AST::DeleteStmt *) {}
    void BytecodeEmitter::visitThrowStmt(AST::ThrowStmt *) {}
    void BytecodeEmitter::visitTryStmt(AST::TryStmt *stmt)
    {
        // Đặt nhãn cho catch, finally, end
        size_t try_start = chunk.size();
        size_t catch_pos = 0, finally_pos = 0, end_pos = 0;

        // Đặt biến error_var đặc biệt (ở đây dùng index -9999)
        std::string error_var = "error";

        // Đặt chỗ TRY, sẽ sửa operand sau
        size_t try_instr_pos = chunk.size();
        emit_instr(OpCode::TRY, std::make_tuple(int64_t(0), int64_t(0), int64_t(0), error_var), stmt->keyword_try.line, stmt->keyword_try.column_start);

        // Sinh code cho try_block
        if (stmt->try_block)
            stmt->try_block->accept(this);

        // Sau try_block, nhảy đến finally (nếu có), hoặc end
        size_t after_try = chunk.size();
        emit_instr(OpCode::JMP, int64_t(0), stmt->keyword_try.line, stmt->keyword_try.column_start); // placeholder

        // catch
        catch_pos = chunk.size();
        if (!stmt->catch_clauses.empty())
        {
            // Chỉ lấy catch đầu tiên (giản lược)
            auto &catch_clause = stmt->catch_clauses[0];
            // Gán biến error (index -9999)
            // (Ở đây không sinh code, VM sẽ tự gán khi lỗi)
            if (catch_clause.body)
                catch_clause.body->accept(this);
        }

        // finally
        finally_pos = chunk.size();
        if (stmt->finally_block.has_value() && stmt->finally_block.value())
        {
            stmt->finally_block.value()->accept(this);
        }

        // end
        end_pos = chunk.size();
        emit_instr(OpCode::END_TRY, {}, stmt->keyword_try.line, stmt->keyword_try.column_start);

        // Sửa lại JMP sau try_block để nhảy qua catch đến finally/end
        chunk[after_try].operand = int64_t(finally_pos);

        // Sửa lại TRY operand
        chunk[try_instr_pos].operand = std::make_tuple(
            int64_t(catch_pos),
            int64_t(finally_pos),
            int64_t(end_pos),
            error_var);
    }

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
                    emit_instr(OpCode::PUSH_STR, std::string(""), expr->getLine(), expr->getCol());
                emit_instr(OpCode::INPUT, {}, expr->getLine(), expr->getCol());
                return {};
            }
            if (id->name.lexeme == "type")
            {
                if (!expr->arguments.empty())
                    expr->arguments[0]->accept(this);
                else
                    emit_instr(OpCode::PUSH_STR, std::string(""), expr->getLine(), expr->getCol());
                emit_instr(OpCode::TYPEOF, {}, expr->getLine(), expr->getCol());
                return {};
            }
            // --- User-defined function call ---
            // Evaluate arguments (push in order)
            for (auto &arg : expr->arguments)
                if (arg)
                    arg->accept(this);
            emit_instr(OpCode::CALL, id->name.lexeme, expr->getLine(), expr->getCol());
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
        emit_instr(OpCode::LOAD_VAR, idx, expr->getLine(), expr->getCol());
        // PUSH_INT 1
        emit_instr(OpCode::PUSH_INT, 1, expr->getLine(), expr->getCol());
        if (expr->op_token.type == TokenType::PLUS_PLUS)
            emit_instr(OpCode::ADD, {}, expr->getLine(), expr->getCol());
        else if (expr->op_token.type == TokenType::MINUS_MINUS)
            emit_instr(OpCode::SUB, {}, expr->getLine(), expr->getCol());
        // STORE_VAR idx
        emit_instr(OpCode::STORE_VAR, idx, expr->getLine(), expr->getCol());
        // Optionally, load value back (for expression value)
        emit_instr(OpCode::LOAD_VAR, idx, expr->getLine(), expr->getCol());
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
        // Nếu chỉ có 1 phần là chuỗi thì PUSH_STR luôn
        if (expr->parts.size() == 1 && std::holds_alternative<std::string>(expr->parts[0]))
        {
            emit_instr(OpCode::PUSH_STR, std::get<std::string>(expr->parts[0]), expr->getLine(), expr->getCol());
            return {};
        }
        // Duyệt từng phần, đẩy từng phần lên stack (dưới dạng string)
        for (const auto &part : expr->parts)
        {
            if (std::holds_alternative<std::string>(part))
            {
                emit_instr(OpCode::PUSH_STR, std::get<std::string>(part), expr->getLine(), expr->getCol());
            }
            else
            {
                // Phần là biểu thức, emit cho expr, rồi ép về string bằng cách nối với ""
                auto *subexpr = std::get<AST::ExprPtr>(part).get();
                if (subexpr)
                {
                    subexpr->accept(this);
                    emit_instr(OpCode::PUSH_STR, std::string(""), expr->getLine(), expr->getCol());
                    emit_instr(OpCode::ADD, {}, expr->getLine(), expr->getCol()); // ép về string
                }
            }
        }
        // Nối tất cả lại thành một chuỗi (dùng toán tử +)
        for (size_t i = 1; i < expr->parts.size(); ++i)
        {
            emit_instr(OpCode::ADD, {}, expr->getLine(), expr->getCol());
        }
        return {};
    }

    void BytecodeEmitter::visitImportStmt(AST::ImportStmt * /*stmt*/)
    {
        // Không sinh bytecode cho import (hoặc xử lý import module ở đây nếu cần)
    }
}