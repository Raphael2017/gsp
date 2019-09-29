#ifndef GSP_SQL_EXPRESSION_H
#define GSP_SQL_EXPRESSION_H

#include <vector>
#include "sql_object.h"
#include <string.h>

namespace GSP {

    class AstExpr;
    class AstConstantValue;
    class AstColumnRef;
    class AstFuncCall;
    class AstCaseExpr;
    class AstExprList;
    class AstSelectStmt;
    typedef std::vector<AstExpr*> AstExprs;

    class AstExpr : public IObject {
    public:
        enum EXPR_TYPE {
            /* BINARY OP BEGIN */
            OR, AND, COMP_LE, COMP_LT, COMP_GE, COMP_GT, COMP_EQ, COMP_NEQ,
            COMP_LE_ALL, COMP_LT_ALL, COMP_GE_ALL, COMP_GT_ALL, COMP_EQ_ALL, COMP_NEQ_ALL,
            COMP_LE_SOME, COMP_LT_SOME, COMP_GE_SOME, COMP_GT_SOME, COMP_EQ_SOME, COMP_NEQ_SOME,
            COMP_LE_ANY, COMP_LT_ANY, COMP_GE_ANY, COMP_GT_ANY, COMP_EQ_ANY, COMP_NEQ_ANY,

            PLUS, MINUS, BARBAR, MUL, DIV, REM, MOD, CARET,
            /* UNARY */
            NOT, IS_TRUE, IS_NOT_TRUE, IS_FALSE, IS_NOT_FALSE, IS_UNKNOWN, IS_NOT_UNKNOWN, IS_NULL, IS_NOT_NULL,
            U_POSITIVE, U_NEGATIVE,

            /* LIKE */
            LIKE, NOT_LIKE,

            /* BETWEEN */
            BETWEEN, NOT_BETWEEN,

            /* IN */
            IN, NOT_IN,

            /* EXISTS */
            EXISTS,

            /* CONSTANT */
            C_QUES, C_TRUE, C_FALSE, C_UNKNOWN, C_DEFAULT, C_NULL, C_NUMBER, C_STRING,
            /* SUBQUERY */
            EXPR_SUBQUERY,
            /* LIST */
            EXPR_LIST,
            /* CASE */
            EXPR_CASE,
            /* CALL FUNCTION */
            EXPR_FUNC,
            /* COLUMN REFERENCE */
            EXPR_COLUMN_REF
        };
        AstExpr(EXPR_TYPE expr_type) : IObject(AST_EXPR), _expr_type(expr_type) {}
        virtual EXPR_TYPE GetExprType() { return _expr_type; }
    private:
        EXPR_TYPE _expr_type;
    };

    class AstBinaryOpExpr : public AstExpr {
    public:
        AstBinaryOpExpr(EXPR_TYPE expr_type) : AstExpr(expr_type), _left(nullptr), _right(nullptr) {}
        void SetLeft(AstExpr *left) { _left = left; }
        AstExpr *GetLeft() { return _left; }
        void SetRight(AstExpr *right) { _right = right; }
        AstExpr *GetRight() { return _right; }
    private:
        AstExpr   *_left;
        AstExpr   *_right;
    };

    class AstUnaryOpExpr : public AstExpr {
    public:
        AstUnaryOpExpr(EXPR_TYPE expr_type) : AstExpr(expr_type), _expr(nullptr) {}
        void SetExpr(AstExpr *expr) { _expr = expr; }
        AstExpr *GetExpr() { return _expr; }
    private:
        AstExpr     *_expr;
    };

    class AstExistsExpr : public AstExpr {
    public:
        AstExistsExpr() : AstExpr(EXISTS), _query(nullptr) {}
        void SetQuery(AstSelectStmt *query) { _query = query; }
        AstSelectStmt *GetQuery() { return _query; }
    private:
        AstSelectStmt   *_query;
    };

    class AstInExpr : public AstExpr {
    public:
        AstInExpr(bool has_not) : AstExpr(has_not ? NOT_IN : IN), _left(nullptr), _in(nullptr) {}
        void SetLeft(AstExpr *left) { _left = left; }
        AstExpr *GetLeft() { return _left; }
        void SetIn(AstExpr *in) { _in = in; }
        AstExpr *GetIn() { return _in; }
    private:
        AstExpr     *_left;
        AstExpr     *_in;
    };

