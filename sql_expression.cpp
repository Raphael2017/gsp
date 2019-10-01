#include "sql_table_ref.h"
#include "sql_expression.h"
#include "sql_select_stmt.h"

namespace GSP {
    /* AstExpr */
    AstExpr::AstExpr(EXPR_TYPE expr_type) : IObject(AST_EXPR), _expr_type(expr_type) {}

    AstExpr::EXPR_TYPE AstExpr::GetExprType() { return _expr_type; }

    /* AstBinaryOpExpr */
    AstBinaryOpExpr::AstBinaryOpExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *right) : AstExpr(expr_type), _left(left), _right(right) {}

    AstBinaryOpExpr::~AstBinaryOpExpr() {
        delete (_left); _left = nullptr;
        delete (_right); _right = nullptr;
    }
    void AstBinaryOpExpr::SetLeft(AstExpr *left) { _left = left; }

    AstExpr *AstBinaryOpExpr::GetLeft() { return _left; }

    void AstBinaryOpExpr::SetRight(AstExpr *right) { _right = right; }

    AstExpr *AstBinaryOpExpr::GetRight() { return _right; }

    /* AstUnaryOpExpr */
    AstUnaryOpExpr::AstUnaryOpExpr(EXPR_TYPE expr_type, AstExpr *expr) : AstExpr(expr_type), _expr(expr) {}

    AstUnaryOpExpr::~AstUnaryOpExpr() { delete (_expr); _expr = nullptr; }

    void AstUnaryOpExpr::SetExpr(AstExpr *expr) { _expr = expr; }

    AstExpr *AstUnaryOpExpr::GetExpr() { return _expr; }

    /* AstQuantifiedCompareExpr */
    AstQuantifiedCompareExpr::AstQuantifiedCompareExpr(EXPR_TYPE expr_type, AstExpr *left, AstSelectStmt *query) : AstExpr(expr_type), _left(left), _query(query) {}

    AstQuantifiedCompareExpr::~AstQuantifiedCompareExpr() {
        delete (_left); _left = nullptr;
        delete (_query); _query = nullptr;
    }
    AstExpr *AstQuantifiedCompareExpr::GetLeft() { return _left; }

    AstSelectStmt *AstQuantifiedCompareExpr::GetQuery() { return _query; }

    /* AstExistsExpr */
    AstExistsExpr::AstExistsExpr(AstSelectStmt *query) : AstExpr(EXISTS), _query(query) {}

    AstExistsExpr::~AstExistsExpr() { delete (_query); _query = nullptr; }

    void AstExistsExpr::SetQuery(AstSelectStmt *query) { _query = query; }

    AstSelectStmt *AstExistsExpr::GetQuery() { return _query; }

    /* AstInExpr */
    AstInExpr::AstInExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *in) : AstExpr(expr_type), _left(left), _in(in) {}

    AstInExpr::~AstInExpr() {
        delete (_left); _left = nullptr;
        delete (_in); _in = nullptr;
    }

    void AstInExpr::SetLeft(AstExpr *left) { _left = left; }

    AstExpr *AstInExpr::GetLeft() { return _left; }

    void AstInExpr::SetIn(AstExpr *in) { _in = in; }

    AstExpr *AstInExpr::GetIn() { return _in; }

    /* AstBetweenExpr */
    AstBetweenExpr::AstBetweenExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *from, AstExpr *to) : AstExpr(expr_type), _left(left), _from(from), _to(to) {}

    AstBetweenExpr::~AstBetweenExpr() {
        delete (_left); _left = nullptr;
        delete (_from); _from = nullptr;
        delete (_to);   _to = nullptr;
    }

    void AstBetweenExpr::SetLeft(AstExpr *left) { _left = left; }

    AstExpr *AstBetweenExpr::GetLeft() { return _left; }

    void AstBetweenExpr::SetFrom(AstExpr *from) { _from = from; }

    AstExpr *AstBetweenExpr::GetFrom() { return _from; }

    void AstBetweenExpr::SetTo(AstExpr *to) { _to = to; }

    AstExpr *AstBetweenExpr::GetTo() { return _to; }

    /* AstLikeExpr */
    AstLikeExpr::AstLikeExpr(EXPR_TYPE expr_type, AstExpr *left, AstExpr *right, AstExpr *escape) : AstExpr(expr_type), _left(left), _right(right), _escape(escape) {}

    AstLikeExpr::~AstLikeExpr() {
        delete (_left); _left = nullptr;
        delete (_right); _right = nullptr;
        delete (_escape); _escape = nullptr;
    }

    void AstLikeExpr::SetLeft(AstExpr *left) { _left = left; }

    AstExpr *AstLikeExpr::GetLeft() { return _left; }

    void AstLikeExpr::SetRight(AstExpr *right) { _right = right; }

    AstExpr *AstLikeExpr::GetRight() { return _right; }

    void AstLikeExpr::SetEscape(AstExpr *escape) { _escape = escape; }

    AstExpr *AstLikeExpr::GetEscape() { return _escape; }

    /* AstConstantValue */
    AstConstantValue::AstConstantValue(EXPR_TYPE expr_type) : AstExpr(expr_type), u{0} {}

    AstConstantValue::~AstConstantValue() {
        if (GetExprType() == C_STRING || GetExprType() == C_NUMBER) {
            free (u._other_data);
            u._other_data = nullptr;
        }
    }

    void AstConstantValue::SetValue(int data) { u._int_data = data; }

    void AstConstantValue::SetValue(const std::string& value) { u._other_data = strdup(value.c_str()); }

    /* AstColumnRef */
    AstColumnRef::AstColumnRef(const AstIds& ids, bool use_wild) : AstExpr(EXPR_COLUMN_REF), _ids(ids), _is_use_wild(use_wild) {}

    AstColumnRef::~AstColumnRef() {
        for (auto it : _ids) delete (it);
        _ids.clear();
    }

    void AstColumnRef::SetColumn(const AstIds& ids, bool is_wild) { _ids = ids; _is_use_wild = is_wild; }

    bool AstColumnRef::IsWild() { return _is_use_wild; }

    const AstIds& AstColumnRef::GetColumn() { return _ids; }

    /* AstFuncCall */
    AstFuncCall::AstFuncCall(const AstIds& func_name, AstExprList *params) : AstExpr(EXPR_FUNC), _func_name(func_name), _params(params) {}

    AstFuncCall::~AstFuncCall() {
        for (auto it : _func_name) delete (it);
        _func_name.clear();
        delete (_params);
        _params = nullptr;
    }

    void AstFuncCall::SetFuncName(const AstIds& name) { _func_name = name; }

    void AstFuncCall::SetParams(AstExprList *params) { _params = params; }

    /* AstCaseExpr */
    AstCaseExpr::AstCaseExpr() : AstExpr(EXPR_CASE), _arg(nullptr), _else(nullptr) {}

    void AstCaseExpr::SetArg(AstExpr *arg) { _arg = arg; }

    AstExpr *AstCaseExpr::GetArg() { return _arg; }

    void AstCaseExpr::SetWhenList(const AstExprs& when_list) { _when_list = when_list; }

    const AstExprs& AstCaseExpr::GetWhenList() { return _when_list; }

    void AstCaseExpr::SetThenList(const AstExprs& then_list) { _then_list = then_list; }

    const AstExprs& AstCaseExpr::GetThenList() { return _then_list; }

    void AstCaseExpr::SetElse(AstExpr *els) { _else = els; }

    AstExpr *AstCaseExpr::GetElse() { return _else; }

    /* AstExprList */
    AstExprList::AstExprList(const AstExprs& exprs) : AstExpr(EXPR_LIST), _exprs(exprs) {}

    AstExprList::~AstExprList() {
        for (auto it : _exprs) delete (it);
        _exprs.clear();
    }

    void AstExprList::SetExprs(const AstExprs& exprs) { _exprs = exprs; }

    const AstExprs& AstExprList::GetExprs() { return _exprs; }

    /* AstSubqueryExpr */
    AstSubqueryExpr::AstSubqueryExpr(AstSelectStmt *subquery) : AstExpr(EXPR_SUBQUERY), _subquery(subquery) {}

    AstSubqueryExpr::~AstSubqueryExpr() { delete (_subquery); _subquery = nullptr; }

    void AstSubqueryExpr::SetQuery(AstSelectStmt *query) { _subquery = query; }

    AstSelectStmt *AstSubqueryExpr::GetQuery() { return _subquery; }

}