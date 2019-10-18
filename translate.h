#ifndef TRANSLATE_H
#define TRANSLATE_H

namespace GSP {
    class AstSelectStmt;
    class RelationAlgebraOperator;

    struct TranslateException {

    };

    RelationAlgebraOperator *translate(AstSelectStmt *query, TranslateException *e);
}

#endif