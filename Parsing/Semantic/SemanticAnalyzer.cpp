#include "SemanticAnalyzer.hpp"
#include <iostream> // For error reporting
#include <utility>  // For std::move (though not explicitly used in SymbolInfo ctor anymore after previous edits, good include)

namespace Linh
{
    namespace Semantic
    {

        // --- SymbolInfo Implementation (nếu có, thường là struct đơn giản nên không cần file .cpp riêng) ---
        // SymbolInfo::SymbolInfo(Token token, bool constant, bool initialized)
        //     : declaration_token(std::move(token)), is_initialized(initialized), is_constant(constant) {}

        // --- Environment Implementation ---
        Environment::Environment()
        {
            begin_scope(); // Create the global scope upon construction
        }

        void Environment::define(const std::string &name, const Token &declaration_token, bool is_constant, bool is_initialized)
        {
            if (m_scopes.empty())
            {
                // This should ideally not be reached if the constructor always calls begin_scope()
                throw std::logic_error("Environment::define called with no active scope. Global scope missing?");
            }

            // Check if already defined in the current scope (m_scopes.back())
            std::map<std::string, SymbolInfo> &current_scope = m_scopes.back();
            if (current_scope.count(name))
            {
                // Error: Variable already declared in this scope.
                // This specific error message might be better handled by the caller (SemanticAnalyzer)
                // which has the token for the re-declaration attempt.
                // For now, Environment throws a generic one.
                throw std::runtime_error("Variable '" + name + "' already declared in the current scope.");
            }
            current_scope.emplace(name, SymbolInfo(declaration_token, is_constant, is_initialized));
        }

        // Helper to find a symbol (const version)
        const SymbolInfo *Environment::find_symbol(const std::string &name) const
        {
            // Iterate from current scope (end of vector) outwards to global scope (beginning of vector)
            for (auto it_scope = m_scopes.rbegin(); it_scope != m_scopes.rend(); ++it_scope)
            {
                const auto &current_scope_map = *it_scope;
                auto it_sym = current_scope_map.find(name);
                if (it_sym != current_scope_map.end())
                {
                    return &(it_sym->second); // Return pointer to the found SymbolInfo
                }
            }
            return nullptr; // Not found
        }

        // Helper to find a symbol (modifiable version)
        SymbolInfo *Environment::find_symbol_modifiable(const std::string &name)
        {
            // Iterate from current scope (end of vector) outwards to global scope (beginning of vector)
            // We need to iterate using indices to get a non-const reference to the map
            // and then a non-const pointer to the SymbolInfo.
            for (size_t i = 0; i < m_scopes.size(); ++i)
            {
                size_t reverse_idx = m_scopes.size() - 1 - i; // m_scopes.back() is at m_scopes.size() - 1
                std::map<std::string, SymbolInfo> &current_scope_map = m_scopes[reverse_idx];
                auto it_sym = current_scope_map.find(name);
                if (it_sym != current_scope_map.end())
                {
                    return &(it_sym->second); // Return modifiable pointer
                }
            }
            return nullptr; // Not found
        }

        const SymbolInfo &Environment::get(const Token &name_token) const
        {
            const SymbolInfo *symbol = find_symbol(name_token.lexeme);
            if (!symbol)
            {
                // Let SemanticAnalyzer handle the specific error message using its error() method
                throw std::runtime_error("Variable '" + name_token.lexeme + "' not declared.");
            }
            return *symbol;
        }

        void Environment::assign(const Token &name_token, bool is_reassigning_with_value)
        {
            SymbolInfo *symbol = find_symbol_modifiable(name_token.lexeme);
            if (!symbol)
            {
                throw std::runtime_error("Variable '" + name_token.lexeme + "' not declared before assignment.");
            }
            if (symbol->is_constant)
            {
                throw std::runtime_error("Cannot assign to constant variable '" + name_token.lexeme + "'.");
            }
            if (is_reassigning_with_value)
            { // i.e., an actual assignment like x = 10, not just passing by ref
                symbol->is_initialized = true;
            }
            // TODO: Type checking will happen here or in SemanticAnalyzer after getting types.
        }

        void Environment::begin_scope()
        {
            m_scopes.emplace_back(); // Add a new empty scope (map) to the end of the vector
        }

