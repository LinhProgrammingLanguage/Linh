#include "SemanticAnalyzer.hpp"
#include "../../../config.hpp"

namespace Linh
{
    namespace Semantic
    {

        void SemanticAnalyzer::analyze(const AST::StmtList &stmts, bool reset_state)
        {
            if (reset_state)
            {
                var_scopes.clear();
                global_functions.clear();
                begin_scope();
                for (const auto &stmt : stmts)
                {
                    if (stmt)
                        stmt->accept(this);
                }
                end_scope();
            }
            else
            {
                // REPL mode: giữ lại scope toàn cục, không pop/push scope
                if (var_scopes.empty())
                {
                    begin_scope(); // Đảm bảo luôn có scope ngoài cùng
                }
                for (const auto &stmt : stmts)
                {
                    if (stmt)
                        stmt->accept(this);
                }
            }
        }

        void SemanticAnalyzer::begin_scope()
        {
            var_scopes.push_back({});
        }
        void SemanticAnalyzer::end_scope()
        {
            if (!var_scopes.empty())
                var_scopes.pop_back();
        }
        void SemanticAnalyzer::declare_var(const std::string &name, const std::string &kind /*= ""*/, const std::string &type /*= ""*/)
        {
            if (!var_scopes.empty())
                var_scopes.back()[name] = true;
            if (!kind.empty())
                var_kinds[name] = kind;
            if (!type.empty())
                var_types[name] = type;
        }
        bool SemanticAnalyzer::is_var_declared(const std::string &name)
        {
            for (auto it = var_scopes.rbegin(); it != var_scopes.rend(); ++it)
            {
                if (it->count(name))
                    return true;
            }
            return false;
        }
        void SemanticAnalyzer::declare_function(const std::string &name, size_t param_count /*= 0*/)
        {
            global_functions[name] = true;
            function_param_counts[name] = param_count;
        }
        bool SemanticAnalyzer::is_function_declared(const std::string &name)
        {
            return global_functions.count(name) > 0;
        }

