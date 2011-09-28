#include "Corpse.h"

Corpse::Corpse()
{
    _type=TYPE_CORPSE;
    _typeid=TYPEID_CORPSE;
    _valuescount=Object::maxvalues[_typeid];
}

void Corpse::Create(uint64 guid)
{
    Object::Create(guid);
}
