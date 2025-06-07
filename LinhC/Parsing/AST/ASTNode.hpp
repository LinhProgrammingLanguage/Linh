#ifndef LINH_AST_NODE_HPP
#define LINH_AST_NODE_HPP

#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <any>
#include <optional>
#include "../Lexer/Lexer.hpp"

namespace Linh
{
    namespace AST
    {
        // --- Type System Nodes (Giữ nguyên từ lần cập nhật trước) ---
        struct TypeNode;
        struct BaseTypeNode;
        struct SizedIntegerTypeNode;
        struct SizedFloatTypeNode;
        struct MapTypeNode;
        struct ArrayTypeNode;
        struct UnionTypeNode;
        using TypeNodePtr = std::unique_ptr<TypeNode>;

        class TypeVisitor
        {
        public:
            virtual ~TypeVisitor() = default;
            virtual std::string visitBaseTypeNode(BaseTypeNode *type_node) = 0;
            virtual std::string visitSizedIntegerTypeNode(SizedIntegerTypeNode *type_node) = 0;
            virtual std::string visitSizedFloatTypeNode(SizedFloatTypeNode *type_node) = 0;
            virtual std::string visitMapTypeNode(MapTypeNode *type_node) = 0;
            virtual std::string visitArrayTypeNode(ArrayTypeNode *type_node) = 0;
            virtual std::string visitUnionTypeNode(UnionTypeNode *type_node) = 0;
        };
        struct TypeNode
        {
            virtual ~TypeNode() = default;
            virtual std::string accept(TypeVisitor *visitor) = 0;
        };
        struct BaseTypeNode : TypeNode
        {
            Token type_keyword_token;
            std::optional<int> template_arg; // <--- thêm dòng này
            BaseTypeNode(Token token) : type_keyword_token(std::move(token)), template_arg(std::nullopt) {}
            std::string accept(TypeVisitor *visitor) override { return visitor->visitBaseTypeNode(this); }
        };
        struct SizedIntegerTypeNode : TypeNode
        {
            Token base_type_keyword_token;
            Token size_token;
            std::optional<int> template_arg; // <--- thêm dòng này
            SizedIntegerTypeNode(Token base_token, Token size_tok)
                : base_type_keyword_token(std::move(base_token)), size_token(std::move(size_tok))
            {
                try
                {
                    template_arg = std::stoi(size_token.lexeme);
                }
                catch (...)
                {
                    template_arg = std::nullopt;
                }
            }
            std::string accept(TypeVisitor *visitor) override { return visitor->visitSizedIntegerTypeNode(this); }
        };
        struct SizedFloatTypeNode : TypeNode
        {
            Token base_type_keyword_token;
            Token size_token;
            std::optional<int> template_arg; // <--- thêm dòng này
            SizedFloatTypeNode(Token base_token, Token size_tok)
                : base_type_keyword_token(std::move(base_token)), size_token(std::move(size_tok))
            {
                try
                {
                    template_arg = std::stoi(size_token.lexeme);
                }
                catch (...)
                {
                    template_arg = std::nullopt;
                }
            }
            std::string accept(TypeVisitor *visitor) override { return visitor->visitSizedFloatTypeNode(this); }
        };
        struct MapTypeNode : TypeNode
        {
            Token map_keyword_token;
            TypeNodePtr key_type;
            TypeNodePtr value_type;
            MapTypeNode(Token map_kw_tok, TypeNodePtr k_type, TypeNodePtr v_type)
                : map_keyword_token(std::move(map_kw_tok)), key_type(std::move(k_type)), value_type(std::move(v_type)) {}
            std::string accept(TypeVisitor *visitor) override { return visitor->visitMapTypeNode(this); }
        };
        struct ArrayTypeNode : TypeNode
        {
            std::optional<Token> array_keyword_token;
            TypeNodePtr element_type;
            Token r_bracket_token;
            ArrayTypeNode(TypeNodePtr el_type, Token r_bracket)
                : array_keyword_token(std::nullopt), element_type(std::move(el_type)), r_bracket_token(std::move(r_bracket)) {}
            ArrayTypeNode(Token arr_kw_tok, TypeNodePtr el_type_for_array_kw, Token dummy_r_bracket_for_consistency)
                : array_keyword_token(std::move(arr_kw_tok)), element_type(std::move(el_type_for_array_kw)), r_bracket_token(std::move(dummy_r_bracket_for_consistency)) {}
            std::string accept(TypeVisitor *visitor) override { return visitor->visitArrayTypeNode(this); }
        };
        struct UnionTypeNode : TypeNode
        {
            std::vector<TypeNodePtr> types;
            Token l_angle_token, r_angle_token;
            UnionTypeNode(Token l_angle, std::vector<TypeNodePtr> t_list, Token r_angle)
                : types(std::move(t_list)), l_angle_token(std::move(l_angle)), r_angle_token(std::move(r_angle)) {}
            std::string accept(TypeVisitor *visitor) override { return visitor->visitUnionTypeNode(this); }
        };

