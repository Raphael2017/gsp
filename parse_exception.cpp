#include "parse_exception.h"
#include "sql_object.h"
#include <assert.h>

namespace GSP {
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
            e->_code = ParseException::FAIL;
            e->_detail = "EXPECT identifier";
            return nullptr;
        }
        AstId *id = new AstId;
        id->SetId(lex->token()->word_semantic());
        lex->next();
        return id;
    }
}