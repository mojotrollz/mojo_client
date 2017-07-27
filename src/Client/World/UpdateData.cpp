#include "common.h"
#include "ZCompressor.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Object.h"
#include "Unit.h"
#include "Bag.h"
#include "GameObject.h"
#include "Corpse.h"
#include "DynamicObject.h"
#include "ObjMgr.h"
#include "UpdateMask.h"
#include "MovementInfo.h"


void WorldSession::_HandleCompressedUpdateObjectOpcode(WorldPacket& recvPacket)
{
    uint32 realsize;
    recvPacket >> realsize;
    ZCompressor z;
    z.append(recvPacket.contents() + sizeof(uint32),recvPacket.size() - sizeof(uint32));
    z.Compressed(true);
    z.RealSize(realsize);
    z.Inflate();
    if(z.Compressed())
    {
        logerror("_HandleCompressedUpdateObjectOpcode(): Inflate() failed! size=%u realsize=%u",z.size(),realsize);
        return;
    }
    WorldPacket wp;
    wp.SetOpcode(recvPacket.GetOpcode());
    wp.append(z.contents(),z.size());

    _HandleUpdateObjectOpcode(wp);
}

void WorldSession::_HandleUpdateObjectOpcode(WorldPacket& recvPacket)
{
    uint8 utype;
    uint8 hasTransport;
    uint32 usize, ublocks, readblocks=0;
    uint64 uguid;
    recvPacket >> ublocks; // >> hasTransport;
    if(GetInstance()->GetConf()->client <= CLIENT_TBC)
      recvPacket >> hasTransport;

    logdev("UpdateObject: blocks = %u", ublocks);
    while((recvPacket.rpos() < recvPacket.size())&& (readblocks < ublocks))
    {
        recvPacket >> utype;
        switch(utype)
        {
            case UPDATETYPE_VALUES:
            {
                uguid = recvPacket.readPackGUID();
                _ValuesUpdate(uguid,recvPacket);
            }
            break;

            case UPDATETYPE_MOVEMENT:
            {
                recvPacket >> uguid; // the guid is NOT packed here!
                uint8 tyid;
                Object *obj = objmgr.GetObj(uguid, true); // here we update also depleted objects, its just safer
                if(obj)
                    tyid = obj->GetTypeId();
                else // sometimes objects get deleted BEFORE a last update packet arrives, this must be handled also
                {
                    tyid = GetTypeIdByGuid(uguid);
                    logerror("Got UpdateObject_Movement for unknown object %016I64X. Using typeid %u",uguid,(uint32)tyid);
                }

                if(obj)
                    this->_MovementUpdate(tyid,uguid,recvPacket);
            }
            break;

            case UPDATETYPE_CREATE_OBJECT2: // will be sent when our very own character is created
            case UPDATETYPE_CREATE_OBJECT: // will be sent on any other object creation
            {
                uguid = recvPacket.readPackGUID();
                uint8 objtypeid;
                recvPacket >> objtypeid;
                logdebug("Create Object type %u with guid %016I64X",objtypeid,uguid);
                // dont create objects if already present in memory.
                // recreate every object except ourself!
                if(objmgr.GetObj(uguid))
                {
                    if(uguid != GetGuid())
                    {
                        logdev("- already exists, deleting old, creating new object");
                        objmgr.Remove(uguid, false);
                        // do not call script here, since the object does not really get deleted
                    }
                    else
                    {
                        logdev("- already exists, but not deleted (has our current GUID)");
                    }
                }

                // only if the obj didnt exist or was just deleted above, create it....
                if(!objmgr.GetObj(uguid))
                {
                    switch(objtypeid)
                    {
                    case TYPEID_OBJECT: // no data to read
                        {
                            logerror("Recieved wrong UPDATETYPE_CREATE_OBJECT to create Object base type!");
                            logerror("%s",toHexDump((uint8*)recvPacket.contents(),recvPacket.size(),true).c_str());
                        }
                    case TYPEID_ITEM:
                        {
                            Item *item = new Item();
                            item->Create(uguid);
                            objmgr.Add(item);
                            logdebug("Created Item with guid %016I64X",uguid);
                            break;
                        }
                    case TYPEID_CONTAINER:
                        {
                            Bag *bag = new Bag();
                            bag->Create(uguid);
                            objmgr.Add(bag);
                            logdebug("Created Bag with guid %016I64X",uguid);
                            break;
                        }
                    case TYPEID_UNIT:
                        {
                            Unit *unit = new Unit();
                            unit->Create(uguid);
                            objmgr.Add(unit);
                            logdebug("Created Unit with guid %016I64X",uguid);
                            break;
                        }
                    case TYPEID_PLAYER:
                        {
                            if(GetGuid() == uguid) // objmgr.Add() would cause quite some trouble if we added ourself again
                                break;
                            Player *player = new Player();
                            player->Create(uguid);
                            objmgr.Add(player);
                            logdebug("Created Player with guid %016I64X",uguid);
                            break;
                        }
                    case TYPEID_GAMEOBJECT:
                        {
                            GameObject *go = new GameObject();
                            go->Create(uguid);
                            objmgr.Add(go);
                            logdebug("Created GO with guid %016I64X",uguid);
                            break;
                        }
                    case TYPEID_CORPSE:
                        {
                            Corpse *corpse = new Corpse();
                            corpse->Create(uguid);
                            objmgr.Add(corpse);
                            logdebug("Created Corpse with guid %016I64X",uguid);
                            break;
                        }
                    case TYPEID_DYNAMICOBJECT:
                        {
                            DynamicObject *dobj = new DynamicObject();
                            dobj->Create(uguid);
                            objmgr.Add(dobj);
                            logdebug("Created DynObj with guid %016I64X",uguid);
                            break;
                        }
                    }
                }
                else
                {
                    logdebug("Obj %016I64X not created, already exists",uguid);
                }
                // ...regardless if it was freshly created or already present, update its values and stuff now...
                this->_MovementUpdate(objtypeid, uguid, recvPacket);
                this->_ValuesUpdate(uguid, recvPacket);

                // ...and ask the server for eventually missing data.
                _QueryObjectInfo(uguid);


                // call script "_OnObjectCreate"
                if(GetInstance()->GetScripts()->ScriptExists("_onobjectcreate"))
                {
                    CmdSet Set;
                    Set.defaultarg = toString(uguid);
                    Set.arg[0] = toString(objtypeid);
                    GetInstance()->GetScripts()->RunScript("_onobjectcreate", &Set);

                }

                // if our own character got finally created, we have successfully entered the world,
                // and should have gotten all info about our char already.
            }
            break;

            case UPDATETYPE_OUT_OF_RANGE_OBJECTS:
            {
                recvPacket >> usize;
                for(uint32 i=0;i<usize;i++)
                {
                    uguid = recvPacket.readPackGUID(); // not 100% sure if this is correct
                    logdebug("GUID %016I64X out of range",uguid);

//                     // call script just before object removal
//                     if(GetInstance()->GetScripts()->ScriptExists("_onobjectdelete"))
//                     {
//                         Object *del_obj = objmgr.GetObj(uguid);
//                         CmdSet Set;
//                         Set.defaultarg = toString(uguid);
//                         Set.arg[0] = del_obj ? toString(del_obj->GetTypeId()) : "";
//                         Set.arg[1] = "true"; // out of range = true
//                         GetInstance()->GetScripts()->RunScript("_onobjectdelete", &Set);
//                     }
//
//                     objmgr.Remove(uguid, false);
                }
            }
            break;

            default:
            {
                logerror("UPDATE_OBJECT: Got unk updatetype 0x%X",utype);
                logerror("UPDATE_OBJECT: Read %u / %u bytes, skipped rest",recvPacket.rpos(),recvPacket.size());
                logerror("%s",toHexDump((uint8*)recvPacket.contents(),recvPacket.size(),true).c_str());
                char buf[100];
                sprintf(buf,"Got unk updatetype=0x%X, read %u / %u bytes",utype,recvPacket.rpos(),recvPacket.size());

                if(GetInstance()->GetConf()->dumpPackets)
                {
                    char buf[100];
                    sprintf(buf,"Got unk updatetype=0x%X, read %u / %u bytes",utype,recvPacket.rpos(),recvPacket.size());
                    DumpPacket(recvPacket, recvPacket.rpos(),buf);
                }

                return;
            }
        } // switch
	readblocks++;
    } // while

} // func

