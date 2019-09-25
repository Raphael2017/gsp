#include "parse_exception.h"
#include "sql_object.h"
#include "lex.h"
#include <assert.h>

namespace GSP {

    std::string token2str(TokenType tkp) {
        switch (tkp) {
            case AND:               return "AND";
            case AS:                return "AS";
            case BETWEEN:           return "BETWEEN";
            case BY:                return "BY";
            case CASE:              return "CASE";
            case FALSE:             return "FALSE";
            case ID:                return "IDENTIFIER";
            case IN:                return "IN";
            case JOIN:              return "JOIN";
            case LPAREN:            return "(";
            case MINUS:             return "-";
            case NOT:               return "NOT";
            case NULLX:         return "NULL";
            case NUMBER:        return "NUMBER";
            case ON:            return "ON";
            case PLUS:          return "+";
            case RPAREN:        return ")";
            case SELECT:        return "SELECT";
            case STAR:          return "*";
            case STR_LITERAL:   return "STRING LITERAL";
            case QUES:          return "?";
            case TRUE:          return "TRUE";
            case UNKNOWN:       return "UNKNOWN";
            default:            { assert(false); return ""; }
        }
    }

    std::string tokens2str(const std::vector<TokenType >& expects) {
        std::string r;
        int i = 0;
        for (auto it : expects) {
            if (i == 0) r += token2str(it);
            else r += (" | " + token2str(it));
            ++i;
        }
        return r;
    }

    void ParseException::SetFail(TokenType expect, ILex *lex) {
        _code = FAIL;
        //"UNEXPECT %s at %d,%d EXPECT %s";
        _detail = "UNEXPECT ";
        _detail += lex->token()->word();
        _detail += " at";
        _detail += "(" + std::to_string(lex->cur_pos_line()) + "," + std::to_string(lex->cur_pos_col()) + ") ";
        _detail += "EXPECT " + token2str(expect);
    }

    void ParseException::SetFail(const std::vector<TokenType >& expects, ILex *lex) {
        _code = FAIL;
        //"UNEXPECT %s at %d,%d EXPECT %s";
        _detail = "UNEXPECT ";
        _detail += lex->token()->word();
        _detail += " at";
        _detail += "(" + std::to_string(lex->cur_pos_line()) + "," + std::to_string(lex->cur_pos_col()) + ") ";
        _detail += "EXPECT " + tokens2str(expects);
    }

    std::vector<AstId*> parse_ids(ILex *lex, ParseException *e, TokenType separator/* = COMMA */) {
        assert( separator == COMMA || separator == DOT);
        std::vector<AstId*> ids;
        AstId *id = parse_id(lex, e);
        if (e->_code != ParseException::SUCCESS) {
            return ids;
        }
        ids.push_back(id);
        for (; lex->token()->type() == separator;) {
            lex->next();
            id = parse_id(lex, e);
            if (e->_code != ParseException::SUCCESS) {
                for (auto it : ids) delete (it);
                ids.clear();
                return ids;
            }
            ids.push_back(id);
        }
        return ids;
    }

    AstId *parse_id(ILex *lex, ParseException *e) {
        if (lex->token()->type() != ID) {
            e->SetFail(ID, lex);
            return nullptr;
        }
        AstId *id = new AstId;
        id->SetId(lex->token()->word_semantic());
        lex->next();
        return id;
    }
}