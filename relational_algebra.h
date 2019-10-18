#ifndef RELATIONAL_ALGEBRA_H
#define RELATIONAL_ALGEBRA_H

#include <vector>
#include <string>

namespace GSP {
    class AstExpr;

    class RelationAlgebraOperator {
    public:
        enum RELATION_ALGEBRA_OPERATOR_TYPE { RAOT_UNARY, RAOT_BINARY };
        explicit RelationAlgebraOperator(RELATION_ALGEBRA_OPERATOR_TYPE tp) : _type(tp) {}
        virtual ~RelationAlgebraOperator() {}
        RELATION_ALGEBRA_OPERATOR_TYPE GetType() const { return _type; }
    protected:
        RELATION_ALGEBRA_OPERATOR_TYPE _type;
    };

    class RelationAlgebraUnaryOp : public RelationAlgebraOperator {
    public:
        enum RELATION_ALGEBRA_UNARY_TYPE { RAUOT_PROJECTION, RAUOT_SELECTION, RAUOT_AGGREGATION, /* todo ... */ };
        explicit RelationAlgebraUnaryOp(RELATION_ALGEBRA_UNARY_TYPE tp) : RelationAlgebraOperator(RAOT_UNARY), _unary_type(tp) {}
        virtual ~RelationAlgebraUnaryOp() { /* todo */ }
        RELATION_ALGEBRA_UNARY_TYPE GetUnaryType() const { return _unary_type; }
        void SetInput(RelationAlgebraOperator *input) { _input = input; }
        RelationAlgebraOperator *GetInput() { return _input; }
    protected:
        RELATION_ALGEBRA_UNARY_TYPE _unary_type;
        RelationAlgebraOperator    *_input;
    };

    class RelationAlgebraBinaryOp : public RelationAlgebraOperator {
    public:
        enum RELATION_ALGEBRA_BINARY_TYPE { RABOT_UNION, RABOT_CROSS_JOIN, /* todo ... */ };
        explicit RelationAlgebraBinaryOp(RELATION_ALGEBRA_BINARY_TYPE tp) : RelationAlgebraOperator(RAOT_BINARY), _binary_type(tp) {}
        virtual ~RelationAlgebraBinaryOp() { /* todo */ }
        RELATION_ALGEBRA_BINARY_TYPE GetBinaryType() const { return _binary_type; }
        void SetLeftInput(RelationAlgebraOperator *left_input) { _left_input = left_input; }
        RelationAlgebraOperator *GetLeftInput() { return _left_input; }
        void SetRightInput(RelationAlgebraOperator *right_input) { _right_input = right_input; }
        RelationAlgebraOperator *GetRightInput() { return _right_input; }
    protected:
        RELATION_ALGEBRA_BINARY_TYPE _binary_type;
        RelationAlgebraOperator     *_left_input;
        RelationAlgebraOperator     *_right_input;
    };

    class RelationAlgebraProjection : RelationAlgebraUnaryOp {
    public:
        RelationAlgebraProjection(const std::vector<AstExpr*>& column_expressions, const std::vector<std::string>& aliases) :
                RelationAlgebraUnaryOp(RAUOT_PROJECTION), _column_expressions(column_expressions), _aliases(aliases) {}
        ~RelationAlgebraProjection() { /* todo */ }
    private:
        std::vector<AstExpr*>       _column_expressions;
        std::vector<std::string>    _aliases;
    };

    class RelationAlgebraSelection : public RelationAlgebraUnaryOp {
    public:
        RelationAlgebraSelection(const std::vector<AstExpr*>& predicates) : RelationAlgebraUnaryOp(RAUOT_SELECTION), _predicates(predicates) {}
        virtual ~RelationAlgebraSelection() { /* todo */ }
    private:
        std::vector<AstExpr*>       _predicates;    // all predicate linked with and
    };

    class RelationAlgebraAggregation : public RelationAlgebraUnaryOp {
    public:
        RelationAlgebraAggregation(const std::vector<AstExpr*>& group_by_column_expressions, const std::vector<AstExpr*>& aggregate_column_expressions) :
                RelationAlgebraUnaryOp(RAUOT_AGGREGATION), _group_by_column_expressions(group_by_column_expressions), _aggregate_column_expressions(aggregate_column_expressions) {}
        ~RelationAlgebraAggregation() { /* todo */ }
    private:
        std::vector<AstExpr*>       _group_by_column_expressions;
        std::vector<AstExpr*>       _aggregate_column_expressions;
    };

    class RelationAlgebraUnion : public RelationAlgebraBinaryOp {
    public:
        RelationAlgebraUnion() : RelationAlgebraBinaryOp(RABOT_UNION) {}
        virtual ~RelationAlgebraUnion() { /* todo */ }
    private:

    };

    class RelationAlgebraCrossJoin : public RelationAlgebraBinaryOp {
    public:
        RelationAlgebraCrossJoin() : RelationAlgebraBinaryOp(RABOT_CROSS_JOIN) {}
        virtual ~RelationAlgebraCrossJoin() { /* todo */ }
    private:

    };
}



#endif