#include <assert.h>
#include "parse_select_stmt.h"
#include "parse_expression.h"
#include "parse_tableref.h"
#include "sql_select_stmt.h"
#include "sql_expression.h"
#include "sql_table_ref.h"
#include "parse_exception.h"
#include "lex.h"

namespace GSP {

    AstSelectStmt           *parse_query_expression         (ILex *lex, ParseException *e);
    AstOrderByItem          *parse_order_by_item            (ILex *lex, ParseException *e);
    AstWithClause           *parse_with_clause              (ILex *lex, ParseException *e);

    AstQueryExpressionBody  *parse_query_expression_body    (ILex *lex, ParseException *e);
    AstQueryExpressionBody  *parse_query_term               (ILex *lex, ParseException *e);
    AstQueryExpressionBody  *parse_query_primary            (ILex *lex, ParseException *e);

    AstProjections                      parse_projection_list           (ILex *lex, ParseException *e);
    AstProjection                      *parse_projection                (ILex *lex, ParseException *e);
    AstGroupingElems                    parse_groupby_list              (ILex *lex, ParseException *e);
    AstCommonTableExprs                 parse_ctes_list                 (ILex *lex, ParseException *e);
    AstCommonTableExpr                 *parse_cte                       (ILex *lex, ParseException *e);