        // --- ExprVisitor and Expr Nodes ---
        struct BinaryExpr;
        struct UnaryExpr;
        struct LiteralExpr;
        struct GroupingExpr;
        struct IdentifierExpr;
        struct AssignmentExpr;
        struct LogicalExpr;
        struct CallExpr;
        struct PostfixExpr;
        struct UninitLiteralExpr;
        struct NewExpr;
        struct ThisExpr;
        struct ArrayLiteralExpr;       // MỚI
        struct MapLiteralExpr;         // MỚI
        struct SubscriptExpr;          // MỚI
        struct InterpolatedStringExpr; // MỚI

        class ExprVisitor
        {
        public:
            virtual ~ExprVisitor() = default;
            virtual std::any visitBinaryExpr(BinaryExpr *expr) = 0;
            virtual std::any visitUnaryExpr(UnaryExpr *expr) = 0;
            virtual std::any visitLiteralExpr(LiteralExpr *expr) = 0;
            virtual std::any visitGroupingExpr(GroupingExpr *expr) = 0;
            virtual std::any visitIdentifierExpr(IdentifierExpr *expr) = 0;
            virtual std::any visitAssignmentExpr(AssignmentExpr *expr) = 0;
            virtual std::any visitLogicalExpr(LogicalExpr *expr) = 0;
            virtual std::any visitCallExpr(CallExpr *expr) = 0;
            virtual std::any visitPostfixExpr(PostfixExpr *expr) = 0;
            virtual std::any visitUninitLiteralExpr(UninitLiteralExpr *expr) = 0;
            virtual std::any visitNewExpr(NewExpr *expr) = 0;
            virtual std::any visitThisExpr(ThisExpr *expr) = 0;
            virtual std::any visitArrayLiteralExpr(ArrayLiteralExpr *expr) = 0;             // MỚI
            virtual std::any visitMapLiteralExpr(MapLiteralExpr *expr) = 0;                 // MỚI
            virtual std::any visitSubscriptExpr(SubscriptExpr *expr) = 0;                   // MỚI
            virtual std::any visitInterpolatedStringExpr(InterpolatedStringExpr *expr) = 0; // MỚI
        };
        struct Expr
        {
            virtual ~Expr() = default;
            virtual std::any accept(ExprVisitor *visitor) = 0;
        };
        using ExprPtr = std::unique_ptr<Expr>;