void WorldSession::_MovementUpdate(uint8 objtypeid, uint64 uguid, WorldPacket& recvPacket)
{
    MovementInfo mi; // TODO: use a reference to a MovementInfo in Unit/Player class once implemented
    uint16 flags;
    uint8 flags_6005;
    // uint64 fullguid; // see below
    float speedWalk =0, speedRun =0, speedSwimBack =0, speedSwim =0, speedWalkBack =0, speedTurn =0, speedFly =0, speedFlyBack =0, speedPitchRate =0;
    uint32 unk32;

    uint16 client = GetInstance()->GetConf()->client;

    Object *obj = (Object*)objmgr.GetObj(uguid, true); // also depleted objects
    Unit *u = NULL;
    if(obj)
    {
        if(obj->IsUnit())
            u = (Unit*)obj; // only use for Unit:: functions!!
        else
            logdev("MovementUpdate: object %016I64X is not Unit (typeId=%u)",obj->GetGUID(),obj->GetTypeId());
    }
    else
    {
        logerror("MovementUpdate for unknown object %016I64X typeid=%u",uguid,objtypeid);
    }

    if(client > CLIENT_TBC)
      recvPacket >> flags;
    else
    {
      recvPacket >> flags_6005;
      flags = flags_6005;
    }
    mi.flags = 0; // not sure if its correct to set it to 0 (needs some starting flag?)
    if(flags & UPDATEFLAG_LIVING)
    {
        recvPacket >> mi;

        logdev("MovementUpdate: TypeID=%u GUID=%016I64X pObj=%X flags=%x mi.flags=%x",objtypeid,uguid,obj,flags,mi.flags);
        logdev("FLOATS: x=%f y=%f z=%f o=%f",mi.pos.x, mi.pos.y, mi.pos.z ,mi.pos.o);
        if(obj && obj->IsWorldObject())
            ((WorldObject*)obj)->SetPosition(mi.pos.x, mi.pos.y, mi.pos.z, mi.pos.o);

        if(mi.flags & MOVEMENTFLAG_ONTRANSPORT)
        {
            logdev("TRANSPORT @ mi.flags: guid=%016I64X x=%f y=%f z=%f o=%f", mi.t_guid, mi.t_pos.x, mi.t_pos.y, mi.t_pos.z, mi.t_pos.o);
        }

        if((mi.flags & (MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING)) || (mi.flags2 & MOVEMENTFLAG2_ALLOW_PITCHING))
        {
            logdev("MovementUpdate: MOVEMENTFLAG_SWIMMING or FLYING is set, angle = %f!", mi.s_angle);
        }

        logdev("MovementUpdate: FallTime = %u", mi.fallTime);

        if(mi.flags & MOVEMENTFLAG_FALLING)
        {
            logdev("MovementUpdate: MOVEMENTFLAG_FALLING is set, velocity=%f sinA=%f cosA=%f xyspeed=%f = %u", mi.j_velocity, mi.j_sinAngle, mi.j_cosAngle, mi.j_xyspeed);
        }

        if(mi.flags & MOVEMENTFLAG_SPLINE_ELEVATION)
        {
            logdev("MovementUpdate: MOVEMENTFLAG_SPLINE is set, got %u", mi.u_unk1);
        }

        recvPacket >> speedWalk >> speedRun >> speedSwimBack >> speedSwim >> speedWalkBack; // speedRun can also be mounted speed if player is mounted; WalkBack is called RunBack in Mangos
        if(client > CLIENT_CLASSIC_WOW)
          recvPacket >> speedFly >> speedFlyBack; // fly added in 2.0.x
        recvPacket >> speedTurn;
        if(client > CLIENT_TBC)
          recvPacket >> speedPitchRate;
        logdev("MovementUpdate: Got speeds, walk=%f run=%f turn=%f", speedWalk, speedRun, speedTurn);
        if(u)
        {
            u->SetPosition(mi.pos.x, mi.pos.y, mi.pos.z, mi.pos.o);
            u->SetSpeed(MOVE_WALK, speedWalk);
            u->SetSpeed(MOVE_RUN, speedRun);
            u->SetSpeed(MOVE_SWIMBACK, speedSwimBack);
            u->SetSpeed(MOVE_SWIM, speedSwim);
            u->SetSpeed(MOVE_WALKBACK, speedWalkBack);
            u->SetSpeed(MOVE_TURN, speedTurn);
            u->SetSpeed(MOVE_FLY, speedFly);
            u->SetSpeed(MOVE_FLYBACK, speedFlyBack);
            u->SetSpeed(MOVE_PITCH_RATE, speedPitchRate);
        }

        // TODO: correct this one as soon as its meaning is known OR if it appears often and needs to be fixed
        if(mi.flags & MOVEMENTFLAG_SPLINE_ENABLED)
        {
            logdev("MovementUpdate: MOVEMENTFLAG_SPLINE_ENABLED!");
            //checked for 3.3.5
            //We do not do anything with the spline stuff so far, it just needs to be read to be skipped correctly
            uint32 splineflags, timepassed, duration, id, effect_start_time, path_nodes;
            uint8 spline_mode;
            float facing_angle,facing_x,facing_y,facing_z, duration_mod, duration_next, vertical_acceleration;
            float x,y,z;
            recvPacket >> splineflags;
            if(splineflags & SF_Final_Angle)
              recvPacket >> facing_angle;
            if(splineflags & SF_Final_Point)
              recvPacket >> facing_x >> facing_y >> facing_z;
            recvPacket >> timepassed >> duration >> id >> duration_mod >> duration_next >> vertical_acceleration >> effect_start_time;
            recvPacket >> path_nodes;
            for(uint32 i = 0;i<path_nodes;i++)
            {
              recvPacket >> x >> y >> z;
            }
            recvPacket >> spline_mode;
            recvPacket >> x >> y >> z; // FinalDestination
        }
    }
    else // !UPDATEFLAG_LIVING
    {
        if(flags & UPDATEFLAG_POSITION)
        {
            uint64 pguid = recvPacket.readPackGUID();
            float x,y,z,o,sx,sy,sz,so;
            recvPacket >> x >> y >> z;
            recvPacket >> sx >> sy >> sz;
            recvPacket >> o >> so;

            if (obj && obj->IsWorldObject())
                ((WorldObject*)obj)->SetPosition(x, y, z, o);
        }
        else
        {
            if(flags & UPDATEFLAG_HAS_POSITION)
            {
                float x,y,z,o;
                if(flags & UPDATEFLAG_TRANSPORT)
                {
                    recvPacket >> x >> y >> z >> o;
                    // only zeroes here
                }
                else
                {
                    recvPacket >> x >> y >> z >> o;
                    if (obj && obj->IsWorldObject())
                        ((WorldObject*)obj)->SetPosition(x, y, z, o);
                }
            }
        }
    }

    if(client > CLIENT_CLASSIC_WOW && flags & UPDATEFLAG_LOWGUID)
    {
        recvPacket >> unk32;
        logdev("MovementUpdate: UPDATEFLAG_LOWGUID is set, got %X", unk32);
    }

    if(client > CLIENT_CLASSIC_WOW && flags & UPDATEFLAG_HIGHGUID)
    {
        recvPacket >> unk32;             // 2.0.6 - high guid was there, unk for 2.0.12
        // not sure if this is correct, MaNGOS sends 0 always.
        //obj->SetUInt32Value(OBJECT_FIELD_GUID+1,higuid); // note that this sets only the high part of the guid
        logdev("MovementUpdate: UPDATEFLAG_HIGHGUID is set, got %X", unk32);
    }
    if(client == CLIENT_CLASSIC_WOW && flags & UPDATEFLAG_ALL_6005)
    {
        recvPacket >> unk32;
        // MaNGOS sends 1 always.
        logdev("MovementUpdate: UPDATEFLAG_ALL is set, got %X", unk32);
    }


    if(flags & UPDATEFLAG_HAS_TARGET)
    {
        uint64 unkguid = recvPacket.readPackGUID(); // MaNGOS sends uint8(0) always, but its probably be a packed guid
        logdev("MovementUpdate: UPDATEFLAG_FULLGUID is set, got %016I64X", unkguid);
    }

    if(flags & UPDATEFLAG_TRANSPORT)
    {
        recvPacket >> unk32; // mangos says: ms time
        logdev("MovementUpdate: UPDATEFLAG_TRANSPORT is set, got %u", unk32);
    }

    if(flags & UPDATEFLAG_VEHICLE)                          // unused for now
    {
        uint32 vehicleId;
        float facingAdj;

        recvPacket >> vehicleId >> facingAdj;
    }

    if(flags & UPDATEFLAG_ROTATION)
    {
        uint64 rotation;
        recvPacket >> rotation;
        // gameobject rotation
    }
}