        void SemanticAnalyzer::visitExpressionStmt(AST::ExpressionStmt *stmt)
        {
            if (stmt->expression)
                stmt->expression->accept(this);
        }
        void SemanticAnalyzer::visitPrintStmt(AST::PrintStmt *stmt)
        {
            if (stmt->expression)
                stmt->expression->accept(this);
        }
        void SemanticAnalyzer::visitVarDeclStmt(AST::VarDeclStmt *stmt)
        {
            // Check for uninit type/value rules
            bool type_is_uninit = is_uninit_type(stmt->declared_type);
            bool value_is_uninit = is_uninit_expr(stmt->initializer);

            std::string kw = stmt->keyword.lexeme;
            int line = stmt->keyword.line;
            int col = stmt->keyword.column_start;

            if (kw == "vas" || kw == "const")
            {
                if (type_is_uninit)
                {
                    errors.emplace_back("'" + kw + "' cannot declare type 'uninit'.", line, col);
                }
                // Always error if value is uninit (even if type is omitted)
                if (value_is_uninit)
                {
                    errors.emplace_back("'" + kw + "' cannot be initialized with 'uninit' value.", line, col);
                }
            }
            else if (kw == "var")
            {
                // Only error if explicit type is given (not uninit) and value is uninit
                if (stmt->declared_type.has_value() && !type_is_uninit && value_is_uninit)
                {
                    errors.emplace_back("'var' with a specific type cannot be initialized with 'uninit' value.", line, col);
                }
            }
            // Kiểm tra trùng tên biến trong cùng scope
            if (!var_scopes.empty() && var_scopes.back().count(stmt->name.lexeme))
            {
                errors.emplace_back("Variable '" + stmt->name.lexeme + "' redeclared in the same scope.", stmt->name.line, stmt->name.column_start);
            }
            // Kiểm tra trùng tên biến với tên hàm toàn cục
            if (is_function_declared(stmt->name.lexeme))
            {
                errors.emplace_back("Variable '" + stmt->name.lexeme + "' redeclared as a function name.", stmt->name.line, stmt->name.column_start);
            }

            // Kiểm tra tham số hàm không trùng tên với biến toàn cục (scope ngoài cùng)
            if (!var_scopes.empty() && var_scopes.front().count(stmt->name.lexeme))
            {
                // Đã kiểm tra ở trên
            }

            // Lưu loại biến và kiểu biến (giản lược: chỉ lấy tên kiểu nếu có)
            std::string kind = kw;
            std::string type;
            int bit_width = 0;
            std::string base_type_name;
            bool has_template = false;
            int str_limit = -1;
            if (stmt->declared_type.has_value() && stmt->declared_type.value())
            {
                auto *base = dynamic_cast<AST::BaseTypeNode *>(stmt->declared_type.value().get());
                auto *sized_int = dynamic_cast<AST::SizedIntegerTypeNode *>(stmt->declared_type.value().get());
                auto *sized_float = dynamic_cast<AST::SizedFloatTypeNode *>(stmt->declared_type.value().get());
                auto *map_type = dynamic_cast<AST::MapTypeNode *>(stmt->declared_type.value().get());
                auto *array_type = dynamic_cast<AST::ArrayTypeNode *>(stmt->declared_type.value().get());

                if (base)
                {
                    type = base->type_keyword_token.lexeme;
                    base_type_name = type;
                    if (base->template_arg.has_value())
                    {
                        bit_width = base->template_arg.value();
                        has_template = true;
                    }
                    if (base->type_keyword_token.lexeme == "str" && base->template_arg.has_value())
                    {
                        str_limit = base->template_arg.value();
                    }
                }
                else if (sized_int)
                {
                    type = sized_int->base_type_keyword_token.lexeme;
                    base_type_name = type;
                    if (sized_int->template_arg.has_value())
                    {
                        bit_width = sized_int->template_arg.value();
                        has_template = true;
                    }
                }
                else if (sized_float)
                {
                    type = sized_float->base_type_keyword_token.lexeme;
                    base_type_name = type;
                    if (sized_float->template_arg.has_value())
                    {
                        bit_width = sized_float->template_arg.value();
                        has_template = true;
                    }
                }
                else if (map_type)
                {
                    type = "map";
                    base_type_name = type;
                    has_template = true;
                }
                else if (array_type)
                {
                    type = "array";
                    base_type_name = type;
                    has_template = true;
                }
                else
                {
                    type = "";
                }
            }
            else if (stmt->initializer)
            {
                if (auto lit = dynamic_cast<AST::LiteralExpr *>(stmt->initializer.get()))
                {
                    type = SemanticAnalyzer::get_linh_literal_type(lit);
                }
            }
            else
            {
                type = "";
            }

            // Chỉ cho phép <...> với int, uint, float, map, array, str
            if (has_template)
            {
                if (type != "int" && type != "uint" && type != "float" && type != "map" && type != "array" && type != "str")
                {
                    errors.emplace_back("Type '" + type + "' does not support template specification (e.g. '<...>').", stmt->name.line, stmt->name.column_start);
                }
            }

            // Kiểm tra số bit hợp lệ cho kiểu số
            if (!type.empty() && (type == "int" || type == "uint" || type == "float"))
            {
                if (bit_width != 0)
                {
                    bool valid = false;
                    for (int opt : number_bit_options)
                    {
                        if (bit_width == opt)
                        {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid)
                    {
                        errors.emplace_back("Invalid bit width for type '" + type + "<" + std::to_string(bit_width) + ">' (must be one of: 8, 16, 32, 64, 128).", stmt->name.line, stmt->name.column_start);
                    }
                }
            }
            // Kiểm tra str<index> chỉ cho phép số nguyên dương
            if (type == "str" && bit_width != 0)
            {
                if (bit_width <= 0)
                {
                    errors.emplace_back("Type 'str<" + std::to_string(bit_width) + ">' index must be positive.", stmt->name.line, stmt->name.column_start);
                }
            }

            // Nếu có giới hạn str<index> thì lưu vào var_str_limit
            if (type == "str" && str_limit > 0)
            {
                var_str_limit[stmt->name.lexeme] = str_limit;
            }

            // Nếu có giới hạn str<index> và có initializer là LiteralExpr thì cắt chuỗi
            if (type == "str" && str_limit > 0 && stmt->initializer)
            {
                if (auto lit = dynamic_cast<AST::LiteralExpr *>(stmt->initializer.get()))
                {
                    if (std::holds_alternative<std::string>(lit->value))
                    {
                        std::string val = std::get<std::string>(lit->value);
                        if (static_cast<int>(val.size()) > str_limit)
                        {
                            // Cắt chuỗi
                            std::string cut_val = val.substr(0, str_limit);
                            lit->value = cut_val;
                        }
                    }
                }
            }
            // Kiểm tra kiểu không phải số/map/array/str mà lại có template/bit_width
            if (!type.empty() && type != "int" && type != "uint" && type != "float" && type != "map" && type != "array" && type != "str" && has_template)
            {
                errors.emplace_back("Type '" + type + "' does not support template specification (e.g. '<...>').", stmt->name.line, stmt->name.column_start);
            }

            declare_var(stmt->name.lexeme, kind, type);

            // Recursively check initializer
            if (stmt->initializer)
                stmt->initializer->accept(this);
        }
        void SemanticAnalyzer::visitBlockStmt(AST::BlockStmt *stmt)
        {
            begin_scope();
            for (const auto &s : stmt->statements)
            {
                if (s)
                    s->accept(this);
            }
            end_scope();
        }
        void SemanticAnalyzer::visitIfStmt(AST::IfStmt *stmt)
        {
            if (stmt->condition)
                stmt->condition->accept(this);
            if (stmt->then_branch)
                stmt->then_branch->accept(this);
            if (stmt->else_branch)
                stmt->else_branch->accept(this);
        }
        void SemanticAnalyzer::visitWhileStmt(AST::WhileStmt *stmt)
        {
            if (stmt->condition)
                stmt->condition->accept(this);
            // Đánh dấu scope là trong vòng lặp
            begin_scope();
            var_scopes.back()["__loop__"] = true;
            if (stmt->body)
                stmt->body->accept(this);
            end_scope();
        }
        void SemanticAnalyzer::visitDoWhileStmt(AST::DoWhileStmt *stmt)
        {
            if (stmt->body)
                stmt->body->accept(this);
            if (stmt->condition)
                stmt->condition->accept(this);
        }
        void SemanticAnalyzer::visitFunctionDeclStmt(AST::FunctionDeclStmt *stmt)
        {
            // Kiểm tra trùng tên hàm với biến toàn cục (scope ngoài cùng)
            if (!var_scopes.empty() && var_scopes.front().count(stmt->name.lexeme))
            {
                errors.emplace_back("Function '" + stmt->name.lexeme + "' redeclared as a variable name.", stmt->name.line, stmt->name.column_start);
            }
            // Đăng ký tên hàm vào global_functions và lưu số lượng tham số
            declare_function(stmt->name.lexeme, stmt->params.size());

            // Kiểm tra trùng tên tham số trong danh sách tham số
            std::unordered_map<std::string, bool> param_names;
            for (const auto &param : stmt->params)
            {
                if (param_names.count(param.name.lexeme))
                {
                    errors.emplace_back("Parameter '" + param.name.lexeme + "' redeclared in parameter list.", param.name.line, param.name.column_start);
                }
                param_names[param.name.lexeme] = true;
            }

            begin_scope();
            // Đăng ký tham số vào scope mới
            for (const auto &param : stmt->params)
            {
                declare_var(param.name.lexeme);
            }
            // Kiểm tra return trong hàm nếu có yêu cầu trả về giá trị
            bool has_return = false;
            if (stmt->body)
            {
                // Duyệt các statement trong body để tìm return
                for (const auto &s : stmt->body->statements)
                {
                    if (dynamic_cast<AST::ReturnStmt *>(s.get()))
                        has_return = true;
                }
                stmt->body->accept(this);
            }
            // Giả sử stmt->return_type là optional<TypeNodePtr> và nullptr nếu không có
            if (stmt->return_type.has_value() && stmt->return_type.value() && !has_return)
            {
                errors.emplace_back("Function '" + stmt->name.lexeme + "' must have a return statement.", stmt->name.line, stmt->name.column_start);
            }
            end_scope();
        }
        void SemanticAnalyzer::visitReturnStmt(AST::ReturnStmt *stmt)
        {
            if (stmt->value)
                stmt->value->accept(this);
        }
        void SemanticAnalyzer::visitBreakStmt(AST::BreakStmt *stmt)
        {
            // Kiểm tra break ngoài vòng lặp
            bool in_loop = false;
            for (auto it = var_scopes.rbegin(); it != var_scopes.rend(); ++it)
            {
                if (it->count("__loop__"))
                {
                    in_loop = true;
                    break;
                }
            }
            if (!in_loop)
            {
                // Sửa lại: dùng stmt->keyword thay vì stmt->token
                errors.emplace_back("'break' statement not inside a loop.", stmt->keyword.line, stmt->keyword.column_start);
            }
        }
        void SemanticAnalyzer::visitContinueStmt(AST::ContinueStmt *stmt)
        {
            // Kiểm tra continue ngoài vòng lặp
            bool in_loop = false;
            for (auto it = var_scopes.rbegin(); it != var_scopes.rend(); ++it)
            {
                if (it->count("__loop__"))
                {
                    in_loop = true;
                    break;
                }
            }
            if (!in_loop)
            {
                // Sửa lại: dùng stmt->keyword thay vì stmt->token
                errors.emplace_back("'continue' statement not inside a loop.", stmt->keyword.line, stmt->keyword.column_start);
            }
        }
        void SemanticAnalyzer::visitSwitchStmt(AST::SwitchStmt *stmt)
        {
            if (stmt->expression_to_switch_on)
                stmt->expression_to_switch_on->accept(this);
            for (const auto &c : stmt->cases)
            {
                for (const auto &s : c.statements)
                {
                    if (s)
                        s->accept(this);
                }
            }
        }
        void SemanticAnalyzer::visitDeleteStmt(AST::DeleteStmt *stmt)
        {
            if (stmt->expression_to_delete)
                stmt->expression_to_delete->accept(this);
        }
        void SemanticAnalyzer::visitThrowStmt(AST::ThrowStmt *stmt)
        {
            if (stmt->expression)
                stmt->expression->accept(this);
        }
        void SemanticAnalyzer::visitTryStmt(AST::TryStmt *stmt)
        {
            if (stmt->try_block)
                stmt->try_block->accept(this);
            for (const auto &c : stmt->catch_clauses)
            {
                if (c.body)
                    c.body->accept(this);
            }
            if (stmt->finally_block.has_value() && stmt->finally_block.value())
            {
                stmt->finally_block.value()->accept(this);
            }
        }

        // ExprVisitor (only need to traverse, except UninitLiteralExpr)
        std::any SemanticAnalyzer::visitBinaryExpr(AST::BinaryExpr *expr)
        {
            if (expr->left)
                expr->left->accept(this);
            if (expr->right)
                expr->right->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitUnaryExpr(AST::UnaryExpr *expr)
        {
            if (expr->right)
                expr->right->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitLiteralExpr(AST::LiteralExpr *) { return {}; }
        std::any SemanticAnalyzer::visitGroupingExpr(AST::GroupingExpr *expr)
        {
            if (expr->expression)
                expr->expression->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitIdentifierExpr(AST::IdentifierExpr *expr)
        {
            // Cho phép dùng 'input' như một hàm built-in mà không cần khai báo
            if (expr->name.lexeme == "input" || expr->name.lexeme == "type")
            {
                // Không kiểm tra biến/hàm cho 'input' hoặc 'type'
                return {};
            }
            // Ưu tiên kiểm tra hàm trước biến
            if (is_function_declared(expr->name.lexeme))
            {
                // Nếu là tên hàm, không báo lỗi dùng như biến (cho phép dùng tên hàm như giá trị hàm)
                return {};
            }
            // Kiểm tra biến đã khai báo chưa
            if (!is_var_declared(expr->name.lexeme))
            {
                errors.emplace_back("Variable '" + expr->name.lexeme + "' used before declaration.", expr->name.line, expr->name.column_start);
            }
            return {};
        }
        std::any SemanticAnalyzer::visitAssignmentExpr(AST::AssignmentExpr *expr)
        {
            std::string name = expr->name.lexeme;
            if (!name.empty())
            {
                // Không cho phép gán lại cho const
                if (var_kinds.count(name) && var_kinds[name] == "const")
                {
                    errors.emplace_back("Cannot assign to '" + name + "' because it is declared as 'const'.", expr->name.line, expr->name.column_start);
                }
                // Kiểm tra kiểu khi gán lại cho vas/var nếu có thể xác định kiểu
                if ((var_kinds.count(name) && (var_kinds[name] == "vas" || var_kinds[name] == "var")) && var_types.count(name))
                {
                    std::string old_type = var_types[name];
                    std::string new_type;
                    if (expr->value)
                    {
                        if (auto lit = dynamic_cast<AST::LiteralExpr *>(expr->value.get()))
                        {
                            new_type = get_linh_literal_type(lit);
                        }
                    }
                    if (!new_type.empty() && !old_type.empty() && new_type != old_type)
                    {
                        if (var_kinds[name] == "vas")
                        {
                            errors.emplace_back("Cannot change type of 'vas' variable '" + name + "' from '" + old_type + "' to '" + new_type + "'.", expr->name.line, expr->name.column_start);
                        }
                        else if (var_kinds[name] == "var")
                        {
                            var_types[name] = new_type; // cập nhật kiểu mới cho var
                        }
                    }
                }

                // --- Cắt chuỗi khi gán lại cho biến kiểu str<index> ---
                if (var_types.count(name) && var_types[name] == "str" && var_str_limit.count(name))
                {
                    int str_limit = var_str_limit[name];
                    if (str_limit > 0 && expr->value)
                    {
                        if (auto lit = dynamic_cast<AST::LiteralExpr *>(expr->value.get()))
                        {
                            if (std::holds_alternative<std::string>(lit->value))
                            {
                                std::string val = std::get<std::string>(lit->value);
                                if (static_cast<int>(val.size()) > str_limit)
                                {
                                    std::string cut_val = val.substr(0, str_limit);
                                    lit->value = cut_val;
                                }
                            }
                        }
                    }
                }
            }
            if (expr->value)
                expr->value->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitLogicalExpr(AST::LogicalExpr *expr)
        {
            if (expr->left)
                expr->left->accept(this);
            if (expr->right)
                expr->right->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitCallExpr(AST::CallExpr *expr)
        {
            // Nếu callee là IdentifierExpr thì kiểm tra tên hàm
            if (auto id = dynamic_cast<AST::IdentifierExpr *>(expr->callee.get()))
            {
                // Cho phép gọi hàm input hoặc type mà không cần khai báo trước (giống Python)
                if (id->name.lexeme != "input" && id->name.lexeme != "type")
                {
                    if (!is_function_declared(id->name.lexeme))
                    {
                        errors.emplace_back("Function '" + id->name.lexeme + "' called but not declared.", id->name.line, id->name.column_start);
                    }
                }
                // Không cần báo lỗi nếu tên này là biến (vì có thể shadow hoặc cho phép tên biến trùng tên hàm)
                // Kiểm tra số lượng tham số khi gọi hàm (trừ input, type)
                if (id->name.lexeme != "input" && id->name.lexeme != "type" && function_param_counts.count(id->name.lexeme))
                {
                    size_t expected = function_param_counts[id->name.lexeme];
                    size_t actual = expr->arguments.size();
                    if (expected != actual)
                    {
                        errors.emplace_back("Function '" + id->name.lexeme + "' called with wrong number of arguments (expected " + std::to_string(expected) + ", got " + std::to_string(actual) + ").", id->name.line, id->name.column_start);
                    }
                }
            }
            if (expr->callee)
                expr->callee->accept(this);
            for (const auto &arg : expr->arguments)
            {
                if (arg)
                    arg->accept(this);
            }
            return {};
        }
        std::any SemanticAnalyzer::visitPostfixExpr(AST::PostfixExpr *expr)
        {
            if (expr->operand)
                expr->operand->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitUninitLiteralExpr(AST::UninitLiteralExpr *)
        {
            // Nothing to do here
            return {};
        }
        std::any SemanticAnalyzer::visitNewExpr(AST::NewExpr *expr)
        {
            if (expr->class_constructor_call)
                expr->class_constructor_call->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitThisExpr(AST::ThisExpr *) { return {}; }
        std::any SemanticAnalyzer::visitArrayLiteralExpr(AST::ArrayLiteralExpr *expr)
        {
            for (const auto &el : expr->elements)
            {
                if (el)
                    el->accept(this);
            }
            return {};
        }
        std::any SemanticAnalyzer::visitMapLiteralExpr(AST::MapLiteralExpr *expr)
        {
            for (const auto &entry : expr->entries)
            {
                if (entry.key)
                    entry.key->accept(this);
                if (entry.value)
                    entry.value->accept(this);
            }
            return {};
        }
        std::any SemanticAnalyzer::visitSubscriptExpr(AST::SubscriptExpr *expr)
        {
            if (expr->object)
                expr->object->accept(this);
            if (expr->index)
                expr->index->accept(this);
            return {};
        }
        std::any SemanticAnalyzer::visitInterpolatedStringExpr(AST::InterpolatedStringExpr *expr)
        {
            for (const auto &part : expr->parts)
            {
                if (std::holds_alternative<AST::ExprPtr>(part))
                {
                    auto &e = std::get<AST::ExprPtr>(part);
                    if (e)
                        e->accept(this);
                }
            }
            return {};
        }

        // Helper: xác định kiểu literal cho Linh từ LiteralExpr
        std::string SemanticAnalyzer::get_linh_literal_type(const AST::LiteralExpr *lit)
        {
            if (!lit)
                return "";
            // Giả sử LiteralExpr có trường value kiểu std::variant<std::monostate, int64_t, double, std::string, bool>
            if (std::holds_alternative<int64_t>(lit->value))
                return "int";
            if (std::holds_alternative<double>(lit->value))
                return "float";
            if (std::holds_alternative<std::string>(lit->value))
                return "str";
            if (std::holds_alternative<bool>(lit->value))
                return "bool";
            // Nếu có thêm kiểu nào khác, bổ sung tại đây
            return "";
        }

        // Helpers
        bool SemanticAnalyzer::is_uninit_type(const std::optional<AST::TypeNodePtr> &type)
        {
            if (!type.has_value() || !type.value())
                return false;
            auto *base = dynamic_cast<AST::BaseTypeNode *>(type.value().get());
            return base && base->type_keyword_token.type == TokenType::UNINIT_KW;
        }
        bool SemanticAnalyzer::is_uninit_expr(const AST::ExprPtr &expr)
        {
            if (!expr)
                return false;
            return dynamic_cast<AST::UninitLiteralExpr *>(expr.get()) != nullptr;
        }

        // Semantic error retrieval
        const std::vector<SemanticError> &SemanticAnalyzer::get_errors() const
        {
            return errors;
        }

    } // namespace Semantic
} // namespace Linh
