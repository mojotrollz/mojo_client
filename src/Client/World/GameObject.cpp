#include "GameObject.h"

GameObject::GameObject() : WorldObject()
{
    _uint32values=NULL;
    _type|=TYPE_GAMEOBJECT;
    _typeid=TYPEID_GAMEOBJECT;
    _valuescount=Object::maxvalues[_typeid];
}

void GameObject::Create(uint64 guid)
{
    Object::Create(guid);
}
