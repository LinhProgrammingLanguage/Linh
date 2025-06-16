#pragma once
#include "../Parsing/AST/ASTNode.hpp"
#include "Bytecode.hpp"
#include <unordered_map>
#include <string>

namespace Linh
{
    class BytecodeEmitter : public AST::ExprVisitor, public AST::StmtVisitor
    {
    public:
        struct FunctionInfo
        {
            BytecodeChunk code;
            std::vector<std::string> param_names;
        };

        BytecodeEmitter();
        void emit(const AST::StmtList &stmts);
        const BytecodeChunk &get_chunk() const { return chunk; }

        // Getter for function table
        const std::unordered_map<std::string, FunctionInfo> &get_functions() const { return functions; }
        std::unordered_map<std::string, FunctionInfo> &get_functions() { return functions; } // <--- Thêm dòng này

        // ExprVisitor
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

        // StmtVisitor
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
        void visitImportStmt(AST::ImportStmt *stmt) override;

    private:
        BytecodeChunk chunk;
        std::unordered_map<std::string, int> var_table; // tên biến -> index (giản lược)
        int next_var_index = 0;

        // --- Add for function support ---
        std::unordered_map<std::string, FunctionInfo> functions;
        // -------------------------------

        int get_var_index(const std::string &name);
        void emit_instr(OpCode op, BytecodeValue val = {}, int line = 0, int col = 0);
    };
}
