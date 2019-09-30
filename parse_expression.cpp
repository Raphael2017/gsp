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
            term = new AstBinaryOpExpr(AstSearchCondition::OR, l, r);
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
            factor = new AstBinaryOpExpr(AstSearchCondition::AND, l, r);
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
            return new AstUnaryOpExpr(AstSearchCondition::NOT, cd);
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
            primary = new AstUnaryOpExpr(expr_type, primary);
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
            AstExistsExpr *exists_expr = new AstExistsExpr(stmt);
            if (lex->token()->type() != RPAREN) {
                e->SetFail(RPAREN, lex);
                delete (exists_expr);
                return nullptr;
            }
            lex->next();
            return exists_expr;
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
                    AstQuantifiedCompareExpr *r = new AstQuantifiedCompareExpr(expr_type, row_expr, stmt);
                    return r;
                }
                else {
                    expr_type = mk_expr_type(tk1);
                    AstRowExpr *rhs = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        return nullptr;
                    }
                    return new AstBinaryOpExpr(expr_type, row_expr, rhs);
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
                return new AstUnaryOpExpr(expr_type, row_expr);
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
                    return new AstBetweenExpr(expr_type, row_expr, from, to);
                }
                else if (tk1 == IN) {
                    lex->next();
                    expr_type = has_not ? AstSearchCondition::NOT_IN : AstSearchCondition::IN;
                    AstRowExpr *in_what = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (row_expr);
                        return nullptr;
                    }
                    if (in_what->GetExprType() != AstRowExpr::EXPR_SUBQUERY &&
                            in_what->GetExprType() != AstRowExpr::EXPR_LIST) {
                        delete (row_expr);
                        delete (in_what);
                        e->_code = ParseException::FAIL;    /* todo */
                        return nullptr;
                    }
                    return new AstInExpr(expr_type, row_expr, in_what);
                }
                else if (tk1 == LIKE) {
                    lex->next();
                    expr_type = has_not ? AstSearchCondition::NOT_LIKE : AstSearchCondition::LIKE;
                    AstRowExpr *like_what = parse_row_expr(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete(row_expr);
                        return nullptr;
                    }
                    return new AstLikeExpr(expr_type, row_expr, like_what, nullptr);
                }
                else {
                    e->SetFail({BETWEEN, IN, LIKE}, lex);
                    delete (row_expr);
                    return nullptr;
                }
            } else {
                return row_expr;
            }
        }
    }

    AstRowExpr::EXPR_TYPE mk_row_expr_type(TokenType tkp) {
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
            factor0 = new AstBinaryOpExpr(mk_row_expr_type(op), left, right);
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
            factor1 = new AstBinaryOpExpr(mk_row_expr_type(tkp), left, right);
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
            primary = new AstBinaryOpExpr(AstExpr::CARET, left, right);
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
            AstRowExpr::EXPR_TYPE row_expr_type = tk1 == PLUS ? AstRowExpr::U_POSITIVE : AstRowExpr::U_NEGATIVE;
            AstRowExpr *r1 = parse_row_primary(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            return new AstUnaryOpExpr(row_expr_type, r1);
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
                AstColumnRef *col_ref = dynamic_cast<AstColumnRef*>(m);
                if (!col_ref->IsWild() && lex->token()->type() == LPAREN) {
                    std::vector<AstId*> ids= col_ref->GetColumn();
                    col_ref->SetColumn({}, false);
                    delete (col_ref);
                    lex->next();
                    AstExprList *exprs = parse_expr_list(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        for (auto it : ids) delete (it);
                        ids.clear();
                        return nullptr;
                    }
                    if (lex->token()->type() != RPAREN) {
                        for (auto it : ids) delete (it);
                        ids.clear();
                        delete (exprs);
                        e->SetFail(RPAREN, lex);
                        return nullptr;
                    }
                    lex->next();
                    return new AstFuncCall(ids, exprs);
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
                return new AstSubqueryExpr(stmt);
            } else {    /* backtrack */
                if (e->_code == ParseException::SUCCESS && lex_c->token()->type() != RPAREN) {
                    e->SetFail(RPAREN, lex_c);
                }
                auto pos = lex_c->cur_pos();
                ParseException e1 = *e;
                e->_code = ParseException::SUCCESS; e->_detail = "";
                expr_list = parse_expr_list(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    if (lex->cur_pos() < pos) {
                        *e = e1;
                        lex->recover(lex_c);
                    }
                    delete (lex_c);
                    return nullptr;
                }
                delete (lex_c);
                if (lex->token()->type() != RPAREN) {
                    delete (expr_list);
                    e->SetFail(RPAREN, lex);
                    return nullptr;
                }
                lex->next();
                AstRowExpr *r = nullptr;
                std::vector<AstSearchCondition*> scs = expr_list->GetExprs();
                assert(scs.size() > 0);
                if (scs.size() == 1) {
                    r = scs[0];
                    expr_list->SetExprs({});
                    delete (expr_list);
                } else {
                    r = expr_list;
                }
                return r;
            }
        }
        else {
            e->SetFail({LPAREN, ID, STAR, CASE, PLUS, MINUS, TRUE, FALSE, NULLX, STR_LITERAL, NUMBER, QUES}, lex);
            return nullptr;
        }
    }

    AstRowExpr *parse_constant_expr(ILex *lex, ParseException *e) {
        AstConstantValue *r = nullptr;
        if (lex->token()->type() == TRUE) {
            lex->next();
            r = new AstConstantValue(AstRowExpr::C_TRUE);
        } else if (lex->token()->type() == FALSE) {
            lex->next();
            r = new AstConstantValue(AstRowExpr::C_FALSE);
        } else if (lex->token()->type() == NULLX) {
            lex->next();
            r = new AstConstantValue(AstRowExpr::C_NULL);
        } else if (lex->token()->type() == NUMBER) {
            r = new AstConstantValue(AstRowExpr::C_NUMBER);
            r->SetValue(lex->token()->word_semantic());
            lex->next();
        } else if (lex->token()->type() == STR_LITERAL) {
            r = new AstConstantValue(AstRowExpr::C_STRING);
            r->SetValue(lex->token()->word_semantic());
            lex->next();
        } else if (lex->token()->type() == QUES) {
            lex->next();
            r = new AstConstantValue(AstRowExpr::C_QUES);
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
            return new AstColumnRef({}, true);
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
        return new AstColumnRef(ids, has_star);
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
        return new AstExprList(exprs);
    }
}