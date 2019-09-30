#include "sql_table_ref.h"
#include "sql_expression.h"
#include "sql_select_stmt.h"


namespace GSP {
    /* AstTableJoin */
    AstTableJoin::AstTableJoin(TABLE_REF_TYPE join_type, AstTableRef *left, AstTableRef *right, AstSearchCondition *on) : AstTableRef(AST_TABLE_JOIN),
        _join_type(join_type), _left(left), _right(right), _on_search_condition(on) {}

    AstTableJoin::~AstTableJoin() {
        delete (_left); _left = nullptr;
        delete (_right); _right = nullptr;
        delete (_on_search_condition); _on_search_condition = nullptr;
    }

    AstTableRef::TABLE_REF_TYPE AstTableJoin::GetTableRefType() {
        return _join_type;
    }

    void AstTableJoin::SetJoinType(TABLE_REF_TYPE tp) {
        _join_type = tp;
    }

    AstTableRef::TABLE_REF_TYPE AstTableJoin::GetJoinType() {
        return _join_type;
    }

    void AstTableJoin::SetLeft(AstTableRef *left) {
        _left = left;
    }

    AstTableRef *AstTableJoin::GetLeft() {
        return _left;
    }

    void AstTableJoin::SetRight(AstTableRef *right) {
        _right = right;
    }

    AstTableRef *AstTableJoin::GetRight() {
        return _right;
    }

    void AstTableJoin::SetOn(AstSearchCondition *search_condition) {
        _on_search_condition = search_condition;
    }

    AstSearchCondition *AstTableJoin::GetOn() {
        return _on_search_condition;
    }

    /* AstRelation */
    AstRelation::AstRelation(const AstIds& ids, AstId *alias) : AstTableRef(AST_RELATION), _ids(ids), _alias(alias) {}

    AstRelation::~AstRelation() {
        for (auto it : _ids) delete (it);
        _ids.clear();
        delete (_alias);
        _alias = nullptr;
    }

    AstTableRef::TABLE_REF_TYPE AstRelation::GetTableRefType() {
        return RELATION;
    }

    void AstRelation::SetRelationAndAlias(const AstIds& ids, AstId *alias) {
        _ids = ids;
        _alias = alias;
    }

    const AstIds& AstRelation::GetIds() {
        return _ids;
    }

    /* AstSubQueryTableRef */
    AstSubQueryTableRef::AstSubQueryTableRef(AstSelectStmt *subquery, AstId *alias, const AstIds& col_alias) : AstTableRef(AST_SUBQUERY_TABLE_REF),
    _subquery(subquery), _alias(alias), _col_alias(col_alias) {}

    AstSubQueryTableRef::~AstSubQueryTableRef() {
        delete (_subquery); _subquery = nullptr;
        delete (_alias); _alias = nullptr;
        for (auto it : _col_alias) delete (it);
        _col_alias.clear();
    }

    AstTableRef::TABLE_REF_TYPE AstSubQueryTableRef::GetTableRefType() {
        return SUBQUERY;
    }

    void AstSubQueryTableRef::SetQuery(AstSelectStmt *subquery, AstId *alias, const std::vector<AstId*>& col_alias) {
        _subquery = subquery; _alias = alias; _col_alias = col_alias;
    }

    AstSelectStmt *AstSubQueryTableRef::GetQuery() {
        return _subquery;
    }

    AstId *AstSubQueryTableRef::GetAlias() {
        return _alias;
    }

    const AstIds& AstSubQueryTableRef::GetColAlias() {
        return _col_alias;
    }
}