        void Environment::end_scope()
        {
            if (m_scopes.empty())
            {
                // This would indicate a bug in scope management, like too many end_scope calls.
                throw std::logic_error("Environment::end_scope called on an empty scope stack.");
            }
            if (m_scopes.size() == 1 && m_scopes.front().empty())
            {
                // This check is a bit fragile. The idea is to not pop the initial global scope
                // if it was just created and is empty, potentially for reuse.
                // However, typical usage is one SemanticAnalyzer per full analysis, so popping is fine.
                // If Environment is intended for long-lived use, this needs more thought.
                // For now, always pop.
            }
            m_scopes.pop_back(); // Remove the current scope (last map in the vector)
        }

        // --- SemanticAnalyzer Implementation ---
        SemanticAnalyzer::SemanticAnalyzer()
        {
            // The m_environment member is default-initialized,
            // and its constructor calls begin_scope(), creating the global scope.
        }

        void SemanticAnalyzer::analyze(const AST::StmtList &statements)
        {
            m_had_error = false; // Reset for this analysis run

            // The global scope is already pushed by m_environment's constructor.
            try
            {
                for (const auto &stmt : statements)
                {
                    resolve_statement(stmt);
                }
            }
            catch (const std::runtime_error & /* semantic_error_exception */)
            {
                // The error message is already printed by SemanticAnalyzer::error()
                // or directly by Environment if it threw one that wasn't caught and re-wrapped.
                // The exception is used to halt analysis. m_had_error is set by error().
            }
            // After analysis, the global scope might remain in m_environment.
            // If SemanticAnalyzer instances are one-shot, this is fine.
            // If reused, m_environment might need to be reset or scopes explicitly managed.
        }

        void SemanticAnalyzer::error(const Token &token, const std::string &message)
        {
            m_had_error = true;
            std::cerr << "[L" << token.line << ":" << token.column << "] Semantic Error";
            if (!token.lexeme.empty() && token.type != TokenType::END_OF_FILE)
            { // Avoid printing lexeme for EOF
                std::cerr << " at '" << token.lexeme << "'";
            }
            std::cerr << ": " << message << std::endl;
            throw std::runtime_error("Semantic error occurred (halting analysis)."); // Throw to stop analysis
        }

        void SemanticAnalyzer::resolve_statement(const AST::StmtPtr &stmt)
        {
            if (!stmt)
                return; // Should ideally not happen with a correct parser
            visit_stmt(stmt);
        }

        void SemanticAnalyzer::resolve_expression(const AST::ExprPtr &expr)
        {
            if (!expr)
                return; // Should ideally not happen
            visit_expr(expr);
        }