        struct LiteralExpr : Expr
        {
            LiteralValue value;
            LiteralExpr(LiteralValue val) : value(std::move(val)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitLiteralExpr(this); }
        };
        struct IdentifierExpr : Expr
        {
            Token name;
            IdentifierExpr(Token n) : name(std::move(n)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitIdentifierExpr(this); }
        };
        struct UnaryExpr : Expr
        {
            Token op;
            ExprPtr right;
            UnaryExpr(Token o, ExprPtr r) : op(std::move(o)), right(std::move(r)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitUnaryExpr(this); }
        };
        struct PostfixExpr : Expr
        {
            ExprPtr operand;
            Token op_token;
            PostfixExpr(ExprPtr left, Token op_tok) : operand(std::move(left)), op_token(std::move(op_tok)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitPostfixExpr(this); }
        };
        struct BinaryExpr : Expr
        {
            ExprPtr left;
            Token op;
            ExprPtr right;
            BinaryExpr(ExprPtr l, Token o, ExprPtr r) : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitBinaryExpr(this); }
        };
        struct LogicalExpr : Expr
        {
            ExprPtr left;
            Token op;
            ExprPtr right;
            LogicalExpr(ExprPtr l, Token o, ExprPtr r) : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitLogicalExpr(this); }
        };
        struct GroupingExpr : Expr
        {
            ExprPtr expression;
            GroupingExpr(ExprPtr expr) : expression(std::move(expr)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitGroupingExpr(this); }
        };
        struct AssignmentExpr : Expr
        {
            Token name;
            ExprPtr value; // Sẽ cần cập nhật target của assignment sau này
            AssignmentExpr(Token n, ExprPtr val) : name(std::move(n)), value(std::move(val)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitAssignmentExpr(this); }
        };
        struct CallExpr : Expr
        {
            ExprPtr callee;
            Token paren;
            std::vector<ExprPtr> arguments;
            CallExpr(ExprPtr callee_expr, Token p, std::vector<ExprPtr> args) : callee(std::move(callee_expr)), paren(std::move(p)), arguments(std::move(args)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitCallExpr(this); }
        };
        struct UninitLiteralExpr : Expr
        {
            Token keyword;
            UninitLiteralExpr(Token kw) : keyword(std::move(kw)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitUninitLiteralExpr(this); }
        };
        struct NewExpr : Expr
        {
            Token keyword_new;
            ExprPtr class_constructor_call;
            NewExpr(Token kw, ExprPtr constructor_call) : keyword_new(std::move(kw)), class_constructor_call(std::move(constructor_call)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitNewExpr(this); }
        };
        struct ThisExpr : Expr
        {
            Token keyword_this;
            ThisExpr(Token kw) : keyword_this(std::move(kw)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitThisExpr(this); }
        };
        struct ArrayLiteralExpr : Expr
        {
            Token l_bracket, r_bracket;
            std::vector<ExprPtr> elements;
            ArrayLiteralExpr(Token lb, std::vector<ExprPtr> elems, Token rb)
                : l_bracket(std::move(lb)), elements(std::move(elems)), r_bracket(std::move(rb)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitArrayLiteralExpr(this); }
        };
        struct MapEntryNode
        {
            ExprPtr key;
            ExprPtr value;
            Token colon_token;
            MapEntryNode(ExprPtr k, Token colon, ExprPtr v)
                : key(std::move(k)), colon_token(std::move(colon)), value(std::move(v)) {}
        };
        struct MapLiteralExpr : Expr
        {
            Token l_brace, r_brace;
            std::vector<MapEntryNode> entries;
            MapLiteralExpr(Token lb, std::vector<MapEntryNode> kvs, Token rb)
                : l_brace(std::move(lb)), entries(std::move(kvs)), r_brace(std::move(rb)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitMapLiteralExpr(this); }
        };
        struct SubscriptExpr : Expr // MỚI
        {
            ExprPtr object;
            Token l_bracket;
            ExprPtr index;
            Token r_bracket;
            SubscriptExpr(ExprPtr obj, Token lb, ExprPtr idx, Token rb)
                : object(std::move(obj)), l_bracket(std::move(lb)), index(std::move(idx)), r_bracket(std::move(rb)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitSubscriptExpr(this); }
        };
        struct InterpolatedStringExpr : Expr // MỚI
        {
            std::vector<std::variant<std::string, ExprPtr>> parts;
            InterpolatedStringExpr(std::vector<std::variant<std::string, ExprPtr>> p)
                : parts(std::move(p)) {}
            std::any accept(ExprVisitor *visitor) override { return visitor->visitInterpolatedStringExpr(this); }
        };

        // --- Stmt Nodes (Giữ nguyên từ lần cập nhật trước) ---
        struct ExpressionStmt;
        struct PrintStmt;
        struct VarDeclStmt;
        struct BlockStmt;
        struct IfStmt;
        struct WhileStmt;
        struct FunctionDeclStmt;
        struct ReturnStmt;
        struct BreakStmt;
        struct ContinueStmt;
        struct SwitchStmt;
        struct DoWhileStmt;
        struct DeleteStmt;
        struct ThrowStmt;
        struct TryStmt;

        class StmtVisitor
        {
        public:
            virtual ~StmtVisitor() = default;
            virtual void visitExpressionStmt(ExpressionStmt *stmt) = 0;
            virtual void visitPrintStmt(PrintStmt *stmt) = 0;
            virtual void visitVarDeclStmt(VarDeclStmt *stmt) = 0;
            virtual void visitBlockStmt(BlockStmt *stmt) = 0;
            virtual void visitIfStmt(IfStmt *stmt) = 0;
            virtual void visitWhileStmt(WhileStmt *stmt) = 0;
            virtual void visitFunctionDeclStmt(FunctionDeclStmt *stmt) = 0;
            virtual void visitReturnStmt(ReturnStmt *stmt) = 0;
            virtual void visitBreakStmt(BreakStmt *stmt) = 0;
            virtual void visitContinueStmt(ContinueStmt *stmt) = 0;
            virtual void visitSwitchStmt(SwitchStmt *stmt) = 0;
            virtual void visitDoWhileStmt(DoWhileStmt *stmt) = 0;
            virtual void visitDeleteStmt(DeleteStmt *stmt) = 0;
            virtual void visitThrowStmt(ThrowStmt *stmt) = 0;
            virtual void visitTryStmt(TryStmt *stmt) = 0;
        };
        struct Stmt
        {
            virtual ~Stmt() = default;
            virtual void accept(StmtVisitor *visitor) = 0;
        };
        using StmtPtr = std::unique_ptr<Stmt>;
        using StmtList = std::vector<StmtPtr>;

        struct ExpressionStmt : Stmt
        {
            ExprPtr expression;
            ExpressionStmt(ExprPtr expr) : expression(std::move(expr)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitExpressionStmt(this); }
        };
        struct PrintStmt : Stmt
        {
            Token keyword;
            ExprPtr expression;
            PrintStmt(Token kw, ExprPtr expr) : keyword(std::move(kw)), expression(std::move(expr)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitPrintStmt(this); }
        };

        struct VarDeclStmt : Stmt
        {
            Token keyword;
            Token name;
            std::optional<TypeNodePtr> declared_type;
            ExprPtr initializer;

            VarDeclStmt(Token kw, Token n, std::optional<TypeNodePtr> type, ExprPtr init)
                : keyword(std::move(kw)), name(std::move(n)), declared_type(std::move(type)), initializer(std::move(init)) {}

            void accept(StmtVisitor *visitor) override { visitor->visitVarDeclStmt(this); }
        };

        struct FuncParamNode
        {
            Token name;
            std::optional<TypeNodePtr> type;

            FuncParamNode(Token param_name, std::optional<TypeNodePtr> param_type = std::nullopt)
                : name(std::move(param_name)), type(std::move(param_type)) {}
        };

        struct BlockStmt : Stmt
        {
            StmtList statements;
            Token opening_brace;
            BlockStmt(StmtList stmts, Token brace) : statements(std::move(stmts)), opening_brace(std::move(brace)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitBlockStmt(this); }
        };
        struct IfStmt : Stmt
        {
            Token keyword_if;
            ExprPtr condition;
            StmtPtr then_branch;
            StmtPtr else_branch;
            IfStmt(Token kw, ExprPtr cond, StmtPtr then_b, StmtPtr else_b) : keyword_if(std::move(kw)), condition(std::move(cond)), then_branch(std::move(then_b)), else_branch(std::move(else_b)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitIfStmt(this); }
        };
        struct WhileStmt : Stmt
        {
            Token keyword_while;
            ExprPtr condition;
            StmtPtr body;
            WhileStmt(Token kw, ExprPtr cond, StmtPtr b) : keyword_while(std::move(kw)), condition(std::move(cond)), body(std::move(b)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitWhileStmt(this); }
        };
        struct DoWhileStmt : Stmt
        {
            Token keyword_do;
            StmtPtr body;
            Token keyword_while;
            ExprPtr condition;
            DoWhileStmt(Token kw_do, StmtPtr b, Token kw_while, ExprPtr cond) : keyword_do(std::move(kw_do)), body(std::move(b)), keyword_while(std::move(kw_while)), condition(std::move(cond)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitDoWhileStmt(this); }
        };

        struct FunctionDeclStmt : Stmt
        {
            Token keyword_func;
            Token name;
            std::vector<FuncParamNode> params;
            std::optional<TypeNodePtr> return_type;
            std::unique_ptr<BlockStmt> body;

            FunctionDeclStmt(Token kw, Token n, std::vector<FuncParamNode> p_nodes,
                             std::optional<TypeNodePtr> ret_type, std::unique_ptr<BlockStmt> b)
                : keyword_func(std::move(kw)), name(std::move(n)), params(std::move(p_nodes)),
                  return_type(std::move(ret_type)), body(std::move(b)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitFunctionDeclStmt(this); }
        };

        struct ReturnStmt : Stmt
        {
            Token keyword_return;
            ExprPtr value;
            ReturnStmt(Token kw, ExprPtr val) : keyword_return(std::move(kw)), value(std::move(val)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitReturnStmt(this); }
        };
        struct BreakStmt : Stmt
        {
            Token keyword;
            BreakStmt(Token kw) : keyword(std::move(kw)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitBreakStmt(this); }
        };
        struct ContinueStmt : Stmt
        {
            Token keyword;
            ContinueStmt(Token kw) : keyword(std::move(kw)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitContinueStmt(this); }
        };

        struct CaseClause
        {
            std::optional<ExprPtr> case_value;
            bool is_default;
            StmtList statements;
            Token keyword_token;
            Token colon_token;

            CaseClause(ExprPtr val, StmtList stmts_list, Token kw_tok, Token colon_tok)
                : case_value(std::move(val)), is_default(false),
                  statements(std::move(stmts_list)),
                  keyword_token(std::move(kw_tok)),
                  colon_token(std::move(colon_tok))
            {
            }

            CaseClause(StmtList stmts_list, Token kw_tok, Token colon_tok)
                : case_value(std::nullopt), is_default(true),
                  statements(std::move(stmts_list)),
                  keyword_token(std::move(kw_tok)),
                  colon_token(std::move(colon_tok))
            {
            }
        };
        struct SwitchStmt : Stmt
        {
            Token keyword_switch;
            ExprPtr expression_to_switch_on;
            std::vector<CaseClause> cases;
            Token opening_brace;
            SwitchStmt(Token kw, ExprPtr expr, std::vector<CaseClause> cls, Token brace) : keyword_switch(std::move(kw)), expression_to_switch_on(std::move(expr)), cases(std::move(cls)), opening_brace(std::move(brace)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitSwitchStmt(this); }
        };
        struct DeleteStmt : Stmt
        {
            Token keyword_delete;
            ExprPtr expression_to_delete;
            DeleteStmt(Token kw, ExprPtr expr) : keyword_delete(std::move(kw)), expression_to_delete(std::move(expr)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitDeleteStmt(this); }
        };
        struct ThrowStmt : Stmt
        {
            Token keyword;
            ExprPtr expression;
            ThrowStmt(Token kw, ExprPtr expr) : keyword(std::move(kw)), expression(std::move(expr)) {}
            void accept(StmtVisitor *visitor) override { visitor->visitThrowStmt(this); }
        };

        struct CatchClauseNode
        {
            Token keyword_catch;
            std::optional<Token> exception_variable;
            std::unique_ptr<BlockStmt> body;
            CatchClauseNode(Token kw, Token var_name, std::unique_ptr<BlockStmt> b) : keyword_catch(std::move(kw)), exception_variable(std::move(var_name)), body(std::move(b)) {}
            CatchClauseNode(Token kw, std::unique_ptr<BlockStmt> b) : keyword_catch(std::move(kw)), exception_variable(std::nullopt), body(std::move(b)) {}
        };
        struct TryStmt : Stmt
        {
            Token keyword_try;
            std::unique_ptr<BlockStmt> try_block;
            std::vector<CatchClauseNode> catch_clauses;
            std::optional<std::unique_ptr<BlockStmt>> finally_block;
            std::optional<Token> keyword_finally;
            TryStmt(Token kw_try, std::unique_ptr<BlockStmt> try_b, std::vector<CatchClauseNode> catches, std::optional<std::unique_ptr<BlockStmt>> finally_b = std::nullopt, std::optional<Token> kw_finally_opt = std::nullopt) : keyword_try(std::move(kw_try)), try_block(std::move(try_b)), catch_clauses(std::move(catches)), finally_block(std::move(finally_b)), keyword_finally(kw_finally_opt) {}
            void accept(StmtVisitor *visitor) override { visitor->visitTryStmt(this); }
        };

    } // namespace AST
} // namespace Linh
#endif // LINH_AST_NODE_HPP