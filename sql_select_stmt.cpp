#include "sql_select_stmt.h"
#include "sql_expression.h"
#include "sql_table_ref.h"

namespace GSP {
    /* AstSelectStmt */
    AstSelectStmt::AstSelectStmt() : IObject(AST_SELECT_STMT), _with_clasue(nullptr), _query_expression_body(nullptr) {}

    AstSelectStmt::~AstSelectStmt() {
        delete (_with_clasue); _with_clasue = nullptr;
        for (auto it : _order_by_items) delete (it);
        _order_by_items.clear();
        delete (_query_expression_body); _query_expression_body = nullptr;
    }

    void AstSelectStmt::SetWithClause(AstWithClause *with_clasue) {
        _with_clasue = with_clasue;
    }

    AstWithClause *AstSelectStmt::GetWithClause() {
        return _with_clasue;
    }

    void AstSelectStmt::SetOrderByItems(const AstOrderByItems& order_by_items) {
        _order_by_items = order_by_items;
    }

    const AstOrderByItems& AstSelectStmt::GetOrderByItems() {
        return _order_by_items;
    }

    void AstSelectStmt::SetBody(AstQueryExpressionBody *body) {
        _query_expression_body = body;
    }

    AstQueryExpressionBody *AstSelectStmt::GetBody() {
        return _query_expression_body;
    }

    /* AstWithClause */
    AstWithClause::AstWithClause() : IObject(AST_WITH_CLAUSE) {}

    AstWithClause::~AstWithClause() {
        for (auto it : _ctes) delete (it);
        _ctes.clear();
    }

    void AstWithClause::SetRecType(REC_TYPE rec_type) {
        _rec_type = rec_type;
    }

    AstWithClause::REC_TYPE AstWithClause::GetRecType() {
        return _rec_type;
    }

    void AstWithClause::SetCtes(const AstCommonTableExprs& ctes) {
        _ctes = ctes;
    }

    const AstCommonTableExprs& AstWithClause::GetCtes() {
        return _ctes;
    }

    /* AstCommonTableExpr */
    AstCommonTableExpr::AstCommonTableExpr() :IObject(AST_COMMON_TABLE_EXPR), _cte_name(nullptr), _query(nullptr) {}

    AstCommonTableExpr::~AstCommonTableExpr() {
        delete (_cte_name); _cte_name = nullptr;
        for (auto it : _cte_columns) delete (it);
        _cte_columns.clear();
        delete (_query); _query = nullptr;
    }

    void AstCommonTableExpr::SetCteName(AstId *cte_name) {
        _cte_name = cte_name;
    }

    AstId *AstCommonTableExpr::GetCteName() {
        return _cte_name;
    }

    void AstCommonTableExpr::SetCteColumns(const AstIds& cte_columns) {
        _cte_columns = cte_columns;
    }

    const AstIds& AstCommonTableExpr::GetCteColumns() {
        return _cte_columns;
    }

    void AstCommonTableExpr::SetQuery(AstSelectStmt *query) {
        _query = query;
    }

    AstSelectStmt *AstCommonTableExpr::GetQuery() {
        return _query;
    }

    /* AstQuerySet */
    AstQuerySet::AstQuerySet() : AstQueryExpressionBody(AST_QUERY_SET), _left(nullptr), _right(nullptr) {}

    AstQuerySet::~AstQuerySet() {
        delete (_left); _left = nullptr;
        delete (_right); _right = nullptr;
    }

    void AstQuerySet::SetSetType(SET_TYPE set_type) {
        _set_type = set_type;
    }

    AstQueryExpressionBody::SET_TYPE AstQuerySet::GetSetType() {
        return _set_type;
    }

    void AstQuerySet::SetLeft(AstQueryExpressionBody *left) {
        _left = left;
    }

    AstQueryExpressionBody *AstQuerySet::GetLeft() {
        return _left;
    }

