#ifndef GSP_PARSE_TABLEREF_H
#define GSP_PARSE_TABLEREF_H

#include <vector>

namespace GSP {
    class ILex;
    class ParseException;
    class AstTableRef;

    std::vector<AstTableRef*>       parse_tableref_list             (ILex *lex, ParseException *e);
    AstTableRef                    *parse_tabelref                  (ILex *lex, ParseException *e);
}

#endif