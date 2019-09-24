#ifndef GSP_PARSE_SELECT_STMT_H
#define GSP_PARSE_SELECT_STMT_H

namespace GSP {
    struct ILex;
    class AstSelectStmt;

    class ParseException;

    AstSelectStmt *parse_select_stmt(ILex *lex, ParseException *e);
}

#endif