    void AstQuerySet::SetRight(AstQueryExpressionBody *right) {
        _right = right;
    }

    AstQueryExpressionBody *AstQuerySet::GetRight() {
        return _right;
    }

    /* AstQueryPrimary */
    AstQueryPrimary::AstQueryPrimary() : AstQueryExpressionBody(AST_QUERY_PRIMARY), _where_search_condition(nullptr), _having_search_condition(nullptr) {}

    AstQueryPrimary::~AstQueryPrimary() {
        for (auto it : _projection_list) delete (it);
        _projection_list.clear();
        for (auto it : _tableref_list) delete (it);
        _tableref_list.clear();
        delete (_where_search_condition); _where_search_condition = nullptr;
        for (auto it : _group_list) delete (it);
        _group_list.clear();
        delete (_having_search_condition); _having_search_condition = nullptr;
    }
    void AstQueryPrimary::SetSelectType(SELECT_TYPE select_type)  {
        _select_type = select_type;
    }

    AstQueryPrimary::SELECT_TYPE AstQueryPrimary::GetSelectType() {
        return _select_type;
    }

    void AstQueryPrimary::SetProjectionList(const AstProjections& projection_list) {
        _projection_list = projection_list;
    }

    const AstProjections& AstQueryPrimary::GetProjectionList() {
        return _projection_list;
    }

    void AstQueryPrimary::SetFrom(const AstTableRefs& tableref_list) {
        _tableref_list = tableref_list;
    }

    const AstTableRefs& AstQueryPrimary::GetFrom() {
        return _tableref_list;
    }

    void AstQueryPrimary::SetWhere(AstSearchCondition *search_condition) {
        _where_search_condition = search_condition;
    }

    AstSearchCondition *AstQueryPrimary::GetWhere() {
        return _where_search_condition;
    }

    void AstQueryPrimary::SetGroupList(const AstGroupingElems& group_list) {
        _group_list = group_list;
    }

    const AstGroupingElems& AstQueryPrimary::GetGroupList() {
        return _group_list;
    }

    void AstQueryPrimary::SetGroupType(GROUP_TYPE group_type) {
        _group_type = group_type;
    }

    AstQueryPrimary::GROUP_TYPE AstQueryPrimary::GetGroupType() {
        return _group_type;
    }

    void AstQueryPrimary::SetHaving(AstSearchCondition *search_condition) {
        _having_search_condition = search_condition;
    }

    AstSearchCondition *AstQueryPrimary::GetHaving() {
        return _having_search_condition;
    }

    AstQueryExpressionBody::SET_TYPE AstQueryPrimary::GetSetType() {
        return SIMPLE;
    }

    /* AstProjection */
    AstProjection::AstProjection() : IObject(AST_PROJECTION), _expr(nullptr), _alias(nullptr) {}

    AstProjection::~AstProjection() {
        delete (_expr); _expr = nullptr;
        delete (_alias); _alias = nullptr;
    }

    void AstProjection::SetExpr(AstRowExpr *expr) {
        _expr = expr;
    }

    AstRowExpr *AstProjection::GetExpr() {
        return _expr;
    }

    void AstProjection::SetAlias(AstId *alias) {
        _alias = alias;
    }

    AstId *AstProjection::GetAlias() {
        return _alias;
    }

    /* AstOrderByItem */
    AstOrderByItem::AstOrderByItem() : IObject(AST_ORDER_BY_ITEM), _expr(nullptr) {}

    AstOrderByItem::~AstOrderByItem() {
        delete (_expr); _expr = nullptr;
    }

    void AstOrderByItem::SetOrderItem(ORDER_TYPE order_type, AstRowExpr *row_expr) {
        _order_type = order_type; _expr = row_expr;
    }

    AstOrderByItem::ORDER_TYPE AstOrderByItem::GetOrderType() {
        return _order_type;
    }

    AstRowExpr *AstOrderByItem::GetExpr() {
        return _expr;
    }
}