#ifndef GSP_PARSE_EXPRESSION_H
#define GSP_PARSE_EXPRESSION_H

namespace GSP {
    struct ILex;
    class ParseException;
    class AstExpr;
    typedef AstExpr                             AstSearchCondition;
    typedef AstExpr                             AstRowExpr;
    class AstExprList;

    AstSearchCondition      *parse_search_condition(ILex *lex, ParseException *e);

    AstRowExpr              *parse_row_expr(ILex *lex, ParseException *e);

    AstExprList             *parse_expr_list        (ILex *lex, ParseException *e);
}

#endif