void WorldSession::_ValuesUpdate(uint64 uguid, WorldPacket& recvPacket)
{
    Object *obj = objmgr.GetObj(uguid);
    uint8 blockcount,tyid;
    uint32 value, masksize, valuesCount;

    if(obj)
    {
        valuesCount = obj->GetValuesCount();
        tyid = obj->GetTypeId();
        logdebug("Type %u Object, %u Values",tyid, valuesCount);
    }
    else
    {
        logcustom(1,LRED, "Got UpdateObject_Values for unknown object %016I64X",uguid);
        tyid = GetTypeIdByGuid(uguid); // can cause problems with TYPEID_CONTAINER!!
        valuesCount = GetValuesCountByTypeId(tyid);
    }


    recvPacket >> blockcount;
    masksize = blockcount << 2; // each sizeof(uint32) == <4> * sizeof(uint8) // 1<<2 == <4>
    UpdateMask umask;
    uint32 *updateMask = new uint32[blockcount];
    umask.SetCount(masksize);
    recvPacket.read((uint8*)updateMask, masksize);
    umask.SetMask(updateMask);
    //delete [] updateMask; // will be deleted at ~UpdateMask() !!!!
    logdev("ValuesUpdate TypeId=%u GUID=%016I64X pObj=%X Blocks=%u Masksize=%u",tyid,uguid,obj,blockcount,masksize);
    // just in case the object does not exist, and we have really a container instead of an item, and a value in
    // the container fields is set, THEN we have a problem. this should never be the case; it can be fixed in a
    // more correct way if there is the need.
    // (-> valuesCount smaller then it should be might skip a few bytes and corrupt the packet)
    for (uint32 i = 0; i < valuesCount; i++)
    {
        if (umask.GetBit(i))
        {
            if(obj)
            {
                recvPacket >> value;
                obj->SetUInt32Value(i, value);  //It does not matter what type of value we are setting, just copy the bytes
                DEBUG(logdev("%u %u",i,value));
            }
            else
            {
                recvPacket >> value; // drop the value, since object doesnt exist (always 4 bytes)
            }
        }
    }
}

