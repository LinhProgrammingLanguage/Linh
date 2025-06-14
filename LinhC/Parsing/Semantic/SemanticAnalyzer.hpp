#pragma once
#include "../AST/ASTNode.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <stack>

namespace Linh
{
    namespace Semantic
    {

        struct SemanticError
        {
            std::string message;
            int line;
            int column;
            SemanticError(std::string msg, int l, int c)
                : message(std::move(msg)), line(l), column(c) {}
        };

        class SemanticAnalyzer : public AST::StmtVisitor, public AST::ExprVisitor
        {
        public:
            std::vector<SemanticError> errors;

            void analyze(const AST::StmtList &stmts, bool reset_state = true);

            // StmtVisitor overrides
            void visitExpressionStmt(AST::ExpressionStmt *stmt) override;
            void visitPrintStmt(AST::PrintStmt *stmt) override;
            void visitVarDeclStmt(AST::VarDeclStmt *stmt) override;
            void visitBlockStmt(AST::BlockStmt *stmt) override;
            void visitIfStmt(AST::IfStmt *stmt) override;
            void visitWhileStmt(AST::WhileStmt *stmt) override;
            void visitDoWhileStmt(AST::DoWhileStmt *stmt) override;
            void visitFunctionDeclStmt(AST::FunctionDeclStmt *stmt) override;
            void visitReturnStmt(AST::ReturnStmt *stmt) override;
            void visitBreakStmt(AST::BreakStmt *stmt) override;
            void visitContinueStmt(AST::ContinueStmt *stmt) override;
            void visitSwitchStmt(AST::SwitchStmt *stmt) override;
            void visitDeleteStmt(AST::DeleteStmt *stmt) override;
            void visitThrowStmt(AST::ThrowStmt *stmt) override;
            void visitTryStmt(AST::TryStmt *stmt) override;

            // ExprVisitor overrides (only need UninitLiteralExpr for this rule)
            std::any visitBinaryExpr(AST::BinaryExpr *expr) override;
            std::any visitUnaryExpr(AST::UnaryExpr *expr) override;
            std::any visitLiteralExpr(AST::LiteralExpr *expr) override;
            std::any visitGroupingExpr(AST::GroupingExpr *expr) override;
            std::any visitIdentifierExpr(AST::IdentifierExpr *expr) override;
            std::any visitAssignmentExpr(AST::AssignmentExpr *expr) override;
            std::any visitLogicalExpr(AST::LogicalExpr *expr) override;
            std::any visitCallExpr(AST::CallExpr *expr) override;
            std::any visitPostfixExpr(AST::PostfixExpr *expr) override;
            std::any visitUninitLiteralExpr(AST::UninitLiteralExpr *expr) override;
            std::any visitNewExpr(AST::NewExpr *expr) override;
            std::any visitThisExpr(AST::ThisExpr *expr) override;
            std::any visitArrayLiteralExpr(AST::ArrayLiteralExpr *expr) override;
            std::any visitMapLiteralExpr(AST::MapLiteralExpr *expr) override;
            std::any visitSubscriptExpr(AST::SubscriptExpr *expr) override;
            std::any visitInterpolatedStringExpr(AST::InterpolatedStringExpr *expr) override;

            const std::vector<SemanticError> &get_errors() const;

        private:
            bool is_uninit_type(const std::optional<AST::TypeNodePtr> &type);
            bool is_uninit_expr(const AST::ExprPtr &expr);

            // --- Quản lý scope ---
            std::vector<std::unordered_map<std::string, bool>> var_scopes;
            std::unordered_map<std::string, bool> global_functions;

            // --- Bổ sung quản lý loại biến, kiểu biến, số lượng tham số hàm ---
            std::unordered_map<std::string, std::string> var_types;        // tên biến -> kiểu
            std::unordered_map<std::string, std::string> var_kinds;        // tên biến -> "vas"/"const"/"var"
            std::unordered_map<std::string, size_t> function_param_counts; // tên hàm -> số lượng tham số

            // --- Bổ sung quản lý str_limit cho từng biến kiểu str<index> ---
            std::unordered_map<std::string, int> var_str_limit; // tên biến -> str_limit

            void begin_scope();
            void end_scope();
            void declare_var(const std::string &name, const std::string &kind = "", const std::string &type = "");
            bool is_var_declared(const std::string &name);
            void declare_function(const std::string &name, size_t param_count = 0);
            bool is_function_declared(const std::string &name);

            // Helper: xác định kiểu literal cho Linh từ LiteralExpr
            static std::string get_linh_literal_type(const AST::LiteralExpr *lit);
        };

    } // namespace Semantic
} // namespace Linh
