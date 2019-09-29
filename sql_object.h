#ifndef GSP_SQL_OBJECT_H
#define GSP_SQL_OBJECT_H

#include <string>
#include <assert.h>

namespace GSP {
    enum SQLObjectType {
        AST_SELECT_STMT,
        AST_WITH_CLAUSE,
        AST_COMMON_TABLE_EXPR,
        AST_QUERY_SET,
        AST_QUERY_PRIMARY,
        AST_PROJECTION,
        AST_ORDER_BY_ITEM,
        AST_TABLE_JOIN,
        AST_TABLE_CROSS_JOIN,
        AST_TABLE_NATURAL_JOIN,
        AST_RELATION,
        AST_SUBQUERY_TABLE_REF,
        AST_EXPR,

    };
    class IObject {
    public:
        IObject (SQLObjectType obj_type) : _parent(nullptr), _obj_type(obj_type) {}
        virtual ~IObject() {}
        virtual SQLObjectType GetObjectType() final { return _obj_type; };
        virtual IObject *GetParent() final { return _parent; }
        virtual void SetParent(IObject *parent) final { assert(_parent == nullptr); _parent = parent; };
    protected:
        IObject             *_parent;
        SQLObjectType        _obj_type;
    };

    class AstId : public IObject {
    public:
        void SetId(const std::string& id) { _id = id; }
    private:
        std::string     _id;
    };

    typedef std::vector<AstId*>                 AstIds;
}

#endif