    class AstBetweenExpr : public AstExpr {
    public:
        AstBetweenExpr(bool has_not) : AstExpr(has_not ? NOT_BETWEEN : BETWEEN), _left(nullptr), _from(nullptr), _to(
                nullptr) {}
        void SetLeft(AstExpr *left) { _left = left; }
        AstExpr *GetLeft() { return _left; }
        void SetFrom(AstExpr *from) { _from = from; }
        AstExpr *GetFrom() { return _from; }
        void SetTo(AstExpr *to) { _to = to; }
        AstExpr *GetTo() { return _to; }
    private:
        AstExpr     *_left;
        AstExpr     *_from;
        AstExpr     *_to;
    };

    class AstLikeExpr : public AstExpr {
    public:
        AstLikeExpr(bool has_not) : AstExpr(has_not ? NOT_LIKE : LIKE),
            _left(nullptr), _right(nullptr), _escape(nullptr) {}
        void SetLeft(AstExpr *left) { _left = left; }
        AstExpr *GetLeft() { return _left; }
        void SetRight(AstExpr *right) { _right = right; }
        AstExpr *GetRight() { return _right; }
        void SetEscape(AstExpr *escape) { _escape = escape; }
        AstExpr *GetEscape() { return _escape; }
    private:
        AstExpr     *_left;
        AstExpr     *_right;
        AstExpr     *_escape;
    };

    class AstConstantValue : public AstExpr {
    public:
        AstConstantValue(EXPR_TYPE expr_type) : AstExpr(expr_type), u{0} {}
        void SetValue(int data) { u._int_data = data; }
        void SetValue(const std::string& value) { u._other_data = strdup(value.c_str()); }
    private:
        union {
            int _int_data;
            char *_other_data;
        } u;
    };

    class AstColumnRef : public AstExpr {
    public:
        AstColumnRef() : AstExpr(EXPR_COLUMN_REF) {}
        void SetColumn(const AstIds& ids, bool is_wild) { _ids = ids; _is_use_wild = is_wild; }
        bool IsWild() { return _is_use_wild; }
        const std::vector<AstId*>& GetColumn() { return _ids; }
    private:
        AstIds  _ids;
        bool    _is_use_wild;
    };

    class AstFuncCall : public AstExpr {
    public:
        AstFuncCall() : AstExpr(EXPR_FUNC), _params(nullptr) {}
        void SetFuncName(const AstIds& name) { _func_name = name; }
        void SetParams(AstExprList *params) { _params = params; }
    private:
        AstIds           _func_name;
        AstExprList     *_params;       /* null means no param */
    };

    class AstCaseExpr : public AstExpr {
    public:
        AstCaseExpr() : AstExpr(EXPR_CASE), _arg(nullptr), _else(nullptr) {}
        void SetArg(AstExpr *arg) { _arg = arg; }
        AstExpr *GetArg() { return _arg; }
        void SetWhenList(const AstExprs& when_list) { _when_list = when_list; }
        const AstExprs& GetWhenList() { return _when_list; }
        void SetThenList(const AstExprs& then_list) { _then_list = then_list; }
        const AstExprs& GetThenList() { return _then_list; }
        void SetElse(AstExpr *els) { _else = els; }
        AstExpr *GetElse() { return _else; }
    private:
        AstExpr      *_arg;      /* null means no arg */
        AstExprs      _when_list;
        AstExprs      _then_list;
        AstExpr      *_else;
    };

    class AstExprList : public AstExpr {
    public:
        AstExprList() : AstExpr(EXPR_LIST) {}
        void SetExprs(const AstExprs& exprs) { _exprs = exprs; }
        const AstExprs& GetExprs() { return _exprs; }
    private:
        AstExprs        _exprs;
    };

    class AstSubqueryExpr : public AstExpr {
    public:
        AstSubqueryExpr() : AstExpr(EXPR_SUBQUERY), _subquery(nullptr) {}
        void SetQuery(AstSelectStmt *query) { _subquery; }
        AstSelectStmt *GetQuery() { return _subquery; }
    private:
        AstSelectStmt   *_subquery;
    };

}

#endif