void WorldSession::_QueryObjectInfo(uint64 guid)
{
    Object *obj = objmgr.GetObj(guid);
    if(obj)
    {
        switch(obj->GetTypeId())
        {
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
            {
                ItemProto *proto = objmgr.GetItemProto(obj->GetEntry());
                if(proto)
                {
                    obj->SetName(proto->Name);
                }
                else
                {
                    logdebug("Found unknown item: GUID=%016I64X entry=%u",obj->GetGUID(),obj->GetEntry());
                    SendQueryItem(obj->GetEntry(),guid); // not sure if sending GUID is correct
                }
                break;
            }
        case TYPEID_PLAYER:
            {
                std::string name = GetOrRequestPlayerName(obj->GetGUID());
                if(!name.empty())
                {
                    obj->SetName(name);
                }
                // else: name will be set when server answers (_HandleNameQueryResponseOpcode)
                break;
            }
        case TYPEID_UNIT: // checked after player; this one here should cover creatures only
            {
                CreatureTemplate *ct = objmgr.GetCreatureTemplate(obj->GetEntry());
                if(ct)
                    obj->SetName(ct->name);
                else
                    SendQueryCreature(obj->GetEntry(),guid);
                break;
            }
        case TYPEID_GAMEOBJECT:
            {
                GameobjectTemplate *go = objmgr.GetGOTemplate(obj->GetEntry());
                if(go)
                    obj->SetName(go->name);
                else
                    SendQueryGameobject(obj->GetEntry(),guid);
                break;
            }
        //case...
        }
    }
}

