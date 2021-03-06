#include "parse_tableref.h"
#include "parse_exception.h"
#include "parse_expression.h"
#include "parse_select_stmt.h"
#include "sql_table_ref.h"
#include "sql_select_stmt.h"
#include "lex.h"
#include <assert.h>

namespace GSP {

    AstTableRef                  *parse_table_primary(ILex *lex, ParseException *e);
    AstTableRef::TABLE_REF_TYPE   parse_join_type(ILex *lex, ParseException *e);

    std::vector<AstTableRef*> parse_tableref_list(ILex *lex, ParseException *e) {
        std::vector<AstTableRef*> tablerefs;
        AstTableRef *tableref = parse_tabelref(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return tablerefs;
        }
        tablerefs.push_back(tableref);
        for (; lex->token()->type() == COMMA;) {
            lex->next();
            tableref  = parse_tabelref(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                for (auto it : tablerefs) {
                    delete (it);
                }
                tablerefs.clear();
                return tablerefs;
            }
            tablerefs.push_back(tableref);
        }
        return tablerefs;
    }

    bool has_join(TokenType tkp) {
        if (tkp == CROSS || tkp == NATURAL ||
            tkp == INNER || tkp == FULL || tkp == LEFT || tkp == RIGHT || tkp == OUTER ||
            tkp == JOIN) {
            return true;
        }
        return false;
    }

    AstTableRef::TABLE_REF_TYPE parse_join_type1(TokenType full_left_right, bool has_natural, bool has_outer) {
        assert(full_left_right == FULL || full_left_right == LEFT || full_left_right == RIGHT);
        if (full_left_right == FULL) {
            if (has_natural) {
                if (has_outer) return AstTableRef::NATURAL_FULL_OUTER_JOIN;
                else return AstTableRef::NATURAL_FULL_JOIN;
            } else {
                if (has_outer) return AstTableRef::FULL_OUTER_JOIN;
                else return AstTableRef::FULL_JOIN;
            }
        } else if (full_left_right == LEFT) {
            if (has_natural) {
                if (has_outer) return AstTableRef::NATURAL_LEFT_OUTER_JOIN;
                else return AstTableRef::NATURAL_LEFT_JOIN;
            } else {
                if (has_outer) return AstTableRef::LEFT_OUTER_JOIN;
                else return AstTableRef::LEFT_JOIN;
            }
        } else {
            if (has_natural) {
                if (has_outer) return AstTableRef::NATURAL_RIGHT_OUTER_JOIN;
                else return AstTableRef::NATURAL_RIGHT_JOIN;
            } else {
                if (has_outer) return AstTableRef::RIGHT_OUTER_JOIN;
                else return AstTableRef::RIGHT_JOIN;
            }
        }
    }

    AstTableRef::TABLE_REF_TYPE  parse_join_type(ILex *lex, ParseException *e) {
        AstTableRef::TABLE_REF_TYPE join_type = AstTableRef::RELATION;
        if (lex->token()->type() == CROSS) {
            lex->next();
            join_type = AstTableRef::CROSS_JOIN;
        } else {
            bool has_natural = false;
            if (lex->token()->type() == NATURAL) {
                has_natural = true;
                lex->next();
            }
            auto jt = lex->token()->type();
            if (jt == INNER || jt == FULL || jt == LEFT || jt == RIGHT) {
                lex->next();
                switch (jt) {
                    case INNER : {
                        join_type = has_natural ? AstTableRef::NATURAL_INNER_JOIN : AstTableRef::INNER_JOIN;
                    } break;
                    case FULL: /* go through */
                    case LEFT: /* go through */
                    case RIGHT: {
                        bool has_out = false;
                        if (lex->token()->type() == OUTER) {
                            lex->next();
                            has_out = true;
                        }
                        join_type = parse_join_type1(jt, has_natural, has_out);
                    } break;
                    default: {}
                }
            } else if (jt == JOIN) {
                join_type = has_natural ? AstTableRef::NATURAL_JOIN : AstTableRef::JOIN;
            }
        }
        if (lex->token()->type() != JOIN) {
            e->SetFail(JOIN, lex);
            return join_type;
        }
        lex->next();
        return join_type;
    }

    AstTableRef *parse_tabelref(ILex *lex, ParseException *e) {
        AstTableRef *tr = parse_table_primary(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return nullptr;
        }
        for (; has_join(lex->token()->type());) {
            AstTableRef *left = tr;
            AstTableRef::TABLE_REF_TYPE join = parse_join_type(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (tr);
                return nullptr;
            }
            AstTableRef *right = parse_table_primary(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                delete (left);
                return nullptr;
            }
            if (join == AstTableRef::CROSS_JOIN || (join >= AstTableRef::NATURAL_JOIN && join <= AstTableRef::NATURAL_INNER_JOIN)) {
                tr = new AstTableJoin(join, left, right, nullptr);
            } else {
                if (lex->token()->type() != ON) {
                    delete (left);
                    delete (right);
                    e->SetFail(ON, lex);
                    return nullptr;
                }
                lex->next();
                AstSearchCondition *search_condition = parse_search_condition(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (left);
                    delete (right);
                    return nullptr;
                }
                tr = new AstTableJoin(join, left, right, search_condition);
            }
        }
        return tr;
    }

    AstTableRef *parse_table_primary(ILex *lex, ParseException *e) {
        if (lex->token()->type() == LPAREN) {
            lex->next();
            auto lex_c = lex->clone();
            AstSelectStmt *stmt = parse_select_stmt(lex_c, e);
            AstTableRef *tr = nullptr;
            if (e->_code == ParseException::SUCCESS && lex_c->token()->type() == RPAREN) {
                lex_c->next();
                lex->recover(lex_c);
                delete (lex_c);
                if (lex->token()->type() == AS) {
                    lex->next();
                }
                AstId *id = parse_id(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    delete (stmt);
                    return nullptr;
                }
                std::vector<AstId*> cols;
                if (lex->token()->type() == LPAREN) {
                    lex->next();
                    cols = parse_ids(lex, e);
                    if (e->_code != ParseException::SUCCESS) {
                        delete (stmt);
                        delete (id);
                        return nullptr;
                    }
                    if (lex->token()->type() != RPAREN) {
                        delete (stmt);
                        delete (id);
                        e->SetFail(RPAREN, lex);
                        return nullptr;
                    }
                    lex->next();
                }
                return new AstSubQueryTableRef(stmt, id, cols);
            } else {        /* backtrack */
                if (e->_code == ParseException::SUCCESS && lex_c->token()->type() != RPAREN) {
                    e->SetFail(RPAREN, lex_c);
                }
                ParseException e1 = *e;
                auto pos = lex_c->cur_pos();
                e->_code = ParseException::SUCCESS; e->_detail = "";
                tr = parse_tabelref(lex, e);
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
                    delete (tr);
                    e->SetFail(RPAREN, lex);
                    return nullptr;
                }
                lex->next();
                return tr;
            }
        }
        else {
            std::vector<AstId*> ids = parse_ids(lex, e, DOT);
            if (e->_code != ParseException::SUCCESS) {
                return nullptr;
            }
            AstId *alias = nullptr;
            if (lex->token()->type() == AS) {
                lex->next();
                alias = parse_id(lex, e);
                if (e->_code != ParseException::SUCCESS) {
                    for (auto it : ids) delete (it);
                    ids.clear();
                    return nullptr;
                }
            }
            else if (lex->token()->type() == ID) {
                alias = parse_id(lex, e);
            }
            return new AstRelation(ids, alias);
        }
    }

}