#include <fstream>
#include "common.h"
#include "SCPDatabase.h"

uint32 SCPDatabase::LoadFromFile(char *fn)
{
    std::fstream fh;
    std::string line,value,entry,storage;
    uint32 id=0,sections=0;
    char c;

    fh.open(fn,std::ios_base::in);
    if( !fh.is_open() )
        return 0;
    while(!fh.eof())
    {
        c=fh.get();
        if(c=='\n' || fh.eof())
        {
            if(line.empty())
                continue;
            while(line[0]==' ' || line[0]=='\t')
                line.erase(0,1);
            if(line.empty() || (line.length() > 1 && (line[0]=='/' && line[1]=='/')) )
            {
                line.clear();
                continue;
            }
            if(line[0]=='[')
            {
                id=(uint32)toInt(line.c_str()+1); // start reading after '['
                sections++;
            }
            else
            {
                uint32 pos=line.find("=");
                if(pos!=std::string::npos)
                {
                    entry=stringToLower(line.substr(0,pos));
                    value=line.substr(pos+1,line.length()-1);
                    _map[id].Set(entry,value);
                }
                // else invalid line, must have '='
            }
            line.clear();
        }
        else
            line+=c; // fill up line until a newline is reached (see above)
    }
    fh.close();
    return sections;
}

bool SCPDatabase::HasField(uint32 id)
{
    for(SCPFieldMap::iterator i = _map.begin(); i != _map.end(); i++)
        if(i->first == id)
            return true;
    return false;
}

bool SCPField::HasEntry(std::string e)
{
    for(SCPEntryMap::iterator i = _map.begin(); i != _map.end(); i++)
        if(i->first == e)
            return true;
    return false;
}

std::string SCPField::GetString(std::string entry)
{
    return HasEntry(entry) ? _map[entry] : "";
}

// note that this can take a while depending on the size of the database!
uint32 SCPDatabase::GetFieldByValue(std::string entry, std::string value)
{
    for(SCPFieldMap::iterator fm = _map.begin(); fm != _map.end(); fm++)
        if(fm->second.HasEntry(entry) && fm->second.GetString(entry)==value)
            return fm->first;
    return 0;
}

bool SCPDatabaseMgr::HasDB(std::string n)
{
    for(SCPDatabaseMap::iterator i = _map.begin(); i != _map.end(); i++)
        if(i->first == n)
            return true;
    return false;
}

SCPDatabase& SCPDatabaseMgr::GetDB(std::string n)
{
    return _map[n];
}

// -- helper functions -- //

std::string SCPDatabaseMgr::GetZoneName(uint32 id)
{
    return GetDB("zone").GetField(id).GetString("name");
}

std::string SCPDatabaseMgr::GetRaceName(uint32 id)
{
    std::string r = GetDB("race").GetField(id).GetString("name");
    //if(r.empty())
    //    r = raceName[id];
    return r;
}

std::string SCPDatabaseMgr::GetMapName(uint32 id)
{
    return GetDB("map").GetField(id).GetString("name");
}

std::string SCPDatabaseMgr::GetClassName_(uint32 id)
{
    std::string r = GetDB("class").GetField(id).GetString("name");
    //if(r.empty())
    //    r = className[id];
    return r;
}

std::string SCPDatabaseMgr::GetGenderName(uint32 id)
{
    return GetDB("gender").GetField(id).GetString("name");
}

std::string SCPDatabaseMgr::GetLangName(uint32 id)
{
    std::string r = GetDB("language").GetField(id).GetString("name");
    //if(r.empty())
    //    r = LookupName(id,langNames);
    return r;
}