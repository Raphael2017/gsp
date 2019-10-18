#include "translate.h"
#include "sql_select_stmt.h"
#include "sql_table_ref.h"
#include "sql_expression.h"
#include "relational_algebra.h"
#include <assert.h>

namespace GSP {
    RelationAlgebraOperator *translate_query_body(AstQueryExpressionBody *body, TranslateException *e);
    RelationAlgebraOperator *translate_query_set(AstQuerySet *body, TranslateException *e);
    RelationAlgebraOperator *translate_query_primary(AstQueryPrimary *primary, TranslateException *e);
    RelationAlgebraOperator *translate_table_ref(AstTableRef *table_ref, TranslateException *e);

    RelationAlgebraOperator *translate(AstSelectStmt *query, TranslateException *e) {
        /* todo with clause */
        RelationAlgebraOperator *ra_query = nullptr;
        ra_query = translate_query_body(query->GetBody(), e);
        /* todo order by */
        return ra_query;
    }

    RelationAlgebraOperator *translate_query_body(AstQueryExpressionBody *body, TranslateException *e) {
        switch (body->GetSetType()) {
            case AstQueryExpressionBody::SIMPLE: {
                return translate_query_primary(dynamic_cast<AstQueryPrimary*>(body), e);
            } break;
            case AstQueryExpressionBody::UNION_ALL: {
                return translate_query_set(dynamic_cast<AstQuerySet*>(body), e);
            } break;
            default: { assert(false); }
        }
    }

    RelationAlgebraOperator *translate_query_set(AstQuerySet *body, TranslateException *e) {
        RelationAlgebraUnion *un = new RelationAlgebraUnion;
        switch (body->GetSetType()) {
            case AstQueryExpressionBody::UNION_ALL: {
                AstQueryExpressionBody *left = body->GetLeft(), *right = body->GetRight();
                un->SetLeftInput(translate_query_body(body->GetLeft(), e));
                un->SetRightInput(translate_query_body(body->GetRight(), e));
            } break;
            default: { assert(false); }
        }
        return un;
    }

    RelationAlgebraOperator *translate_query_primary(AstQueryPrimary *primary, TranslateException *e) {
        assert(primary->GetFrom().size() > 0);
        RelationAlgebraOperator *r = translate_table_ref(primary->GetFrom()[0], e);
        for (size_t i = 1; i < primary->GetFrom().size(); ++i) {
            RelationAlgebraOperator *left = r;
            RelationAlgebraOperator *right = translate_table_ref(primary->GetFrom()[i], e);
            auto cross = new RelationAlgebraCrossJoin();
            cross->SetLeftInput(left); cross->SetRightInput(right);
            r = cross;
        }
    }
}