void MovementInfo::Read(ByteBuffer &data)
{
    data >> flags;
    if(_c == CLIENT_WOTLK)
      data >> flags2;
    if(_c == CLIENT_TBC)
    {
      uint8 tempFlags2;
      data >> tempFlags2;
      flags2 = tempFlags2;
    }
    data >> time;
    data >> pos.x;
    data >> pos.y;
    data >> pos.z;
    data >> pos.o;

    if(flags & (MOVEMENTFLAG_ONTRANSPORT))
    {
        if(_c < CLIENT_WOTLK)
          data >> t_guid;
        else
          t_guid =data.readPackGUID();
        data >> t_pos.x;
        data >> t_pos.y;
        data >> t_pos.z;
        data >> t_pos.o;
        if(_c > CLIENT_CLASSIC_WOW)
          data >> t_time;
        if(_c > CLIENT_TBC)
          data >> t_seat;

        if(_c > CLIENT_TBC && flags2 & MOVEMENTFLAG2_INTERP_MOVEMENT)
            data >> t_time2;
    }

    if((flags & (MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING)) || (flags2 & MOVEMENTFLAG2_ALLOW_PITCHING))
    {
        data >> s_angle;
    }

    data >> fallTime;

    if(flags & (MOVEMENTFLAG_FALLING))
    {
        data >> j_velocity;
        data >> j_sinAngle;
        data >> j_cosAngle;
        data >> j_xyspeed;
    }

    if(flags & (MOVEMENTFLAG_SPLINE_ELEVATION))
    {
        data >> u_unk1;
    }
}