        void SemanticAnalyzer::visit_stmt(const AST::StmtPtr &stmt_node)
        {
            std::visit([this](auto &&arg)
                       {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, AST::ExpressionStmt>) this->visit_expression_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::PrintStmt>) this->visit_print_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::VarDeclStmt>) this->visit_var_decl_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::BlockStmt>) this->visit_block_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::IfStmt>) this->visit_if_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::WhileStmt>) this->visit_while_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::FuncDeclStmt>) this->visit_func_decl_stmt(arg);
                else if constexpr (std::is_same_v<T, AST::ReturnStmt>) this->visit_return_stmt(arg);
                else { /* Should not happen if variant is exhaustive for parsed StmtNode types */ } }, *stmt_node);
        }

        void SemanticAnalyzer::visit_expression_stmt(const AST::ExpressionStmt &stmt)
        {
            resolve_expression(stmt.expression);
        }

        void SemanticAnalyzer::visit_print_stmt(const AST::PrintStmt &stmt)
        {
            for (const auto &expr : stmt.expressions)
            {
                resolve_expression(expr);
            }
        }

        void SemanticAnalyzer::visit_var_decl_stmt(const AST::VarDeclStmt &stmt)
        {
            // 1. Resolve initializer first (if any). It's evaluated in the current scope
            //    before the new variable is defined in it.
            if (stmt.initializer)
            {
                resolve_expression(*stmt.initializer);
                // TODO: Get the type of the initializer here for type checking.
            }

            // 2. Define the variable in the current scope.
            bool is_const = (stmt.keyword.type == TokenType::CONST);
            bool is_initialized_by_expr = stmt.initializer.has_value();
            bool is_initialized_by_type_default = false; // For `const X: int;`

            if (!is_initialized_by_expr && stmt.type_annotation)
            {
                // According to Linh spec: "const IS_ACTIVATED: bool;" -> IS_ACTIVATED = false
                // This implies a variable with a type annotation but no initializer is considered
                // initialized to its type's zero value.
                is_initialized_by_type_default = true;
            }
            bool final_is_initialized = is_initialized_by_expr || is_initialized_by_type_default;

            // Parser should have already caught "var x;" errors.
            // Parser should have also caught "const X;" (no type, no init).
            // We re-check for const here based on semantic initialization.
            if (is_const && !final_is_initialized)
            {
                error(stmt.name, "Constant '" + stmt.name.lexeme + "' must be initialized or have a type for default zero-value.");
                return; // Stop processing this faulty declaration
            }

            try
            {
                m_environment.define(stmt.name.lexeme, stmt.name, is_const, final_is_initialized);
            }
            catch (const std::runtime_error &e)
            {
                // Catch errors from Environment (like redeclaration) and use our error reporting.
                error(stmt.name, e.what()); // e.what() will contain the message from Environment
                return;                     // Stop if variable couldn't be defined
            }

            // TODO: Type checking:
            // 1. If stmt.type_annotation exists, resolve it to an internal Type representation.
            // 2. If stmt.initializer exists, determine its type from resolve_expression.
            // 3. Check compatibility:
            //    - If both exist: initializer type must be assignable to annotation type.
            //    - If only initializer: variable gets type of initializer.
            //    - If only annotation: variable gets annotation type.
            // 4. Store the final resolved type in SymbolInfo (will need to modify Environment::define or add a setType method).
        }

        void SemanticAnalyzer::visit_block_stmt(const AST::BlockStmt &stmt)
        {
            m_environment.begin_scope();
            for (const auto &s : stmt.statements)
            {
                resolve_statement(s);
            }
            m_environment.end_scope();
        }

        void SemanticAnalyzer::visit_if_stmt(const AST::IfStmt &stmt)
        {
            resolve_expression(stmt.condition);
            // TODO: Type check: stmt.condition's resolved type must be boolean.

            resolve_statement(stmt.then_branch);
            if (stmt.else_branch)
            {
                resolve_statement(*stmt.else_branch);
            }
        }

        void SemanticAnalyzer::visit_while_stmt(const AST::WhileStmt &stmt)
        {
            resolve_expression(stmt.condition);
            // TODO: Type check: stmt.condition's resolved type must be boolean.
            resolve_statement(stmt.body);
        }

        void SemanticAnalyzer::visit_func_decl_stmt(const AST::FuncDeclStmt &stmt)
        {
            // TODO: Proper function handling:
            // 1. Define the function in the current (enclosing) scope. Its SymbolInfo would mark it as a function,
            //    store parameter types, return type.
            //    m_environment.define(stmt.name.lexeme, stmt.name, true /*is_const? functions are usually const*/, true);
            //
            // 2. Begin a new scope for the function parameters and body.
            //    m_environment.begin_scope();
            //
            // 3. For each parameter in stmt.params:
            //    - Resolve its type_annotation.
            //    - Define the parameter in the function's new scope.
            //      m_environment.define(param.name.lexeme, param.name, false, true /*params are initialized by call*/);
            //
            // 4. If stmt.return_type exists, resolve it. Store this as the function's expected return type.
            //
            // 5. Set a flag indicating we are inside this function (e.g., m_current_function_return_type).
            //
            // 6. Resolve the function body (stmt.body).
            //    resolve_statement(stmt.body);
            //
            // 7. Unset the "inside function" flag.
            //
            // 8. End the function's scope.
            //    m_environment.end_scope();

            // Minimalistic placeholder to allow body resolution:
            if (stmt.body)
            {                                // Body is StmtPtr, which should point to a BlockStmt for functions
                m_environment.begin_scope(); // Simplistic: create a scope for the body
                resolve_statement(stmt.body);
                m_environment.end_scope();
            }
        }

        void SemanticAnalyzer::visit_return_stmt(const AST::ReturnStmt &stmt)
        {
            // TODO: Check if currently inside a function.
            //       If not, error(stmt.keyword, "Return statement outside of a function.");
            //
            if (stmt.value)
            {
                resolve_expression(*stmt.value);
                // TODO: Get the type of *stmt.value.
                // TODO: Check if this type is compatible with the current function's declared return type.
            }
            else
            {
                // TODO: Check if the current function's declared return type is void (or compatible with returning nothing).
            }
        }

        // --- Expression Visitors ---
        void SemanticAnalyzer::visit_expr(const AST::ExprPtr &expr_node)
        {
            std::visit([this](auto &&arg)
                       {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, AST::LiteralExpr>) this->visit_literal_expr(arg);
                else if constexpr (std::is_same_v<T, AST::UnaryExpr>) this->visit_unary_expr(arg);
                else if constexpr (std::is_same_v<T, AST::BinaryExpr>) this->visit_binary_expr(arg);
                else if constexpr (std::is_same_v<T, AST::GroupingExpr>) this->visit_grouping_expr(arg);
                else if constexpr (std::is_same_v<T, AST::VariableExpr>) this->visit_variable_expr(arg);
                else if constexpr (std::is_same_v<T, AST::AssignExpr>) this->visit_assign_expr(arg);
                else if constexpr (std::is_same_v<T, AST::CallExpr>) this->visit_call_expr(arg);
                else { /* Should not happen if variant is exhaustive */ } }, *expr_node);
        }

        void SemanticAnalyzer::visit_literal_expr(const AST::LiteralExpr & /*expr*/)
        {
            // Literals are type-inferred.
            // TODO: When implementing full type checking, this visit method should determine and
            //       return/store the type of the literal (e.g., int, string, bool).
        }

        void SemanticAnalyzer::visit_unary_expr(const AST::UnaryExpr &expr)
        {
            resolve_expression(expr.right);
            // TODO: Type checking for unary operator.
            // Example: For '!', expr.right must be boolean. Result is boolean.
            //          For '-', expr.right must be numeric. Result is numeric of same type.
            // This method should determine and return/store the type of the UnaryExpr.
        }

        void SemanticAnalyzer::visit_binary_expr(const AST::BinaryExpr &expr)
        {
            resolve_expression(expr.left);
            resolve_expression(expr.right);
            // TODO: Type checking for binary operator.
            // Example: For '+', left and right must be numeric (or string for concatenation if supported).
            //          For '&&', '||', left and right must be boolean.
            //          For '==', '!=', types must be comparable.
            // This method should determine and return/store the type of the BinaryExpr.
        }

        void SemanticAnalyzer::visit_grouping_expr(const AST::GroupingExpr &expr)
        {
            resolve_expression(expr.expression);
            // TODO: The type of the GroupingExpr is the type of its inner expression.
        }

        void SemanticAnalyzer::visit_variable_expr(const AST::VariableExpr &expr)
        {
            try
            {
                const SymbolInfo &symbol = m_environment.get(expr.name); // Throws if not declared

                // Check for initialization (basic check, Linh's zero-value spec complicates this)
                // A variable `let x: int;` is considered "initialized" to 0.
                // SymbolInfo.is_initialized should reflect this based on declaration.
                if (!symbol.is_initialized)
                {
                    // This error might be too strict if zero-values are always assumed usable.
                    // error(expr.name, "Variable '" + expr.name.lexeme + "' might be used before initialization.");
                }
                // TODO: This method should determine and return/store the type of the variable
                //       (fetched from SymbolInfo.type after type resolution is implemented).
            }
            catch (const std::runtime_error &e)
            {
                // Error from m_environment.get() (e.g., not declared)
                error(expr.name, e.what()); // Use SemanticAnalyzer's error reporting
            }
        }

        void SemanticAnalyzer::visit_assign_expr(const AST::AssignExpr &expr)
        {
            // The target of assignment (expr.name) is resolved first to check if it's assignable.
            // SymbolInfo *target_symbol = nullptr; // For storing type later
            try
            {
                // This assign call also checks if const and marks as initialized.
                m_environment.assign(expr.name);
                // If we need the symbol info for type checking:
                // target_symbol = m_environment.find_symbol_modifiable(expr.name.lexeme);
            }
            catch (const std::runtime_error &e)
            {
                error(expr.name, e.what()); // e.g., not declared, or assigning to const
                return;                     // Don't process value if target is invalid
            }

            resolve_expression(expr.value);
            // TODO: Type checking:
            // 1. Get the type of the target variable (target_symbol->type).
            // 2. Get the type of the assigned value (expr.value's resolved type).
            // 3. Check if value_type is assignable to target_type.
            //    - For `let` and `const`, types must match (or be implicitly convertible).
            //    - For `var`, the type of the variable might change. Update SymbolInfo.type.
            // TODO: The type of an assignment expression itself is usually the type of the assigned value.
        }

        void SemanticAnalyzer::visit_call_expr(const AST::CallExpr &expr)
        {
            resolve_expression(expr.callee);
            // TODO: Check that expr.callee resolves to a function type.
            //       Get its parameter types and return type from its SymbolInfo.

            for (const auto &arg : expr.arguments)
            {
                resolve_expression(arg);
                // TODO: Get the type of each argument.
            }
            // TODO: Type check:
            // - Number of arguments must match number of parameters.
            // - Type of each argument must be assignable to type of corresponding parameter.
            // TODO: The type of the CallExpr is the return type of the function.
        }

    } // namespace Semantic
} // namespace Linh