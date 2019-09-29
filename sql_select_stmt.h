#ifndef GSP_SQL_SELECT_STMT_H
#define GSP_SQL_SELECT_STMT_H

#include <vector>
#include <assert.h>
#include "sql_object.h"

namespace GSP {

    class AstWithClause;
    class AstCommonTableExpr;
    class AstQueryExpressionBody;
    class AstOrderByItem;
    class AstQueryPrimary;
    class AstProjection;
    class AstTableRef;
    class AstExpr;
    typedef AstExpr                             AstSearchCondition;
    typedef AstExpr                             AstRowExpr;
    typedef AstRowExpr                          AstGroupingElem;
    typedef std::vector<AstGroupingElem*>       AstGroupingElems;
    typedef std::vector<AstCommonTableExpr*>    AstCommonTableExprs;
    typedef std::vector<AstOrderByItem*>        AstOrderByItems;
    typedef std::vector<AstProjection*>         AstProjections;
    typedef std::vector<AstTableRef*>           AstTableRefs;

    class AstSelectStmt : public IObject {
    public:
        AstSelectStmt();
        ~AstSelectStmt() ;
        void                    SetWithClause(AstWithClause *with_clasue);
        AstWithClause          *GetWithClause();
        void                    SetOrderByItems(const AstOrderByItems& order_by_items);
        const AstOrderByItems&  GetOrderByItems();
        void                    SetBody(AstQueryExpressionBody *body);
        AstQueryExpressionBody *GetBody();
    private:
        AstWithClause                   *_with_clasue;      /* null means no with clause */
        std::vector<AstOrderByItem*>     _order_by_items;  /* size 0 means no order by */
        AstQueryExpressionBody          *_query_expression_body;
    };

    class AstWithClause : public IObject {
    public:
        enum REC_TYPE { NIL_RECURSIVE, RECURSIVE };
        AstWithClause();
        ~AstWithClause();
        void        SetRecType(REC_TYPE rec_type);
        REC_TYPE    GetRecType();
        void        SetCtes(const AstCommonTableExprs& ctes);
        const       AstCommonTableExprs& GetCtes();
    private:
        REC_TYPE                            _rec_type;
        AstCommonTableExprs                 _ctes;
    };

    class AstCommonTableExpr : public IObject {
    public:
        AstCommonTableExpr();
        ~AstCommonTableExpr();
        void            SetCteName(AstId *cte_name);
        AstId          *GetCteName();
        void            SetCteColumns(const AstIds& cte_columns);
        const AstIds&   GetCteColumns();
        void            SetQuery(AstSelectStmt *query);
        AstSelectStmt  *GetQuery();
    private:
        AstId                   *_cte_name;
        std::vector<AstId*>      _cte_columns;  /* size 0 means no alias */
        AstSelectStmt           *_query;
    };

    class AstQueryExpressionBody : public IObject {
    public:
        AstQueryExpressionBody(SQLObjectType obj_type) : IObject(obj_type) {}
        virtual ~AstQueryExpressionBody() {}
        enum SET_TYPE { SIMPLE,
            UNION, UNION_ALL, UNION_DISTINCT,
            EXCEPT, EXCEPT_ALL, EXCEPT_DISTINCT,
            INTERSECT, INTERSECT_ALL, INTERSECT_DISTINCT} ;
        virtual SET_TYPE GetSetType() = 0;
    };

    class AstQuerySet : public AstQueryExpressionBody {
    public:
        AstQuerySet();
        ~AstQuerySet();
        void                    SetSetType(SET_TYPE set_type);
        SET_TYPE                GetSetType() override;
        void                    SetLeft(AstQueryExpressionBody *left);
        AstQueryExpressionBody *GetLeft();
        void                    SetRight(AstQueryExpressionBody *right);
        AstQueryExpressionBody *GetRight();
    private:
        SET_TYPE _set_type;
        AstQueryExpressionBody *_left;
        AstQueryExpressionBody *_right;
    };

    class AstQueryPrimary : public AstQueryExpressionBody {
    public:
        enum SELECT_TYPE    { SELECT, SELECT_ALL, SELECT_DISTINCT };
        enum GROUP_TYPE     { GROUP_BY, GROUP_BY_ALL, GROUP_BY_DISTINCT };
        AstQueryPrimary();
        ~AstQueryPrimary();
        void                    SetSelectType(SELECT_TYPE select_type);
        SELECT_TYPE             GetSelectType();
        void                    SetProjectionList(const AstProjections& projection_list);
        const AstProjections&   GetProjectionList();
        void                    SetFrom(const AstTableRefs& tableref_list);
        const AstTableRefs&     GetFrom();
        void                    SetWhere(AstSearchCondition *search_condition);
        AstSearchCondition     *GetWhere();
        void                    SetGroupList(const AstGroupingElems& group_list);
        const AstGroupingElems& GetGroupList();
        void                    SetGroupType(GROUP_TYPE group_type);
        GROUP_TYPE              GetGroupType();
        void                    SetHaving(AstSearchCondition *search_condition);
        AstSearchCondition     *GetHaving();
        SET_TYPE                GetSetType() override;
    private:
        SELECT_TYPE                     _select_type;
        AstProjections                  _projection_list;           /* size at least 1 */
        AstTableRefs                    _tableref_list;             /* size 0 means NO FROM */
        AstSearchCondition             *_where_search_condition;    /* null means NO WHERE */
        AstGroupingElems                _group_list;                /* size 0 means NO GROUP BY */
        GROUP_TYPE                      _group_type;
        AstSearchCondition             *_having_search_condition;   /* null means NO HAVING */
    };

    class AstProjection : public IObject {
    public:
        AstProjection();
        ~AstProjection();
        void            SetExpr(AstRowExpr *expr);
        AstRowExpr     *GetExpr();
        void            SetAlias(AstId *alias);
        AstId          *GetAlias();
    private:
        AstRowExpr      *_expr;
        AstId           *_alias;    /* null means no alias */
    };

    class AstOrderByItem : public IObject {
    public:
        enum ORDER_TYPE { NIL, ASC, DESC };
        AstOrderByItem();
        ~AstOrderByItem();
        void            SetOrderItem(ORDER_TYPE order_type, AstRowExpr *row_expr);
        ORDER_TYPE      GetOrderType();
        AstRowExpr     *GetExpr();
    private:
        ORDER_TYPE      _order_type;
        AstRowExpr     *_expr;
    };

}

#endif