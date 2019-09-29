#ifndef GSP_SQL_TABLEREF_H
#define GSP_SQL_TABLEREF_H

#include <vector>
#include "sql_object.h"

namespace GSP {

    class AstRelation;
    class AstSelectStmt;
    class AstSearchCondition;

    class AstTableRef : public IObject {
    public:
        AstTableRef(SQLObjectType obj_type) : IObject(obj_type) {}
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
        AstTableJoin();
        ~AstTableJoin();
        TABLE_REF_TYPE      GetTableRefType() override;
        void                SetJoinType(TABLE_REF_TYPE tp);
        TABLE_REF_TYPE      GetJoinType();
        void                SetLeft(AstTableRef *left);
        AstTableRef        *GetLeft();
        void                SetRight(AstTableRef *right);
        AstTableRef        *GetRight();
        void                SetOn(AstSearchCondition *search_condition);
        AstSearchCondition *GetOn();
    private:
        TABLE_REF_TYPE       _join_type;
        AstTableRef         *_left;
        AstTableRef         *_right;
        AstSearchCondition  *_on_search_condition;
    };

    class AstRelation : public AstTableRef {
    public:
        AstRelation();
        ~AstRelation();
        TABLE_REF_TYPE  GetTableRefType() override;
        void            SetRelationAndAlias(const AstIds& ids, AstId *alias);
        const AstIds&   GetIds();
    private:
        AstIds              _ids;
        AstId              *_alias;
    };

    class AstSubQueryTableRef : public AstTableRef {
    public:
        AstSubQueryTableRef();
        ~AstSubQueryTableRef();
        TABLE_REF_TYPE  GetTableRefType() override;
        void            SetQuery(AstSelectStmt *subquery, AstId *alias, const AstIds& col_alias);
        AstSelectStmt  *GetQuery();
        AstId          *GetAlias();
        const AstIds&   GetColAlias();
    private:
        AstSelectStmt       *_subquery;
        AstId               *_alias;
        AstIds               _col_alias;
    };
}

#endif