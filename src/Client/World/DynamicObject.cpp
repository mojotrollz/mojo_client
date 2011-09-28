#include "DynamicObject.h"

DynamicObject::DynamicObject() : WorldObject()
{
    _uint32values=NULL;
    _type=TYPE_DYNAMICOBJECT;
    _typeid=TYPEID_DYNAMICOBJECT;
    _valuescount=Object::maxvalues[_typeid];
}

void DynamicObject::Create(uint64 guid)
{
    Object::Create(guid);
}
