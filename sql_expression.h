#ifndef GSP_SQL_EXPRESSION_H
#define GSP_SQL_EXPRESSION_H

#include <vector>
#include "sql_object.h"
#include <string.h>

namespace GSP {

    class AstRowExpr;
    class AstConstantValue;
    class AstColumnRef;
    class AstFuncCall;
    class AstCaseExpr;
    class AstExprList;
    class AstSelectStmt;
    class AstId;

    class AstSearchCondition : public IObject {
    public:
        enum EXPR_TYPE {
                NOT, IS_TRUE, IS_NOT_TRUE, IS_FALSE, IS_NOT_FALSE, IS_UNKNOWN, IS_NOT_UNKNOWN, IS_NULL, IS_NOT_NULL, EXISTS,
                OR, AND, COMP_LE, COMP_LT, COMP_GE, COMP_GT, COMP_EQ, COMP_NEQ, IN, NOT_IN,
                    COMP_LE_ALL, COMP_LT_ALL, COMP_GE_ALL, COMP_GT_ALL, COMP_EQ_ALL, COMP_NEQ_ALL,
                    COMP_LE_SOME, COMP_LT_SOME, COMP_GE_SOME, COMP_GT_SOME, COMP_EQ_SOME, COMP_NEQ_SOME,
                    COMP_LE_ANY, COMP_LT_ANY, COMP_GE_ANY, COMP_GT_ANY, COMP_EQ_ANY, COMP_NEQ_ANY,
                BETWEEN, NOT_BETWEEN,
                LIKE, NOT_LIKE,
                ROW_EXPR } ;
        void SetExprType(EXPR_TYPE tp) { _expr_type = tp; }
        EXPR_TYPE GetExprType() { return _expr_type; }
        void SetLeft(AstSearchCondition *sc) { u._bbody._left = sc; }
        void SetRight(AstSearchCondition *sc) { u._bbody._right = sc; }
        void SetSc(AstSearchCondition *sc) { u._not_search_condition = sc; }
        void SetExist(AstSelectStmt *s) { u._exist_query = s; }
        void SetRowExpr1(AstRowExpr *re) { u._row_expr = re; }
        AstRowExpr *GetRowExpr1() { return u._row_expr; }
        void SetRowExpr3(AstRowExpr *r1, AstRowExpr *r2, AstRowExpr *r3) {
            u._tbody._r1 = r1; u._tbody._r2 = r2; u._tbody._r3 = r3;
        }
    private:
        EXPR_TYPE _expr_type;
        union {
            struct {
                AstSearchCondition  *_left;
                AstSearchCondition  *_right;
            } _bbody;
            AstSearchCondition *_not_search_condition;  // for NOT, IS_TRUE ...
            AstRowExpr         *_row_expr;
            AstSelectStmt      *_exist_query;   // EXISTS COMP_EQ_ANY
            struct {
                AstRowExpr      *_r1;
                AstRowExpr      *_r2;
                AstRowExpr      *_r3;
            } _tbody;
        } u;
    };

    class AstRowExpr : public IObject {
    public:
        enum ROW_EXPR_TYPE { PLUS, MINUS, BARBAR, MUL, DIV, REM, MOD, CARET,
            U_PLUS, U_MINUS,
            C_QUES, C_TRUE, C_FALSE, C_UNKNOWN, C_DEFAULT, C_NULL, C_VALUE,
            COLUMN_REF, FUNC_CALL,
            CASE_EXPR,
            SC_LIST, SEARCH_COND, SUBQUERY};
        ROW_EXPR_TYPE GetRowType() { return  _type; }
        void SetRowType(ROW_EXPR_TYPE tp) { _type = tp; }
        void SetLeft(AstRowExpr *left) { u._bbody._left = left; }
        void SetRight(AstRowExpr *right) { u._bbody._right = right; }
        void SetQuery(AstSelectStmt *s) { u._subquery = s; }
        void SetExprList(AstExprList *rl) { u._expr_list = rl; }
        void SetColumnRef(AstColumnRef *cr) { u._column_ref = cr; }
        AstColumnRef *GetColumnRef() { return u._column_ref; }
        void SetConstantValue(AstConstantValue *c) { u._value = c; }
        void SetUnaryExpr(AstRowExpr *r) { u._u_row_expr = r; }
        void SetFunc(AstFuncCall *f) { u._func_call = f; }
        void SetSearchCondition(AstSearchCondition *sc) { u._sc = sc; }
    private:
        ROW_EXPR_TYPE _type;
        union {
            struct {
                AstRowExpr      *_left;
                AstRowExpr      *_right;
            } _bbody;
            AstRowExpr          *_u_row_expr;
            AstConstantValue    *_value;
            AstColumnRef        *_column_ref;
            AstFuncCall         *_func_call;
            AstCaseExpr         *_case;
            AstExprList         *_expr_list;
            AstSearchCondition  *_sc;
            AstSelectStmt       *_subquery;
        } u;
    };

    class AstConstantValue : public IObject {
    public:
        enum CONSTANT_TYPE { C_NUMBER, C_STRING };
        void SetConstantType(CONSTANT_TYPE type) { _type = type; }
        void SetValue(int data) { u._int_data = data; }
        void SetValue(const std::string& value) { u._other_data = strdup(value.c_str()); }
    private:
        CONSTANT_TYPE _type;
        union {
            int _int_data;
            char *_other_data;
        } u;
    };

    class AstColumnRef : public IObject {
    public:
        void SetColumn(const std::vector<AstId*>& ids, bool is_wild) { _ids = ids; _is_use_wild = is_wild; }
        bool IsWild() { return _is_use_wild; }
        std::vector<AstId*> GetColumn() { return _ids; }
    private:
        std::vector<AstId*> _ids;
        bool _is_use_wild;
    };

    class AstFuncCall : public IObject {
    public:
        void SetFuncName(const std::vector<AstId*>& name) { _func_name = name; }
        void SetParams(AstExprList *params) { _params = params; }
    private:
        std::vector<AstId*> _func_name;
        AstExprList     *_params;       /* null means no param */
    };

    class AstCaseExpr : public IObject {
    private:
        AstSearchCondition      *_arg;      /* null means no arg */
        std::vector<AstSearchCondition *> _when_list;
        std::vector<AstSearchCondition *> _then_list;
        AstSearchCondition      *_else;     /* null means no arg */
    };

    class AstExprList : public IObject {
    public:
        void SetExprs(const std::vector<AstSearchCondition*>& exprs) { _exprs = exprs; }
        std::vector<AstSearchCondition*> GetExprs() { return _exprs; }
    private:
        std::vector<AstSearchCondition*> _exprs;
    };
}

#endif