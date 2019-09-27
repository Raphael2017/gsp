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
    class AstSearchCondition;
    class AstRowExpr;
    class AstId;

    typedef AstRowExpr AstGroupingElem;

    class AstSelectStmt : public IObject {
    public:
        AstSelectStmt() : _with_clasue(nullptr), _query_expression_body(nullptr) {}
        ~AstSelectStmt() {
            delete (_with_clasue); _with_clasue = nullptr;
            for (auto it : _order_by_items) delete (it);
            _order_by_items.clear();
            delete (_query_expression_body); _query_expression_body = nullptr;
        }
        void SetWithClause(AstWithClause *with_clasue) { _with_clasue = with_clasue; }
        AstWithClause *GetWithClause() { return _with_clasue; }
        void SetOrderByItems(const std::vector<AstOrderByItem*>& order_by_items) { _order_by_items = order_by_items; }
        const std::vector<AstOrderByItem*>& GetOrderByItems() { return _order_by_items; }
        void SetBody(AstQueryExpressionBody *body) { _query_expression_body = body; }
        AstQueryExpressionBody *GetBody() { return _query_expression_body; }
    private:
        AstWithClause                   *_with_clasue;
        std::vector<AstOrderByItem*>     _order_by_items;  /* size 0 means no order by */
        AstQueryExpressionBody          *_query_expression_body;
    };

    class AstWithClause : public IObject {
    public:
        ~AstWithClause() {
            for (auto it : _ctes) delete (it);
            _ctes.clear();
        }
        enum REC_TYPE { NIL_RECURSIVE, RECURSIVE };
        void SetRecType(REC_TYPE rec_type) { _rec_type = rec_type; }
        REC_TYPE GetRecType() { return _rec_type; }
        void SetCtes(const std::vector<AstCommonTableExpr*>& ctes) { _ctes = ctes; }
        const std::vector<AstCommonTableExpr*>& GetCtes() { return _ctes; }
    private:
        REC_TYPE                            _rec_type;
        std::vector<AstCommonTableExpr*>    _ctes;
    };

    class AstCommonTableExpr : public IObject {
    public:
        AstCommonTableExpr() : _cte_name(nullptr), _query(nullptr) {}
        ~AstCommonTableExpr() {
            delete (_cte_name); _cte_name = nullptr;
            for (auto it : _cte_columns) delete (it);
            _cte_columns.clear();
            delete (_query); _query = nullptr;
        }
        void SetCteName(AstId *cte_name) { _cte_name = cte_name; }
        AstId *GetCteName() { return _cte_name; }
        void SetCteColumns(const std::vector<AstId*>& cte_columns) { _cte_columns = cte_columns; }
        const std::vector<AstId*>& GetCteColumns() { return _cte_columns; }
        void SetQuery(AstSelectStmt *query) { _query = query; }
        AstSelectStmt *GetQuery() { return _query; }
    private:
        AstId                   *_cte_name;
        std::vector<AstId*>      _cte_columns;  /* size 0 means no alias */
        AstSelectStmt           *_query;
    };

    class AstQueryExpressionBody : public IObject {
    public:
        virtual ~AstQueryExpressionBody() {}
        enum SET_TYPE { SIMPLE, UNION, UNION_ALL, UNION_DISTINCT,
            EXCEPT, EXCEPT_ALL, EXCEPT_DISTINCT,
            INTERSECT, INTERSECT_ALL, INTERSECT_DISTINCT} ;
        virtual SET_TYPE GetSetType() = 0;
    };

    class AstQuerySet : public AstQueryExpressionBody {
    public:
        AstQuerySet() : _left(nullptr), _right(nullptr) {}
        ~AstQuerySet() { delete (_left); _left = nullptr; delete (_right); _right = nullptr; }
        void SetSetType(SET_TYPE set_type) { _set_type = set_type; }
        SET_TYPE GetSetType() override { return _set_type; }
        void SetLeft(AstQueryExpressionBody *left) { _left = left; }
        AstQueryExpressionBody *GetLeft() { return _left; }
        void SetRight(AstQueryExpressionBody *right) { _right = right; }
        AstQueryExpressionBody *GetRight() { return _right; }
    private:
        SET_TYPE _set_type;
        AstQueryExpressionBody *_left;
        AstQueryExpressionBody *_right;
    };

    class AstQueryPrimary : public AstQueryExpressionBody {
    public:
        AstQueryPrimary() : _where_search_condition(nullptr), _having_search_condition(nullptr) {}
        ~AstQueryPrimary() {
            for (auto it : _projection_list) delete (it);
            _projection_list.clear();
            for (auto it : _tableref_list) delete (it);
            _tableref_list.clear();
            delete (_where_search_condition); _where_search_condition = nullptr;
            for (auto it : _group_list) delete (it);
            _group_list.clear();
            delete (_having_search_condition); _having_search_condition = nullptr;
        }
        enum SELECT_TYPE    { SELECT, SELECT_ALL, SELECT_DISTINCT };
        enum GROUP_TYPE     { GROUP_BY, GROUP_BY_ALL, GROUP_BY_DISTINCT };
        void                SetSelectType(SELECT_TYPE select_type)  { _select_type = select_type; }
        SELECT_TYPE         GetSelectType()                         { return _select_type; }
        void                SetProjectionList(const std::vector<AstProjection*>& projection_list) { _projection_list = projection_list; }
        const std::vector<AstProjection*>&
                            GetProjectionList() { return _projection_list; }
        void                SetFrom(const std::vector<AstTableRef*>& tableref_list) { _tableref_list = tableref_list; }
        const std::vector<AstTableRef*>&
                            GetFrom() { return _tableref_list; }
        void                SetWhere(AstSearchCondition *search_condition) { _where_search_condition = search_condition; }
        AstSearchCondition *GetWhere()                              { return _where_search_condition; }
        void                SetGroupList(const std::vector<AstGroupingElem*>& group_list) { _group_list = group_list; }
        const std::vector<AstGroupingElem*>&
                            GetGroupList() { return _group_list; }
        void                SetGroupType(GROUP_TYPE group_type)     { _group_type = group_type; }
        GROUP_TYPE          GetGroupType()                          { return _group_type; }
        void                SetHaving(AstSearchCondition *search_condition) { _having_search_condition = search_condition; }
        AstSearchCondition *GetHaving()                             { return _having_search_condition; }
        SET_TYPE            GetSetType() override                   { return SIMPLE; }
    private:
        SELECT_TYPE                     _select_type;
        std::vector<AstProjection*>     _projection_list;           /* size at least 1 */
        std::vector<AstTableRef*>       _tableref_list;             /* size 0 means NO FROM */
        AstSearchCondition             *_where_search_condition;    /* null means NO WHERE */
        std::vector<AstGroupingElem*>   _group_list;                /* size 0 means NO GROUP BY */
        GROUP_TYPE                      _group_type;
        AstSearchCondition             *_having_search_condition;   /* null means NO HAVING */
    };

    class AstProjection : public IObject {
    public:
        AstProjection() : _expr(nullptr), _alias(nullptr) {}
        ~AstProjection()                        { delete (_expr); _expr = nullptr; delete (_alias); _alias = nullptr; }
        void        SetExpr(AstRowExpr *expr)   { _expr = expr; }
        AstRowExpr *GetExpr()                   { return _expr; }
        void        SetAlias(AstId *alias)      { _alias = alias; }
        AstId      *GetAlias()                  { return _alias; }
    private:
        AstRowExpr      *_expr;
        AstId           *_alias;    /* null means no alias */
    };

    class AstOrderByItem : IObject {
    public:
        AstOrderByItem() : _expr(nullptr) {}
        ~AstOrderByItem()               { delete (_expr); _expr = nullptr; }
        enum        ORDER_TYPE          { NIL, ASC, DESC };
        void        SetOrderItem(ORDER_TYPE order_type, AstRowExpr *row_expr)
                                        { _order_type = order_type; _expr = row_expr; }
        ORDER_TYPE  GetOrderType()      { return _order_type; }
        AstRowExpr *GetExpr()           { return _expr; }
    private:
        ORDER_TYPE      _order_type;
        AstRowExpr     *_expr;
    };

}

#endif