#ifndef GSP_SQL_SELECT_STMT_H
#define GSP_SQL_SELECT_STMT_H

#include <vector>
#include "sql_object.h"

namespace GSP {
    class AstWithClause;
    class AstCommonTableExpr;
    class AstQueryExpressionBody;
    class AstOrderByItem;
    class AstQueryPrimary;
    class AstProjection;
    class AstTableRef;
    class AstSearchCondition;
    //class AstGroupingElem;
    class AstRowExpr;
    //class AstLabel;
    class AstId;

    typedef AstRowExpr AstGroupingElem;
    typedef AstId   AstLabel;

    class AstSelectStmt : public IObject {
    public:
        void SetOrderByClause(const std::vector<AstOrderByItem*>& order_by_clasue) { _order_by_clause = order_by_clasue; }
        void SetWithClause(AstWithClause *with_clasue) { _with_clasue = with_clasue; }
        void SetBody(AstQueryExpressionBody *body) { _query_expression_body = body; }
        AstQueryExpressionBody *GetBody() { return _query_expression_body; }
    private:
        AstWithClause                   *_with_clasue;
        std::vector<AstOrderByItem*>     _order_by_clause;  /* size 0 means no order by */
        AstQueryExpressionBody          *_query_expression_body;
    };

    class AstWithClause : public IObject {
    public:
        enum REC_TYPE { NIL_RECURSIVE, RECURSIVE };
        void SetRecType(REC_TYPE tp) { _type = tp; }
        void SetCtes(const std::vector<AstCommonTableExpr*>& ctes) { _ctes = ctes; }
    private:
        REC_TYPE  _type;
        std::vector<AstCommonTableExpr*> _ctes;
    };

    class AstCommonTableExpr : public IObject {
    public:
        void SetCteName(AstId *cte_name) { _cte_name = cte_name; }
        void SetCteColumns(const std::vector<AstId*>& cte_columns) { _cte_columns = cte_columns; }
        void SetQuery(AstSelectStmt *query) { _query = query; }
    private:
        AstId                   *_cte_name;
        std::vector<AstId*>      _cte_columns;
        AstSelectStmt           *_query;
    };

    class AstQueryExpressionBody : public IObject {
    public:
        enum SET_TYPE { SIMPLE, UNION, UNION_ALL, UNION_DISTINCT,
                                EXCEPT, EXCEPT_ALL, EXCEPT_DISTINCT,
                                INTERSECT, INTERSECT_ALL, INTERSECT_DISTINCT} ;
        void SetSetType(SET_TYPE tp) { _set_type = tp; }
        SET_TYPE GetSetType() { return _set_type; }
        void SetLeft(AstQueryExpressionBody *left) { u._body._left = left; }
        void SetRight(AstQueryExpressionBody *right) { u._body._right = right; }
        void SetPrimary(AstQueryPrimary *primary) { u._query_primary = primary; }
        AstQueryPrimary *GetPrimary() { return u._query_primary; }
    private:
        SET_TYPE _set_type;
        union {
            struct {
                AstQueryExpressionBody *_left;
                AstQueryExpressionBody *_right;
            } _body;
            AstQueryPrimary *_query_primary;
        } u;
    };

    class AstQueryPrimary : public IObject {
    public:
        enum SELECT_TYPE { SELECT, SELECT_ALL, SELECT_DISTINCT };
        enum GROUP_TYPE { GROUP_BY, GROUP_BY_ALL, GROUP_BY_DISTINCT };
        void SetSelectType(SELECT_TYPE tp) { _sel_quantifier = tp; }
        void SetProjectionList(const std::vector<AstProjection*>& projection_list) { _projection_list = projection_list; }
        void SetFrom(const std::vector<AstTableRef*>& tableref_list) { _tableref_list = tableref_list; }
        void SetWhere(AstSearchCondition *sc) { _where_search_condition = sc; }
        void SetGroupList(const std::vector<AstGroupingElem*>& g) { _grouping_elem_list = g; }
        void SetGroupType(GROUP_TYPE g) { _group_quantifier = g; }
        void SetHaving(AstSearchCondition *sc) { _having_search_condition = sc; }
        const std::vector<AstTableRef*>& GetFrom() const { return _tableref_list; }
    private:
        SELECT_TYPE     _sel_quantifier;
        std::vector<AstProjection*>     _projection_list;           /* size at least 1 */
        std::vector<AstTableRef*>       _tableref_list;             /* size 0 means NO FROM */
        AstSearchCondition             *_where_search_condition;    /* null means NO WHERE */
        std::vector<AstGroupingElem*>   _grouping_elem_list;        /* size 0 means NO GROUP BY */
        GROUP_TYPE _group_quantifier;
        AstSearchCondition             *_having_search_condition;   /* null means NO HAVING */
    };

    class AstProjection : public IObject {
    public:
        void SetExpr(AstRowExpr *expr) { _expr = expr; }
        void SetLable(AstLabel *label) { _label = label; }
    private:
        AstRowExpr      *_expr;
        AstLabel        *_label;    /* null means no label */
    };

    class AstOrderByItem : IObject {
    public:
        enum ORDER_TYPE { NIL, ASC, DESC };
        void SetOrder(ORDER_TYPE q, AstRowExpr *row_expr) { _quantifier = q; _expr = row_expr; }
    private:
        ORDER_TYPE _quantifier;
        AstRowExpr  *_expr;
    };

}

#endif