    AstSelectStmt *parse_select_stmt(ILex *lex, ParseException *e) {
        AstSelectStmt *select_stmt = parse_query_expression(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        if (lex->token()->type() == ORDER) {
            lex->next();
            if (lex->token()->type() != BY) {
                e->SetFail(BY, lex);
                return nullptr;
            }
            lex->next();
            AstOrderByItems items;
            AstOrderByItem *order_by_item = parse_order_by_item(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (select_stmt);
                return nullptr;
            }
            items.push_back(order_by_item);
            for (; lex->token()->type() == COMMA; ) {
                lex->next();
                order_by_item = parse_order_by_item(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (select_stmt);
                    for (auto it : items) {
                        delete (it);
                    }
                    items.clear();
                    return nullptr;
                }
                items.push_back(order_by_item);
            }
            select_stmt->SetOrderByItems(items);
        }
        return select_stmt;
    }

    AstSelectStmt *parse_query_expression(ILex *lex, ParseException *e) {
        AstWithClause *with_clause = nullptr;
        if (lex->token()->type() == WITH) {
            with_clause = parse_with_clause(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
        }
        AstQueryExpressionBody *body = parse_query_expression_body(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            delete (with_clause);
            return nullptr;
        }

        return new AstSelectStmt(with_clause, body, {});
    }

    AstQueryExpressionBody::SET_TYPE mk_set_type(TokenType tk1, TokenType tk2) {
        assert(tk1 == UNION || tk1 == EXCEPT || tk1 == INTERSECT);
        if (tk1 == UNION) {
            if (tk2 == ALL) return AstQueryExpressionBody::UNION_ALL;
            else if (tk2 == DISTINCT) return AstQueryExpressionBody::UNION_DISTINCT;
            else return AstQueryExpressionBody::UNION;
        }
        else if (tk1 == EXCEPT) {
            if (tk2 == ALL) return AstQueryExpressionBody::EXCEPT_ALL;
            else if (tk2 == DISTINCT) return AstQueryExpressionBody::EXCEPT_DISTINCT;
            else return AstQueryExpressionBody::EXCEPT;
        }
        else {
            if (tk2 == ALL) return AstQueryExpressionBody::INTERSECT_ALL;
            else if (tk2 == DISTINCT) return AstQueryExpressionBody::INTERSECT_DISTINCT;
            else return AstQueryExpressionBody::INTERSECT;
        }
    }

    AstQueryExpressionBody *parse_query_expression_body(ILex *lex, ParseException *e) {
        AstQueryExpressionBody *query_term = parse_query_term(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }

        auto tkp = lex->token()->type();
        for (; tkp == UNION || tkp == EXCEPT; tkp = lex->token()->type()) {
            lex->next();
            AstQueryExpressionBody *left = query_term;
            auto all_distinct = lex->token()->type();
            auto set_type = mk_set_type(tkp, all_distinct);
            if (all_distinct == ALL || all_distinct == DISTINCT) {
                lex->next();
            }
            AstQueryExpressionBody *right = parse_query_term(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (left);
                return nullptr;
            }
            query_term = new AstQuerySet(set_type, left, right);
        }
        return query_term;
    }

    AstQueryExpressionBody *parse_query_term(ILex *lex, ParseException *e) {
        // primary
        AstQueryExpressionBody *primary = parse_query_primary(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        TokenType tkp = lex->token()->type();
        for (; tkp == INTERSECT; tkp = lex->token()->type()) {
            lex->next();
            AstQueryExpressionBody *left = primary;
            auto all_distinct = lex->token()->type();
            auto set_type = mk_set_type(tkp, all_distinct);
            if (all_distinct == ALL || all_distinct == DISTINCT) {
                lex->next();
            }
            AstQueryExpressionBody *right = parse_query_primary(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (left);
                return nullptr;
            }
            primary = new AstQuerySet(set_type, left, right);
        }
        return primary;
    }

    AstQueryPrimary::SELECT_TYPE mk_select_type(TokenType tkp) {
        if (tkp == ALL) return AstQueryPrimary::SELECT_ALL;
        else if (tkp == DISTINCT) return AstQueryPrimary::SELECT_DISTINCT;
        else return AstQueryPrimary::SELECT;
    }

    AstQueryPrimary::GROUP_TYPE  mk_group_type(TokenType tkp) {
        if (tkp == ALL) return AstQueryPrimary::GROUP_BY_ALL;
        else if (tkp == DISTINCT) return AstQueryPrimary::GROUP_BY_DISTINCT;
        else return AstQueryPrimary::GROUP_BY;
    }

    AstQueryExpressionBody *parse_query_primary(ILex *lex, ParseException *e) {
        AstQueryExpressionBody *r = nullptr;
        if (lex->token()->type() == SELECT) {
            AstQueryPrimary *primary = nullptr;
            lex->next();
            auto all_distinct = lex->token()->type();
            primary = new AstQueryPrimary(mk_select_type(all_distinct));
            if (all_distinct == ALL || all_distinct == DISTINCT) {
                lex->next();
            }
            AstProjections  projection_list = parse_projection_list(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (primary);
                return nullptr;
            }
            primary->SetProjectionList(projection_list);
            AstTableRefs tableref_list;
            if (lex->token()->type() == FROM) {
                lex->next();
                tableref_list = parse_tableref_list(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (primary);
                    return nullptr;
                }
                primary->SetFrom(tableref_list);
            }
            if (lex->token()->type() == WHERE) {
                lex->next();
                AstSearchCondition *where = parse_search_condition(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (primary);
                    return nullptr;
                }
                primary->SetWhere(where);
            }
            if (lex->token()->type() == GROUP) {
                lex->next();
                if (lex->token()->type() != BY) {
                    delete (primary);
                    return nullptr;
                }
                lex->next();
                TokenType tk1 = lex->token()->type();
                primary->SetGroupType(mk_group_type(tk1));
                if (tk1 == ALL || tk1 == DISTINCT) {
                    lex->next();
                }
                AstGroupingElems group_by = parse_groupby_list(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (primary);
                    return nullptr;
                }
                primary->SetGroupList(group_by);
            }
            if (lex->token()->type() == HAVING) {
                lex->next();
                AstSearchCondition *having = parse_search_condition(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (primary);
                    return nullptr;
                }
                primary->SetHaving(having);
            }
            r = primary;
        }
        else if (lex->token()->type() == LPAREN) {
            lex->next();
            AstQueryExpressionBody *body = parse_query_expression_body(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            if (lex->token()->type() != RPAREN) {
                delete (body);
                e->SetFail(RPAREN, lex);
                return nullptr;
            }
            lex->next();    // skip ')'
            r = body;
        }
        else {
            e->SetFail({SELECT, LPAREN}, lex);
            return nullptr;
        }
        return r;
    }

    AstWithClause *parse_with_clause(ILex *lex, ParseException *e) {
        assert(lex->token()->type() == WITH);
        lex->next();
        auto has_recursive = AstWithClause::NIL_RECURSIVE;
        if (lex->token()->type() == RECURSIVE) {
            lex->next();
            has_recursive = AstWithClause::RECURSIVE;
        }
        AstCommonTableExprs ctes = parse_ctes_list(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        return new AstWithClause(has_recursive, ctes);
    }

    AstCommonTableExprs parse_ctes_list(ILex *lex, ParseException *e) {
        AstCommonTableExprs ctes;
        AstCommonTableExpr *cte = parse_cte(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return ctes;
        }
        ctes.push_back(cte);
        for (; lex->token()->type() == COMMA;) {
            lex->next();
            cte = parse_cte(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                for (auto it : ctes) {
                    delete (it);
                }
                ctes.clear();
                return ctes;
            }
            ctes.push_back(cte);
        }
        return ctes;
    }

    AstCommonTableExpr *parse_cte(ILex *lex, ParseException *e) {
        if (lex->token()->type() != ID) {
            e->SetFail(ID, lex);
            return nullptr;
        }
        AstId *name = parse_id(lex, e);
        AstCommonTableExpr *cte = new AstCommonTableExpr(name);
        if (lex->token()->type() == LPAREN) {
            lex->next();
            AstIds columns = parse_ids(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (cte);
                return nullptr;
            }
            cte->SetCteColumns(columns);
            if (lex->token()->type() != RPAREN) {
                delete (cte);
                return nullptr;
            }
            lex->next();
        }
        if (lex->token()->type() != AS ) {
            delete (cte);
            e->SetFail(AS, lex);
            return nullptr;
        }
        lex->next();
        AstSelectStmt *query = parse_query_expression(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            delete (cte);
            return nullptr;
        }
        cte->SetQuery(query);
        return cte;
    }

    AstProjections parse_projection_list(ILex *lex, ParseException *e) {
        AstProjections projections;
        AstProjection *projection = parse_projection(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return projections;
        }
        projections.push_back(projection);
        for (; lex->token()->type() == COMMA;) {
            lex->next();
            projection = parse_projection(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                for (auto it : projections) {
                    delete (it);
                }
                projections.clear();
                return projections;
            }
            projections.push_back(projection);
        }
        return projections;
    }

    AstProjection *parse_projection(ILex *lex, ParseException *e) {
        AstRowExpr *row_expr = parse_row_expr(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        AstProjection *proj = new AstProjection(row_expr, nullptr);
        if (row_expr->GetExprType() == AstRowExpr::EXPR_COLUMN_REF &&
            dynamic_cast<AstColumnRef*>(row_expr)->GetColumn().size() == 0 &&
            dynamic_cast<AstColumnRef*>(row_expr)->IsWild()) /* here means only * */ {
            return proj;
        }
        if (lex->token()->type() == AS) {
            lex->next();
            if (lex->token()->type() != ID) {
                e->SetFail(ID, lex);
                delete (proj);
                return nullptr;
            }
        }
        if (lex->token()->type() == ID) {
            AstId *label = parse_id(lex, e);
            proj->SetAlias(label);
        }
        return proj;
    }

    AstOrderByItem *parse_order_by_item(ILex *lex, ParseException *e) {
        AstRowExpr *expr = parse_row_expr(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        AstOrderByItem::ORDER_TYPE ot = AstOrderByItem::NIL;
        if (lex->token()->type() == ASC) {
            lex->next();
            ot = AstOrderByItem::ASC;
        } else if (lex->token()->type() == DESC) {
            lex->next();
            ot = AstOrderByItem::DESC;
        } else {
            ot = AstOrderByItem::NIL;
        }
        return new AstOrderByItem(ot, expr);
    }

    AstGroupingElems parse_groupby_list(ILex *lex, ParseException *e) {
        AstGroupingElems exprs;
        AstRowExpr *row_expr = parse_row_expr(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return exprs;
        }
        exprs.push_back(row_expr);
        for (; lex->token()->type() == COMMA; ) {
            lex->next();
            row_expr = parse_row_expr(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                for (auto it : exprs) delete (it);
                exprs.clear();
                return exprs;
            }
            exprs.push_back(row_expr);
        }
        return exprs;
    }

}