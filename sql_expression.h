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

            PLUS, MINUS, BARBAR, MUL, DIV, REM, MOD, CARET,
            /* UNARY */
            NOT, IS_TRUE, IS_NOT_TRUE, IS_FALSE, IS_NOT_FALSE, IS_UNKNOWN, IS_NOT_UNKNOWN, IS_NULL, IS_NOT_NULL,
            U_POSITIVE, U_NEGATIVE,

            /* COMPARE ALL/SOME/ANY */
            COMP_LE_ALL, COMP_LT_ALL, COMP_GE_ALL, COMP_GT_ALL, COMP_EQ_ALL, COMP_NEQ_ALL,
            COMP_LE_SOME, COMP_LT_SOME, COMP_GE_SOME, COMP_GT_SOME, COMP_EQ_SOME, COMP_NEQ_SOME,
            COMP_LE_ANY, COMP_LT_ANY, COMP_GE_ANY, COMP_GT_ANY, COMP_EQ_ANY, COMP_NEQ_ANY,

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
        AstExpr(EXPR_TYPE expr_type);
        EXPR_TYPE GetExprType();
        virtual ~AstExpr() {}
    private:
        EXPR_TYPE _expr_type;
    };

    class AstBinaryOpExpr : public AstExpr {
    public:
        AstBinaryOpExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *right);
        ~AstBinaryOpExpr();
        void        SetLeft(AstExpr *left);
        AstExpr    *GetLeft();
        void        SetRight(AstExpr *right);
        AstExpr    *GetRight();
    private:
        AstExpr   *_left;
        AstExpr   *_right;
    };

    class AstUnaryOpExpr : public AstExpr {
    public:
        AstUnaryOpExpr(EXPR_TYPE expr_type, AstExpr *expr);
        ~AstUnaryOpExpr();
        void        SetExpr(AstExpr *expr);
        AstExpr    *GetExpr();
    private:
        AstExpr     *_expr;
    };

    class AstQuantifiedCompareExpr : public AstExpr {
    public:
        AstQuantifiedCompareExpr(EXPR_TYPE expr_type, AstExpr *left, AstSelectStmt *query);
        ~AstQuantifiedCompareExpr();
        AstExpr         *GetLeft();
        AstSelectStmt   *GetQuery();
    private:
        AstExpr         *_left;
        AstSelectStmt   *_query;
    };

    class AstExistsExpr : public AstExpr {
    public:
        AstExistsExpr(AstSelectStmt *query);
        ~AstExistsExpr();
        void            SetQuery(AstSelectStmt *query);
        AstSelectStmt  *GetQuery();
    private:
        AstSelectStmt   *_query;
    };

    class AstInExpr : public AstExpr {
    public:
        AstInExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *in);
        ~AstInExpr();
        void        SetLeft(AstExpr *left);
        AstExpr    *GetLeft();
        void        SetIn(AstExpr *in);
        AstExpr    *GetIn();
    private:
        AstExpr     *_left;
        AstExpr     *_in;
    };

    class AstBetweenExpr : public AstExpr {
    public:
        AstBetweenExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *from, AstExpr *to);
        ~AstBetweenExpr();
        void        SetLeft(AstExpr *left);
        AstExpr    *GetLeft();
        void        SetFrom(AstExpr *from);
        AstExpr    *GetFrom();
        void        SetTo(AstExpr *to);
        AstExpr    *GetTo();
    private:
        AstExpr     *_left;
        AstExpr     *_from;
        AstExpr     *_to;
    };

    class AstLikeExpr : public AstExpr {
    public:
        AstLikeExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *right, AstExpr *escape);
        ~AstLikeExpr();
        void        SetLeft(AstExpr *left);
        AstExpr    *GetLeft();
        void        SetRight(AstExpr *right);
        AstExpr    *GetRight();
        void        SetEscape(AstExpr *escape);
        AstExpr    *GetEscape();
    private:
        AstExpr     *_left;
        AstExpr     *_right;
        AstExpr     *_escape;
    };

    class AstConstantValue : public AstExpr {
    public:
        AstConstantValue(EXPR_TYPE expr_type);
        ~AstConstantValue();
        void SetValue(int data);
        void SetValue(const std::string& value);
        int  GetValueAsInt();
        const char *GetValue();
    private:
        union {
            int _int_data;
            char *_other_data;
        } u;
    };

    class AstColumnRef : public AstExpr {
    public:
        AstColumnRef(const AstIds& ids, bool use_wild);
        ~AstColumnRef();
        void                        SetColumn(const AstIds& ids, bool is_wild);
        bool                        IsWild();
        const AstIds&  GetColumn();
    private:
        AstIds  _ids;
        bool    _is_use_wild;
    };

    class AstFuncCall : public AstExpr {
    public:
        AstFuncCall(const AstIds& func_name, AstExprList *params);
        ~AstFuncCall();
        void SetFuncName(const AstIds& name);
        void SetParams(AstExprList *params);
    private:
        AstIds           _func_name;
        AstExprList     *_params;       /* null means no param */
    };

    class AstCaseExpr : public AstExpr {
    public:
        AstCaseExpr();
        void                SetArg(AstExpr *arg);
        AstExpr            *GetArg();
        void                SetWhenList(const AstExprs& when_list);
        const AstExprs&     GetWhenList();
        void                SetThenList(const AstExprs& then_list);
        const AstExprs&     GetThenList();
        void                SetElse(AstExpr *els);
        AstExpr            *GetElse();
    private:
        AstExpr      *_arg;      /* null means no arg */
        AstExprs      _when_list;
        AstExprs      _then_list;
        AstExpr      *_else;
    };

    class AstExprList : public AstExpr {
    public:
        AstExprList(const AstExprs& exprs);
        ~AstExprList();
        void            SetExprs(const AstExprs& exprs);
        const AstExprs& GetExprs();
    private:
        AstExprs        _exprs;
    };

    class AstSubqueryExpr : public AstExpr {
    public:
        AstSubqueryExpr(AstSelectStmt *subquery);
        ~AstSubqueryExpr();
        void            SetQuery(AstSelectStmt *query);
        AstSelectStmt  *GetQuery();
    private:
        AstSelectStmt   *_subquery;
    };

}

#endif