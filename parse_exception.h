#ifndef GSP_PARSE_EXCEPTION_H
#define GSP_PARSE_EXCEPTION_H

#include <string>
#include <vector>
#include "lex.h"

namespace GSP {
    struct ParseException {
        void SetFail(TokenType expect, ILex *lex);
        void SetFail(const std::vector<TokenType >& expects, ILex *lex);
        enum { SUCCESS, FAIL } _code = SUCCESS;
        std::string _detail;
    };

    class AstId;

    std::vector<AstId*>         parse_ids                   (ILex *lex, ParseException *e, TokenType separator = COMMA);
    AstId                      *parse_id                    (ILex *lex, ParseException *e);

}

#endif