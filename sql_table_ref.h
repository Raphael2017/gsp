#ifndef GSP_SQL_TABLEREF_H
#define GSP_SQL_TABLEREF_H

#include "sql_object.h"

namespace GSP {

    class AstRelation;
    class AstSelectStmt;
    class AstSearchCondition;

    class AstTableRef : public IObject {
    public:
        virtual ~AstTableRef() {}
        enum TABLE_REF_TYPE { RELATION, SUBQUERY, JOIN,
            FULL_JOIN, FULL_OUTER_JOIN,
            LEFT_JOIN, LEFT_OUTER_JOIN,
            RIGHT_JOIN, RIGHT_OUTER_JOIN,
            INNER_JOIN,
            CROSS_JOIN,
            NATURAL_JOIN,
            NATURAL_FULL_JOIN, NATURAL_FULL_OUTER_JOIN,
            NATURAL_LEFT_JOIN, NATURAL_LEFT_OUTER_JOIN,
            NATURAL_RIGHT_JOIN, NATURAL_RIGHT_OUTER_JOIN,
            NATURAL_INNER_JOIN };
        virtual TABLE_REF_TYPE GetTableRefType() = 0;
    };

    class AstTableJoin : public AstTableRef {
    public:
        AstTableJoin() : _left(nullptr), _right(nullptr), _on_search_condition(nullptr) {}
        ~AstTableJoin() {
            delete (_left); _left = nullptr;
            delete (_right); _right = nullptr;
            delete (_on_search_condition); _on_search_condition = nullptr;
            for (auto it : _using_ids) delete (it);
            _using_ids.clear();
        }
        TABLE_REF_TYPE GetTableRefType() override { return _join_type; }
        void SetJoinType(TABLE_REF_TYPE tp) { _join_type = tp; }
        TABLE_REF_TYPE GetJoinType() { return _join_type; }
        void SetLeft(AstTableRef *left) { _left = left; }
        AstTableRef *GetLeft() { return _left; }
        void SetRight(AstTableRef *right) { _right = right; }
        AstTableRef *GetRight() { return _right; }
        void SetOn(AstSearchCondition *search_condition) { _on_search_condition = search_condition; }
        AstSearchCondition *GetOn() { return _on_search_condition; }
    private:
        TABLE_REF_TYPE       _join_type;
        AstTableRef         *_left;
        AstTableRef         *_right;
        AstSearchCondition  *_on_search_condition;
        std::vector<AstId*>  _using_ids;            /* todo */
    };

    class AstTableCrossJoin : public AstTableRef {
    public:
        AstTableCrossJoin() : _left(nullptr), _right(nullptr) {}
        ~AstTableCrossJoin() { delete (_left); _left = nullptr; delete (_right); _right = nullptr; }
        TABLE_REF_TYPE GetTableRefType() override { return CROSS_JOIN; }
        void SetLeft(AstTableRef *left) { _left = left; }
        AstTableRef *GetLeft() { return _left; }
        void SetRight(AstTableRef *right) { _right = right; }
        AstTableRef *GetRight() { return _right; }
    private:
        AstTableRef         *_left;
        AstTableRef         *_right;
    };

    class AstNaturalJoin : public AstTableRef {
    public:
        AstNaturalJoin() : _left(nullptr), _right(nullptr) {}
        ~AstNaturalJoin() { delete (_left); _left = nullptr; delete (_right); _right = nullptr; }
        TABLE_REF_TYPE GetTableRefType() override { return _join_type; }
        void SetJoinType(TABLE_REF_TYPE tp) { _join_type = tp; }
        TABLE_REF_TYPE GetJoinType() { return _join_type; }
        void SetLeft(AstTableRef *left) { _left = left; }
        AstTableRef *GetLeft() { return _left; }
        void SetRight(AstTableRef *right) { _right = right; }
        AstTableRef *GetRight() { return _right; }
    private:
        TABLE_REF_TYPE       _join_type;
        AstTableRef         *_left;
        AstTableRef         *_right;
    };

    class AstRelation : public AstTableRef {
    public:
        ~AstRelation() {
            for (auto it : _ids) delete (it);
            _ids.clear();
        }
        TABLE_REF_TYPE GetTableRefType() override { return RELATION; }
        void SetRelationAndAlias(const std::vector<AstId*>& ids, AstId *alias) { _ids = ids; _alias = alias; }
        const std::vector<AstId*>& GetIds() { return _ids; }
    private:
        std::vector<AstId*> _ids;
        AstId              *_alias;
    };

    class AstSubQueryTableRef : public AstTableRef {
    public:
        AstSubQueryTableRef() : _subquery(nullptr), _alias(nullptr) {}
        ~AstSubQueryTableRef() {
            delete (_subquery); _subquery = nullptr;
            delete (_alias); _alias = nullptr;
            for (auto it : _col_alias) delete (it);
            _col_alias.clear();
        }
        TABLE_REF_TYPE GetTableRefType() override { return SUBQUERY; }
        void SetQuery(AstSelectStmt *subquery, AstId *alias, const std::vector<AstId*>& col_alias) { _subquery = subquery; _alias = alias; _col_alias = col_alias; }
        AstSelectStmt *GetQuery() { return _subquery; }
        AstId *GetAlias() { return _alias; }
        const std::vector<AstId*>& GetColAlias() { return _col_alias; }
    private:
        AstSelectStmt       *_subquery;
        AstId               *_alias;
        std::vector<AstId*>  _col_alias;
    };
}

#endif