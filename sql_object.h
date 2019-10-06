#ifndef GSP_SQL_OBJECT_H
#define GSP_SQL_OBJECT_H

#include <string>
#include <assert.h>

namespace GSP {
    enum SQLObjectType {
        AST_ANY,
        AST_SELECT_STMT,
        AST_QUERY_SET,
        AST_QUERY_PRIMARY,
        AST_TABLE_JOIN,
        AST_RELATION,
        AST_SUBQUERY_TABLE_REF,
        AST_EXPR,
    };
    class IObject {
    public:
        IObject () : _parent(nullptr), _obj_type(AST_ANY) {}
        IObject (SQLObjectType obj_type) : _parent(nullptr), _obj_type(obj_type) {}
        virtual ~IObject() {}
        virtual SQLObjectType GetObjectType() final { return _obj_type; };
        virtual IObject *GetParent() final { return _parent; }
        virtual void SetParent(IObject *parent) final { assert(_parent == nullptr); _parent = parent; };
    protected:
        IObject             *_parent;
        SQLObjectType        _obj_type;
    };

    class AstId {
    public:
        AstId(const std::string& id) : _id(id) {}
        void SetId(const std::string& id) { _id = id; }
        const std::string& GetId() { return _id; }
    private:
        std::string     _id;
    };

    typedef std::vector<AstId*>                 AstIds;
}

#endif