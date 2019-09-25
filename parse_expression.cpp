#include "parse_expression.h"
#include "sql_expression.h"
#include "lex.h"
#include "parse_exception.h"
#include "parse_select_stmt.h"
#include "sql_select_stmt.h"
#include "assert.h"


namespace GSP {

    AstSearchCondition      *parse_boolean_term     (ILex *lex, ParseException *e);
    AstSearchCondition      *parse_boolean_factor   (ILex *lex, ParseException *e);
    AstSearchCondition      *parse_boolean_test     (ILex *lex, ParseException *e);
    AstSearchCondition      *parse_boolean_primary  (ILex *lex, ParseException *e);

    AstRowExpr              *parse_factor0          (ILex *lex, ParseException *e);
    AstRowExpr              *parse_factor1          (ILex *lex, ParseException *e);
    AstRowExpr              *parse_row_primary      (ILex *lex, ParseException *e);

    AstRowExpr              *parse_case_expr        (ILex *lex, ParseException *e);
    AstRowExpr              *parse_constant_expr    (ILex *lex, ParseException *e);
    AstRowExpr              *parse_columnref_expr   (ILex *lex, ParseException *e);



    AstSearchCondition *parse_search_condition(ILex *lex, ParseException *e) {
        AstSearchCondition *term = parse_boolean_term(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        for (; lex->token()->type() == OR; ) {
            lex->next();
            AstSearchCondition *l = term;
            AstSearchCondition *r = parse_boolean_term(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (term);
                return nullptr;
            }
            term = new AstSearchCondition;
            term->SetExprType(AstSearchCondition::OR);
            term->SetLeft(l);
            term->SetRight(r);
        }
        return term;
    }

    AstSearchCondition *parse_boolean_term(ILex *lex, ParseException *e) {
        AstSearchCondition *factor = parse_boolean_factor(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        for (; lex->token()->type() == AND; ) {
            lex->next();
            AstSearchCondition *l = factor;
            AstSearchCondition *r = parse_boolean_factor(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (factor);
                return nullptr;
            }
            factor = new AstSearchCondition;
            factor->SetExprType(AstSearchCondition::AND);
            factor->SetLeft(l);
            factor->SetRight(r);
        }
        return factor;
    }

    AstSearchCondition *parse_boolean_factor(ILex *lex, ParseException *e) {
        if (lex->token()->type() == NOT) {
            lex->next();
            AstSearchCondition *cd = parse_boolean_factor(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            AstSearchCondition *r = new AstSearchCondition;
            r->SetExprType(AstSearchCondition::NOT);
            r->SetSc(cd);
            return r;
        }
        return parse_boolean_test(lex, e);
    }

    AstSearchCondition *parse_boolean_test(ILex *lex, ParseException *e) {
        AstSearchCondition *primary = parse_boolean_primary(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        if (lex->token()->type() == IS) {
            lex->next();
            bool has_not = false;
            AstSearchCondition::EXPR_TYPE expr_type = AstSearchCondition::NOT;
            if (lex->token()->type() == NOT) {
                lex->next();
                has_not = true;
            }
            if (lex->token()->type() == TRUE) {
                lex->next();
                expr_type = has_not ? AstSearchCondition::IS_NOT_TRUE : AstSearchCondition::IS_TRUE;
            }
            else if (lex->token()->type() == FALSE) {
                lex->next();
                expr_type = has_not ? AstSearchCondition::IS_NOT_FALSE : AstSearchCondition::IS_FALSE;
            }
            else if (lex->token()->type() == UNKNOWN) {
                lex->next();
                expr_type = has_not ? AstSearchCondition::IS_NOT_UNKNOWN : AstSearchCondition::IS_UNKNOWN;
            }
            else {
                delete (primary);
                e->SetFail({NOT, TRUE, FALSE, UNKNOWN}, lex);
                return nullptr;
            }
            AstSearchCondition *t = primary;
            primary = new AstSearchCondition;
            primary->SetExprType(expr_type);
            primary->SetSc(t);
        }
        return primary;
    }

    AstSearchCondition::EXPR_TYPE mk_expr_type(TokenType tk1) {
        assert(tk1 == LTEQ || tk1 == LT || tk1 == GTEQ || tk1 == GT || tk1 == EQ || tk1 == LTGT);
        if (tk1 == LTEQ) return AstSearchCondition::COMP_LE;
        else if (tk1 == LT) return AstSearchCondition::COMP_LT;
        else if (tk1 == GTEQ) return AstSearchCondition::COMP_GE;
        else if (tk1 == GT) return AstSearchCondition::COMP_GT;
        else if (tk1 == EQ) return AstSearchCondition::COMP_EQ;
        else return AstSearchCondition::COMP_NEQ;
    }

    AstSearchCondition::EXPR_TYPE mk_expr_type(TokenType tk1, TokenType all_some_any) {
        assert(tk1 == LTEQ || tk1 == LT || tk1 == GTEQ || tk1 == GT || tk1 == EQ || tk1 == LTGT);
        assert(all_some_any == ALL || all_some_any == SOME || all_some_any == ANY);
        if (tk1 == LTEQ) {
            if (all_some_any == ALL) return AstSearchCondition::COMP_LE_ALL;
            else if (all_some_any == SOME) return AstSearchCondition::COMP_LE_SOME;
            else return AstSearchCondition::COMP_LE_ANY;
        }
        else if (tk1 == LT) {
            if (all_some_any == ALL) return AstSearchCondition::COMP_LT_ALL;
            else if (all_some_any == SOME) return AstSearchCondition::COMP_LT_SOME;
            else return AstSearchCondition::COMP_LT_ANY;
        }
        else if (tk1 == GTEQ) {
            if (all_some_any == ALL) return AstSearchCondition::COMP_GE_ALL;
            else if (all_some_any == SOME) return AstSearchCondition::COMP_GE_SOME;
            else return AstSearchCondition::COMP_GE_ANY;
        }
        else if (tk1 == GT) {
            if (all_some_any == ALL) return AstSearchCondition::COMP_GT_ALL;
            else if (all_some_any == SOME) return AstSearchCondition::COMP_GT_SOME;
            else return AstSearchCondition::COMP_GT_ANY;
        }
        else if (tk1 == EQ) {
            if (all_some_any == ALL) return AstSearchCondition::COMP_EQ_ALL;
            else if (all_some_any == SOME) return AstSearchCondition::COMP_EQ_SOME;
            else return AstSearchCondition::COMP_EQ_ANY;
        }
        else {  /* LTGT */
            if (all_some_any == ALL) return AstSearchCondition::COMP_NEQ_ALL;
            else if (all_some_any == SOME) return AstSearchCondition::COMP_NEQ_SOME;
            else return AstSearchCondition::COMP_NEQ_ANY;
        }
    }

    AstSearchCondition *parse_boolean_primary(ILex *lex, ParseException *e) {
        if (lex->token()->type() == EXISTS) {
            lex->next();
            if (lex->token()->type() != LPAREN) {
                e->SetFail(LPAREN, lex);
                return nullptr;
            }
            lex->next();
            AstSelectStmt *stmt = parse_select_stmt(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            AstSearchCondition *r = new AstSearchCondition;
            r->SetExprType(AstSearchCondition::EXISTS);
            r->SetExist(stmt);
            if (lex->token()->type() != RPAREN) {
                e->SetFail(RPAREN, lex);
                delete (r);
                return nullptr;
            }
            lex->next();
            return r;
        }
        else {
            AstRowExpr *row_expr = parse_row_expr(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            AstSearchCondition::EXPR_TYPE expr_type = AstSearchCondition::BETWEEN;
            auto tk1 = lex->token()->type();
            if (tk1 == LTEQ || tk1 == LT || tk1 == GTEQ || tk1 == GT || tk1 == EQ || tk1 == LTGT) {
                lex->next();
                auto all_some_any = lex->token()->type();
                if (all_some_any == ALL || all_some_any == SOME || all_some_any == ANY) {
                    lex->next();
                    expr_type = mk_expr_type(tk1, all_some_any);
                    if (lex->token()->type() != LPAREN) {
                        e->SetFail(LPAREN, lex);
                        delete (row_expr);
                        return nullptr;
                    }
                    AstSelectStmt *stmt = parse_select_stmt(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        return nullptr;
                    }
                    if (lex->token()->type() != RPAREN) {
                        delete (row_expr);
                        delete (stmt);
                        e->SetFail(RPAREN, lex);
                        return nullptr;
                    }
                    AstSearchCondition *r = new AstSearchCondition;
                    r->SetExprType(expr_type);
                    r->SetRowExpr1(row_expr);
                    r->SetExist(stmt);
                    return r;
                }
                else {
                    expr_type = mk_expr_type(tk1);
                    AstRowExpr *rhs = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        return nullptr;
                    }
                    AstSearchCondition *r = new AstSearchCondition;
                    r->SetExprType(expr_type);
                    r->SetRowExpr3(row_expr, rhs, nullptr);
                    return r;
                }


            }
            else if (tk1 == IS) {
                lex->next();
                bool has_not = false;
                if (lex->token()->type() == NOT) {
                    lex->next();
                    has_not = true;
                }
                if (lex->token()->type() != NULLX) {
                    e->SetFail(NULLX, lex);
                    delete (row_expr);
                    return nullptr;
                }
                expr_type = has_not ? AstSearchCondition::IS_NOT_NULL : AstSearchCondition::IS_NULL;
                AstSearchCondition *r = new AstSearchCondition;
                r->SetExprType(expr_type);
                r->SetRowExpr1(row_expr);
                return r;
            }
            else if (tk1 == NOT || tk1 == BETWEEN || tk1 == IN || tk1 == LIKE) {
                bool has_not = false;
                if (tk1 == NOT) {
                    lex->next();
                    tk1 = lex->token()->type();
                    has_not = true;
                }
                if (tk1 == BETWEEN) {
                    lex->next();
                    expr_type = has_not ? AstSearchCondition::NOT_BETWEEN : AstSearchCondition::BETWEEN;
                    AstRowExpr *from = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        return nullptr;
                    }
                    if (lex->token()->type() != AND) {
                        delete (row_expr);
                        e->SetFail(AND, lex);
                        return nullptr;
                    }
                    lex->next();
                    AstRowExpr *to = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        delete (from);
                        return nullptr;
                    }
                    AstSearchCondition *r = new AstSearchCondition;
                    r->SetExprType(expr_type);
                    r->SetRowExpr3(row_expr, from, to);
                    return r;
                }
                else if (tk1 == IN) {
                    lex->next();
                    expr_type = has_not ? AstSearchCondition::NOT_IN : AstSearchCondition::IN;
                    AstRowExpr *in_what = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        return nullptr;
                    }
                    if (in_what->GetRowType() != AstRowExpr::SUBQUERY &&
                            in_what->GetRowType() != AstRowExpr::SC_LIST) {
                        delete (row_expr);
                        delete (in_what);
                        e->_code = ParseException::FAIL;    /* todo */
                        return nullptr;
                    }
                    AstSearchCondition *r = new AstSearchCondition;
                    r->SetExprType(expr_type);
                    r->SetRowExpr3(row_expr, in_what, nullptr);
                    return r;
                }
                else if (tk1 == LIKE) {
                    lex->next();
                    expr_type = has_not ? AstSearchCondition::NOT_LIKE : AstSearchCondition::LIKE;
                    AstRowExpr *like_what = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete(row_expr);
                        return nullptr;
                    }
                    AstSearchCondition *r = new AstSearchCondition;
                    r->SetExprType(expr_type);
                    r->SetRowExpr3(row_expr, like_what, nullptr);
                    return r;
                }
                else {
                    e->SetFail({BETWEEN, IN, LIKE}, lex);
                    delete (row_expr);
                    return nullptr;
                }
            } else {
                AstSearchCondition *r = new AstSearchCondition;
                r->SetExprType(AstSearchCondition::ROW_EXPR);
                r->SetRowExpr1(row_expr);
                return r;
            }
        }
    }

    AstRowExpr::ROW_EXPR_TYPE mk_row_expr_type(TokenType tkp) {
        switch (tkp) {
            case PLUS : return AstRowExpr::PLUS;
            case MINUS : return AstRowExpr::MINUS;
            case BARBAR : return AstRowExpr::BARBAR;
            case STAR : return AstRowExpr::MUL;
            case DIVIDE : return AstRowExpr::DIV;
            case PERCENT : return AstRowExpr::REM;
            default: {assert(false);}
        }
        return AstRowExpr::PLUS;
    }

    AstRowExpr *parse_row_expr(ILex *lex, ParseException *e) {
        AstRowExpr *factor0 = parse_factor0(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        auto op = lex->token()->type();
        for (; op == PLUS || op == MINUS || op == BARBAR; op = lex->token()->type()) {
            lex->next();
            AstRowExpr *left = factor0;
            AstRowExpr *right = parse_factor0(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (factor0);
                return nullptr;
            }
            factor0 = new AstRowExpr;
            factor0->SetRowType(mk_row_expr_type(op));
            factor0->SetLeft(left);
            factor0->SetRight(right);
        }
        return factor0;
    }

    AstRowExpr *parse_factor0(ILex *lex, ParseException *e) {
        AstRowExpr *factor1 = parse_factor1(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        auto tkp = lex->token()->type();
        for (; tkp == STAR || tkp == DIVIDE || tkp == PERCENT; tkp = lex->token()->type()) {
            lex->next();
            AstRowExpr *left = factor1;
            AstRowExpr *right = parse_factor1(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (factor1);
                return nullptr;
            }
            factor1 = new AstRowExpr;
            factor1->SetRowType(mk_row_expr_type(tkp));
            factor1->SetLeft(left);
            factor1->SetRight(right);
        }
        return factor1;
    }

    AstRowExpr *parse_factor1(ILex *lex, ParseException *e) {
        AstRowExpr *primary = parse_row_primary(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        auto op = lex->token()->type();
        for (; op == CARET; op = lex->token()->type()) {
            lex->next();
            AstRowExpr *left = primary;
            AstRowExpr *right = parse_row_primary(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (primary);
                return nullptr;
            }
            primary = new AstRowExpr;
            primary->SetRowType(AstRowExpr::CARET);
            primary->SetLeft(left);
            primary->SetRight(right);
        }
        return primary;
    }

    AstRowExpr *parse_row_primary(ILex *lex, ParseException *e) {
        auto tk1 = lex->token()->type();
        if (tk1 == TRUE || tk1 == FALSE || tk1 == NULLX || tk1 == STR_LITERAL || tk1 == NUMBER || tk1 == QUES) {
            return parse_constant_expr(lex, e);
        }
        else if (tk1 == PLUS || tk1 == MINUS) {
            lex->next();
            AstRowExpr::ROW_EXPR_TYPE row_expr_type = tk1 == PLUS ? AstRowExpr::U_PLUS : AstRowExpr::U_MINUS;
            AstRowExpr *r1 = parse_row_primary(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            AstRowExpr *r = new AstRowExpr;
            r->SetRowType(row_expr_type);
            r->SetUnaryExpr(r1);
            return r;
        }
        else if (tk1 == CASE) {
            return parse_case_expr(lex, e);
        }
        else if (tk1 == ID || tk1 == STAR) {
            if (tk1 == STAR) return parse_columnref_expr(lex, e);
            else {
                AstRowExpr *m = parse_columnref_expr(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    return nullptr;
                }
                assert(m->GetRowType() == AstRowExpr::COLUMN_REF);
                if (!m->GetColumnRef()->IsWild() && lex->token()->type() == LPAREN) {
                    std::vector<AstId*> ids= m->GetColumnRef()->GetColumn();
                    m->GetColumnRef()->SetColumn({}, false);
                    delete (m);
                    lex->next();
                    AstExprList *exprs = nullptr;
                    if (lex->token()->type() != RPAREN) {
                        exprs = parse_expr_list(lex, e);
                        if (e->_code != ParseException::SUCCESS) {
                            for (auto it : ids) delete (it);
                            ids.clear();
                            return nullptr;
                        }
                    }
                    if (lex->token()->type() != RPAREN) {
                        for (auto it : ids) delete (it);
                        ids.clear();
                        delete (exprs);
                        e->SetFail(RPAREN, lex);
                        return nullptr;
                    }
                    lex->next();
                    AstRowExpr *r = new AstRowExpr;
                    r->SetRowType(AstRowExpr::FUNC_CALL);
                    AstFuncCall *f = new AstFuncCall;
                    r->SetFunc(f);
                    f->SetFuncName(ids); f->SetParams(exprs);
                    return r;
                }
                else return m;
            }
        }
        else if (tk1 == LPAREN) {
            lex->next();
            ILex *lex_c = lex->clone();
            AstSelectStmt *stmt = nullptr;
            AstExprList *expr_list = nullptr;
            stmt = parse_select_stmt(lex_c, e);
            if (e->_code == ParseException::SUCCESS && lex_c->token()->type() == RPAREN) {
                lex_c->next();
                lex->recover(lex_c);
                delete (lex_c);
                AstRowExpr *r = new AstRowExpr;
                r->SetRowType(AstRowExpr::SUBQUERY);
                r->SetQuery(stmt);
                return r;
            } else {    /* backtrack */
                delete (lex_c);
                e->_code = ParseException::SUCCESS; e->_detail = "";
                expr_list = parse_expr_list(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    return nullptr;
                }
                if (lex->token()->type() != RPAREN) {
                    delete (expr_list);
                    e->SetFail(RPAREN, lex);
                    return nullptr;
                }
                lex->next();
                AstRowExpr *r = new AstRowExpr;
                r->SetRowType(AstRowExpr::SC_LIST);
                r->SetExprList(expr_list);
                return r;
            }
        }
        else {
            e->SetFail({LPAREN, ID, STAR, CASE, PLUS, MINUS, TRUE, FALSE, NULLX, STR_LITERAL, NUMBER, QUES}, lex);
            return nullptr;
        }
    }

    AstRowExpr *parse_constant_expr(ILex *lex, ParseException *e) {
        AstRowExpr *r = new AstRowExpr;
        if (lex->token()->type() == TRUE) {
            lex->next();
            r->SetRowType(AstRowExpr::C_TRUE);
        } else if (lex->token()->type() == FALSE) {
            lex->next();
            r->SetRowType(AstRowExpr::C_FALSE);
        } else if (lex->token()->type() == NULLX) {
            lex->next();
            r->SetRowType(AstRowExpr::C_NULL);
        } else if (lex->token()->type() == NUMBER) {
            r->SetRowType(AstRowExpr::C_VALUE);
            AstConstantValue *c = new AstConstantValue;
            c->SetConstantType(AstConstantValue::C_NUMBER);
            c->SetValue(lex->token()->word_semantic());
            r->SetConstantValue(c);
            lex->next();
        } else if (lex->token()->type() == STR_LITERAL) {
            r->SetRowType(AstRowExpr::C_VALUE);
            AstConstantValue *c = new AstConstantValue;
            c->SetConstantType(AstConstantValue::C_STRING);
            c->SetValue(lex->token()->word_semantic());
            r->SetConstantValue(c);
            lex->next();
        } else if (lex->token()->type() == QUES) {
            lex->next();
            r->SetRowType(AstRowExpr::C_QUES);
        } else {
            assert(false);
        }
        return r;
    }

    AstRowExpr *parse_case_expr(ILex *lex, ParseException *e) {
        assert(false);
        return nullptr;
    }

    AstRowExpr *parse_columnref_expr(ILex *lex, ParseException *e) {
        if (lex->token()->type() == STAR) {
            lex->next();
            AstColumnRef *r = new AstColumnRef;
            r->SetColumn({}, true);
            AstRowExpr *re = new AstRowExpr;
            re->SetRowType(AstRowExpr::COLUMN_REF);
            re->SetColumnRef(r);
            return re;
        }

        std::vector<AstId*> ids;
        AstId *id = parse_id(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        ids.push_back(id);
        bool has_star = false;
        for (; lex->token()->type() == DOT;) {
            lex->next();
            if (lex->token()->type() == STAR) {
                lex->next();
                has_star = true;
                break;
            } else {
                id = parse_id(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    for (auto it : ids) delete (it);
                    ids.clear();
                    return nullptr;
                }
                ids.push_back(id);
            }
        }
        AstColumnRef *r = new AstColumnRef;
        r->SetColumn(ids, has_star);
        AstRowExpr *re = new AstRowExpr;
        re->SetRowType(AstRowExpr::COLUMN_REF);
        re->SetColumnRef(r);
        return re;
    }

    AstExprList *parse_expr_list(ILex *lex, ParseException *e) {
        std::vector<AstSearchCondition*> exprs;
        AstSearchCondition *expr = parse_search_condition(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        exprs.push_back(expr);
        for (; lex->token()->type() == COMMA;) {
            lex->next();
            expr = parse_search_condition(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                for (auto it : exprs) delete (it);
                exprs.clear();
                return nullptr;
            }
            exprs.push_back(expr);
        }
        AstExprList *r = new AstExprList;
        r->SetExprs(exprs);
        return r;
    }
}