void MovementInfo::Write(ByteBuffer &data) const
{
    data << flags;
    if(_c == CLIENT_WOTLK)
      data << flags2;
    if(_c == CLIENT_TBC)
    {
      data << (uint8)flags2;
    }
    data << time;
    data << pos.x;
    data << pos.y;
    data << pos.z;
    data << pos.o;

    if(flags & (MOVEMENTFLAG_ONTRANSPORT))
    {
        if(_c < CLIENT_WOTLK)
          data << t_guid;
        else
          data.appendPackGUID(t_guid);
        data << t_pos.x;
        data << t_pos.y;
        data << t_pos.z;
        data << t_pos.o;
        if(_c > CLIENT_CLASSIC_WOW)
          data << t_time;
        if(_c > CLIENT_TBC)
          data << t_seat;

        if(_c > CLIENT_TBC && flags2 & MOVEMENTFLAG2_INTERP_MOVEMENT)
            data << t_time2;
    }

    if((flags & (MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING)) || (flags2 & MOVEMENTFLAG2_ALLOW_PITCHING))
    {
        data << s_angle;
    }

    data << fallTime;

    if(flags & (MOVEMENTFLAG_FALLING))
    {
        data << j_velocity;
        data << j_sinAngle;
        data << j_cosAngle;
        data << j_xyspeed;
    }

    if(flags & (MOVEMENTFLAG_SPLINE_ELEVATION))
    {
        data << u_unk1;
    }
}



