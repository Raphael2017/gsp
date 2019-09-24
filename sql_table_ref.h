#ifndef GSP_SQL_TABLEREF_H
#define GSP_SQL_TABLEREF_H

#include "sql_object.h"

namespace GSP {

    class AstRelation;
    class AstSelectStmt;
    class AstSearchCondition;
    class AstIds;

    class AstTableRef : public IObject {
    public:
        enum JOIN_TYPE { RELATION, SUBQUERY, JOIN,
            FULL_JOIN, FULL_OUTER_JOIN,
            LEFT_JOIN, LEFT_OUTER_JOIN,
            RIGHT_JOIN, RIGHT_OUTER_JOIN,
            INNER_JOIN, CROSS_JOIN,
            NATURAL_JOIN,
            NATURAL_FULL_JOIN, NATURAL_FULL_OUTER_JOIN,
            NATURAL_LEFT_JOIN, NATURAL_LEFT_OUTER_JOIN,
            NATURAL_RIGHT_JOIN, NATURAL_RIGHT_OUTER_JOIN,
            NATURAL_INNER_JOIN };
        void SetJoinType(JOIN_TYPE tp) { _join_type = tp; }
        void SetLeft(AstTableRef *left) { u._join._left = left; }
        void SetRight(AstTableRef *right) { u._join._right = right; }
        void SetOn(AstSearchCondition *sc) { u._join._on_search_condition = sc; }
        void SetRelation(AstRelation *r, AstId *alias) { u._relation_alias._relation = r; u._relation_alias._alias = alias; }
        void SetQuery(AstSelectStmt *s, AstId *alias, const std::vector<AstId*>& cols) {
            u._subquery_alias._subquery = s;
            u._subquery_alias._alias = alias;
            _col_alias = cols;
        }
    private:
        JOIN_TYPE _join_type;
        union {
            struct {
                AstRelation     *_relation;
                AstId           *_alias;
            } _relation_alias;
            struct {
                AstSelectStmt   *_subquery;
                AstId           *_alias;
            } _subquery_alias;
            struct {
                AstTableRef         *_left;
                AstTableRef         *_right;
                AstSearchCondition  *_on_search_condition;
                AstIds              *_using_ids;
            } _join;
        } u;
        std::vector<AstId*> _col_alias;
    };

    class AstRelation : public IObject {
    public:
        void SetIds(const std::vector<AstId*>& ids) { _ids = ids; }
    private:
        std::vector<AstId*> _ids;
    };
}

#endif