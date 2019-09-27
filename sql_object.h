#ifndef GSP_SQL_OBJECT_H
#define GSP_SQL_OBJECT_H

#include <string>

namespace GSP {
    enum SQLObjectType {
        SQL_SELECT_STMT,
    };
    class IObject {
    public:
        virtual ~IObject() {}
        //IObject *GetParent() = 0;
        //void SetParent(IObject *parent) = 0;
        //SQLObjectType GetType() = 0;
    };


    class AstId : public IObject {
    public:
        void SetId(const std::string& id) { _id = id; }
    private:
        std::string     _id;
    };
}

#endif