#include "SemanticAnalyzer.hpp"
#include "../../../config.hpp"
#include <array>                              // thêm dòng này
#include <fstream>                            // thêm dòng này
#include "../Parser/Parser.hpp"               // thêm dòng này
#include "../../Bytecode/BytecodeEmitter.hpp" // Thêm dòng này để dùng BytecodeEmitter

namespace Linh
{
    namespace Semantic
    {

        // Thêm các mảng constexpr cho bit options
        constexpr std::array<int, 4> int_and_uint_bit_options = {8, 16, 32, 64};
        constexpr std::array<int, 2> float_bit_options = {32, 64};

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
                // Bỏ báo lỗi cho phép vas và const được khởi tạo với uninit
                // if (value_is_uninit)
                // {
                //     errors.emplace_back("'" + kw + "' cannot be initialized with 'uninit' value.", line, col);
                // }
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
            if (!type.empty() && (type == "int" || type == "uint"))
            {
                if (bit_width != 0)
                {
                    bool valid = false;
                    for (int opt : int_and_uint_bit_options)
                    {
                        if (bit_width == opt)
                        {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid)
                    {
                        errors.emplace_back("Invalid bit width for type '" + type + "<" + std::to_string(bit_width) + ">' (must be one of: 8, 16, 32, 64).", stmt->name.line, stmt->name.column_start);
                    }
                }
            }
            else if (!type.empty() && type == "float")
            {
                if (bit_width != 0)
                {
                    bool valid = false;
                    for (int opt : float_bit_options)
                    {
                        if (bit_width == opt)
                        {
                            valid = true;
                            break;
                        }
                    }
                    if (!valid)
                    {
                        errors.emplace_back("Invalid bit width for type 'float<" + std::to_string(bit_width) + ">' (must be one of: 32, 64).", stmt->name.line, stmt->name.column_start);
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
                // --- Sửa: Không bắt buộc return nếu kiểu trả về là void hoặc uninit ---
                auto *base = dynamic_cast<AST::BaseTypeNode *>(stmt->return_type.value().get());
                if (!base || (base->type_keyword_token.type != TokenType::VOID_KW && base->type_keyword_token.type != TokenType::UNINIT_KW))
                {
                    errors.emplace_back("Function '" + stmt->name.lexeme + "' must have a return statement.", stmt->name.line, stmt->name.column_start);
                }
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
            // Kiểm tra break ngoài vòng lặp hoặc switch
            bool in_loop_or_switch = false;
            for (auto it = var_scopes.rbegin(); it != var_scopes.rend(); ++it)
            {
                if (it->count("__loop__") || it->count("__switch__"))
                {
                    in_loop_or_switch = true;
                    break;
                }
            }
            if (!in_loop_or_switch)
            {
                errors.emplace_back("'break' statement not inside a loop or switch.", stmt->getLine(), stmt->getCol());
            }
        }
        void SemanticAnalyzer::visitContinueStmt(AST::ContinueStmt *stmt)
        {
            // Kiểm tra continue ngoài vòng lặp hoặc switch
            bool in_loop_or_switch = false;
            for (auto it = var_scopes.rbegin(); it != var_scopes.rend(); ++it)
            {
                if (it->count("__loop__") || it->count("__switch__"))
                {
                    in_loop_or_switch = true;
                    break;
                }
            }
            if (!in_loop_or_switch)
            {
                errors.emplace_back("'continue' statement not inside a loop or switch.", stmt->getLine(), stmt->getCol());
            }
        }
        void SemanticAnalyzer::visitSwitchStmt(AST::SwitchStmt *stmt)
        {
            if (stmt->expression_to_switch_on)
                stmt->expression_to_switch_on->accept(this);

            // Đánh dấu scope là trong switch
            begin_scope();
            var_scopes.back()["__switch__"] = true;
            for (const auto &c : stmt->cases)
            {
                for (const auto &s : c.statements)
                {
                    if (s)
                        s->accept(this);
                }
            }
            end_scope();
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
                // --- Sửa tại đây: Đưa biến catch vào scope ---
                if (c.exception_variable.has_value())
                {
                    begin_scope();
                    declare_var(c.exception_variable.value().lexeme);
                    if (c.body)
                        c.body->accept(this);
                    end_scope();
                }
                else
                {
                    if (c.body)
                        c.body->accept(this);
                }
            }
            if (stmt->finally_block.has_value() && stmt->finally_block.value())
            {
                stmt->finally_block.value()->accept(this);
            }
        }

        void SemanticAnalyzer::visitImportStmt(AST::ImportStmt *stmt)
        {
            // Đảm bảo chỉ xử lý import module dạng: import module_name;
            if (!stmt->module_name.lexeme.empty())
            {
                std::string module_path = "Li/" + stmt->module_name.lexeme;
                if (module_path.find(".li") == std::string::npos)
                    module_path += ".li";
                std::ifstream mod_file(module_path);
                if (!mod_file)
                {
                    errors.emplace_back("Không thể mở file module: " + module_path, stmt->module_name.line, stmt->module_name.column_start);
                    return;
                }
                std::string line, mod_source;
                while (std::getline(mod_file, line))
                    mod_source += line + "\n";
                // Lex và parse module
                Linh::Lexer mod_lexer(mod_source);
                auto mod_tokens = mod_lexer.scan_tokens();
                Linh::Parser mod_parser(mod_tokens);
                auto mod_ast = mod_parser.parse();
                if (mod_parser.had_error())
                {
                    errors.emplace_back("Lỗi cú pháp trong module: " + module_path, stmt->module_name.line, stmt->module_name.column_start);
                    return;
                }
                // Phân tích semantic cho module (không reset state để giữ lại các hàm/biến)
                this->analyze(mod_ast, false);

                // --- Sinh bytecode cho module và merge function table ---
                Linh::BytecodeEmitter mod_emitter;
                mod_emitter.emit(mod_ast);
                // Giả sử bạn có một con trỏ emitter chính hoặc một biến toàn cục để merge
                extern Linh::BytecodeEmitter *g_main_emitter; // Khai báo ở Main.cpp
                if (g_main_emitter)
                {
                    auto &main_funcs = g_main_emitter->get_functions(); // non-const reference
                    for (const auto &kv : mod_emitter.get_functions())
                    {
                        // Nếu chưa có trong main emitter thì thêm vào
                        if (main_funcs.count(kv.first) == 0)
                        {
                            main_funcs.insert(kv);
                        }
                    }
                }
                // --- Kết thúc merge ---
            }
            // ...nếu muốn hỗ trợ import name from module thì xử lý thêm ở đây...
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
            // --- Sửa tại đây: Cho phép error.message nếu error đã khai báo ---
            const std::string &lex = expr->name.lexeme;
            auto dot_pos = lex.find('.');
            if (dot_pos != std::string::npos)
            {
                std::string base = lex.substr(0, dot_pos);
                if (is_var_declared(base))
                {
                    // Cho phép error.message nếu error đã khai báo
                    return {};
                }
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
                // Không cho phép đổi kiểu cho vas
                else if (var_kinds.count(name) && var_kinds[name] == "vas" && var_types.count(name))
                {
                    std::string old_type = var_types[name];
                    std::string new_type;
                    // Nếu là literal thì lấy kiểu như cũ
                    if (expr->value)
                    {
                        if (auto lit = dynamic_cast<AST::LiteralExpr *>(expr->value.get()))
                        {
                            new_type = get_linh_literal_type(lit);
                        }
                        else if (auto id = dynamic_cast<AST::IdentifierExpr *>(expr->value.get()))
                        {
                            // Nếu gán từ biến khác, lấy kiểu của biến đó nếu có
                            std::string rhs_name = id->name.lexeme;
                            if (var_types.count(rhs_name))
                                new_type = var_types[rhs_name];
                        }
                        else if (auto bin = dynamic_cast<AST::BinaryExpr *>(expr->value.get()))
                        {
                            // Nếu là biểu thức nhị phân, thử lấy kiểu của vế trái hoặc phải nếu là literal/identifier
                            std::string left_type, right_type;
                            if (auto litl = dynamic_cast<AST::LiteralExpr *>(bin->left.get()))
                                left_type = get_linh_literal_type(litl);
                            else if (auto idl = dynamic_cast<AST::IdentifierExpr *>(bin->left.get()))
                                if (var_types.count(idl->name.lexeme))
                                    left_type = var_types[idl->name.lexeme];
                            if (auto litr = dynamic_cast<AST::LiteralExpr *>(bin->right.get()))
                                right_type = get_linh_literal_type(litr);
                            else if (auto idr = dynamic_cast<AST::IdentifierExpr *>(bin->right.get()))
                                if (var_types.count(idr->name.lexeme))
                                    right_type = var_types[idr->name.lexeme];
                            // Ưu tiên lấy kiểu giống old_type nếu có
                            if (left_type == old_type)
                                new_type = left_type;
                            else if (right_type == old_type)
                                new_type = right_type;
                            else if (!left_type.empty())
                                new_type = left_type;
                            else if (!right_type.empty())
                                new_type = right_type;
                        }
                        // Có thể mở rộng cho các loại biểu thức khác nếu cần
                    }
                    if (!new_type.empty() && !old_type.empty() && new_type != old_type)
                    {
                        errors.emplace_back("Cannot change type of 'vas' variable '" + name + "' from '" + old_type + "' to '" + new_type + "'.", expr->name.line, expr->name.column_start);
                    }
                    // Nếu không xác định được kiểu mới (biểu thức phức tạp), không cho phép đổi kiểu
                    if (new_type.empty() && expr->value)
                    {
                        errors.emplace_back("Cannot assign non-literal or unknown type to 'vas' variable '" + name + "'.", expr->name.line, expr->name.column_start);
                    }
                }
                // Nếu là var thì cho phép đổi kiểu (cập nhật lại var_types)
                else if (var_kinds.count(name) && var_kinds[name] == "var" && var_types.count(name))
                {
                    std::string new_type;
                    if (expr->value)
                    {
                        if (auto lit = dynamic_cast<AST::LiteralExpr *>(expr->value.get()))
                        {
                            new_type = get_linh_literal_type(lit);
                        }
                        else if (auto id = dynamic_cast<AST::IdentifierExpr *>(expr->value.get()))
                        {
                            std::string rhs_name = id->name.lexeme;
                            if (var_types.count(rhs_name))
                                new_type = var_types[rhs_name];
                        }
                        else if (auto bin = dynamic_cast<AST::BinaryExpr *>(expr->value.get()))
                        {
                            std::string left_type, right_type;
                            if (auto litl = dynamic_cast<AST::LiteralExpr *>(bin->left.get()))
                                left_type = get_linh_literal_type(litl);
                            else if (auto idl = dynamic_cast<AST::IdentifierExpr *>(bin->left.get()))
                                if (var_types.count(idl->name.lexeme))
                                    left_type = var_types[idl->name.lexeme];
                            if (auto litr = dynamic_cast<AST::LiteralExpr *>(bin->right.get()))
                                right_type = get_linh_literal_type(litr);
                            else if (auto idr = dynamic_cast<AST::IdentifierExpr *>(bin->right.get()))
                                if (var_types.count(idr->name.lexeme))
                                    right_type = var_types[idr->name.lexeme];
                            if (!left_type.empty())
                                new_type = left_type;
                            else if (!right_type.empty())
                                new_type = right_type;
                        }
                    }
                    if (!new_type.empty())
                    {
                        var_types[name] = new_type;
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