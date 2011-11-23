// #define _DEBUG 1
#include <iostream>
#include "MemoryDataHolder.h"
#include "MemoryInterface.h"
#include "CM2MeshFileLoader.h"
#include "common.h"

namespace irr
{
namespace scene
{

CM2MeshFileLoader::CM2MeshFileLoader(IrrlichtDevice* device):Device(device)
{
    Mesh = NULL;

}

CM2MeshFileLoader::~CM2MeshFileLoader()
{

}

core::vector3df fixCoordSystem(core::vector3df v)
{
        return core::vector3df(v.X, v.Z, v.Y);
}

core::quaternion fixQuaternion(core::quaternion q)
{
        return core::quaternion(q.X, q.Z, q.Y, q.W);
}
bool CM2MeshFileLoader::isALoadableFileExtension(const c8* filename)const
{
	return strstr(filename, ".m2")!=0;
}


//! creates/loads an animated mesh from the file.
//! \return Pointer to the created mesh. Returns 0 if loading failed.
//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
//! See IUnknown::drop() for more information.
IAnimatedMesh* CM2MeshFileLoader::createMesh(io::IReadFile* file)
{
    if(!file)
        return 0;
    MeshFile = file;
    AnimatedMesh = new scene::CM2Mesh();

	if ( load() )
	{
		AnimatedMesh->finalize();
	}
	else
	{
		AnimatedMesh->drop();
		AnimatedMesh = 0;
	}

	return AnimatedMesh;
}
void CM2MeshFileLoader::ReadVertices()
{
    //Vertices.  Global data
    if(!M2MVertices.empty())
        M2MVertices.clear();

    ModelVertex tempM2MVert;
    MeshFile->seek(header.Vertices.ofs);

    for(u32 i =0;i<header.Vertices.num;i++)
    {
        MeshFile->read(&tempM2MVert,sizeof(ModelVertex));
        tempM2MVert.pos = fixCoordSystem(tempM2MVert.pos);
        tempM2MVert.normal = fixCoordSystem(tempM2MVert.normal);
        M2MVertices.push_back(tempM2MVert);
    }
    DEBUG(logdebug("Read %u/%u Vertices",M2MVertices.size(),header.Vertices.num));
}

void CM2MeshFileLoader::ReadViewData(io::IReadFile* file)
{
    //Vertex indices of a specific view.Local to View 0
    if(M2MIndices.size()>0)
        M2MIndices.clear();

    u16 tempM2Index;
    file->seek(currentView.Index.ofs);
    for(u32 i =0;i<currentView.Index.num;i++)
    {
        file->read(&tempM2Index,sizeof(u16));
        M2MIndices.push_back(tempM2Index);
    }
    DEBUG(logdebug("Read %u/%u Indices",M2MIndices.size(),currentView.Index.num));

    //Triangles. Data Points point to the Vertex Indices, not the vertices themself. 3 Points = 1 Triangle, Local to View 0
    if(M2MTriangles.size()>0)
        M2MTriangles.clear();

    u16 tempM2Triangle;
    file->seek(currentView.Triangle.ofs);
    for(u32 i =0;i<currentView.Triangle.num;i++)
    {
        file->read(&tempM2Triangle,sizeof(u16));
        M2MTriangles.push_back(tempM2Triangle);
    }
    DEBUG(logdebug("Read %u/%u Triangles",M2MTriangles.size(),currentView.Triangle.num));
    //Submeshes, Local to View 0
    if(M2MSubmeshes.size()>0)
        M2MSubmeshes.clear();

    ModelViewSubmesh tempM2Submesh;
    file->seek(currentView.Submesh.ofs);
    for(u32 i =0;i<currentView.Submesh.num;i++)
    {
        file->read(&tempM2Submesh,sizeof(ModelViewSubmesh)-(header.version==0x100?16:0));
        M2MSubmeshes.push_back(tempM2Submesh);
        DEBUG(logdebug("Submesh %u MeshPartID %u",i,tempM2Submesh.meshpartId));
    //    std::cout<< "Submesh " <<i<<" ID "<<tempM2Submesh.meshpartId<<" starts at V/T "<<tempM2Submesh.ofsVertex<<"/"<<tempM2Submesh.ofsTris<<" and has "<<tempM2Submesh.nVertex<<"/"<<tempM2Submesh.nTris<<" V/T\n";
    }
    DEBUG(logdebug("Read %u/%u Submeshes",M2MSubmeshes.size(),currentView.Submesh.num));

    //Texture units. Local to view 0
    TextureUnit tempM2TexUnit;
    if(!M2MTextureUnit.empty())
    {
        M2MTextureUnit.clear();
    }
    file->seek(currentView.Tex.ofs);
    for(u32 i=0;i<currentView.Tex.num;i++)
    {
        file->read(&tempM2TexUnit,sizeof(TextureUnit));
        M2MTextureUnit.push_back(tempM2TexUnit);
    DEBUG(logdebug(" TexUnit %u: Submesh: %u %u Render Flag: %u TextureUnitNumber: %u %u TTU: %u",i,tempM2TexUnit.submeshIndex1,tempM2TexUnit.submeshIndex2, tempM2TexUnit.renderFlagsIndex, tempM2TexUnit.TextureUnitNumber, tempM2TexUnit.TextureUnitNumber2 ,tempM2TexUnit.textureIndex));
    }
    DEBUG(logdebug("Read %u Texture Unit entries for View 0",M2MTextureUnit.size()));

}

void CM2MeshFileLoader::ReadBones()
{

//Bones. This is global data
Bone tempBone;
if(!M2MBones.empty())
{
    M2MBones.clear();
}
MeshFile->seek(header.Bones.ofs);
for(u32 i=0;i<header.Bones.num;i++)
{
    MeshFile->read(&tempBone,12+(header.version==0x100?0:4));

    MeshFile->read(&tempBone.translation.header,sizeof(AnimBlockHead));
    MeshFile->read(&tempBone.rotation.header,sizeof(AnimBlockHead));
    MeshFile->read(&tempBone.scaling.header,sizeof(AnimBlockHead));

    MeshFile->read(&tempBone.PivotPoint,sizeof(core::vector3df));
    tempBone.PivotPoint=fixCoordSystem(tempBone.PivotPoint);
    M2MBones.push_back(tempBone);
    DEBUG(logdebug("Bone %u Parent %u PP %f %f %f Flags %X",i,tempBone.parentBone,tempBone.PivotPoint.X,tempBone.PivotPoint.Y,tempBone.PivotPoint.Z, tempBone.flags));
}
//Fill in values referenced in Bones. local to each bone
//Interpolation Ranges are not used
u32 tempBoneTS;
numofs tempBoneNumOfs;
float tempBoneValue;
for(u32 i=0; i<M2MBones.size(); i++)
{

    if(M2MBones[i].translation.header.TimeStamp.num>0)
    {
        MeshFile->seek(M2MBones[i].translation.header.TimeStamp.ofs);
        for(u32 j=0; j<M2MBones[i].translation.header.TimeStamp.num;j++)
        {
            MeshFile->read(&tempBoneTS, sizeof(u32));
            M2MBones[i].translation.timestamps.push_back(tempBoneTS);
        }
    }
    if(M2MBones[i].rotation.header.TimeStamp.num>0)
    {
        MeshFile->seek(M2MBones[i].rotation.header.TimeStamp.ofs);
        for(u32 j=0; j<M2MBones[i].rotation.header.TimeStamp.num;j++)
        {
            MeshFile->read(&tempBoneTS, sizeof(u32));
            M2MBones[i].rotation.timestamps.push_back(tempBoneTS);
        }
    }
    if(M2MBones[i].scaling.header.TimeStamp.num>0)
    {
        MeshFile->seek(M2MBones[i].scaling.header.TimeStamp.ofs);
        for(u32 j=0; j<M2MBones[i].scaling.header.TimeStamp.num;j++)
        {
            MeshFile->read(&tempBoneTS, sizeof(u32));
            M2MBones[i].scaling.timestamps.push_back(tempBoneTS);
        }
    }
    if(M2MBones[i].translation.header.Values.num>0)
    {
        MeshFile->seek(M2MBones[i].translation.header.Values.ofs);
        for(u32 j=0; j<M2MBones[i].translation.header.Values.num*3;j++)
        {
            MeshFile->read(&tempBoneValue, sizeof(float));
            M2MBones[i].translation.values.push_back(tempBoneValue);
        }
    }
    if(M2MBones[i].rotation.header.Values.num>0)
    {
        MeshFile->seek(M2MBones[i].rotation.header.Values.ofs);
        for(u32 j=0; j<M2MBones[i].rotation.header.Values.num*4;j++)
        {
            if(header.version>=0x104)
            {
            s16 tempBoneShort;
            MeshFile->read(&tempBoneShort, sizeof(s16));
            tempBoneValue=(tempBoneShort>0?tempBoneShort-32767:tempBoneShort+32767)/32767.0f;
            M2MBones[i].rotation.values.push_back(tempBoneValue);
            }
            else
            {
            MeshFile->read(&tempBoneValue, sizeof(f32));
            M2MBones[i].rotation.values.push_back(tempBoneValue);
            }
        }
    }
    if(M2MBones[i].scaling.header.Values.num>0)
    {
        MeshFile->seek(M2MBones[i].scaling.header.Values.ofs);
        for(u32 j=0; j<M2MBones[i].scaling.header.Values.num*3;j++)
        {
            MeshFile->read(&tempBoneValue, sizeof(float));
            M2MBones[i].scaling.values.push_back(tempBoneValue);
        }
    }
}

DEBUG(logdebug("Read %u Bones",M2MBones.size()));
}

void CM2MeshFileLoader::ReadBonesWOTLK()
{
//Bones. This is global data
Bone tempBone;
if(!M2MBones.empty())
{
    M2MBones.clear();
}
MeshFile->seek(header.Bones.ofs);
for(u32 i=0;i<header.Bones.num;i++)
{
    MeshFile->read(&tempBone,16);
    //Skip Interpolation Ranges
    MeshFile->read(&tempBone.translation.header,4);
    MeshFile->read(&tempBone.translation.header.TimeStamp,sizeof(numofs));
    MeshFile->read(&tempBone.translation.header.Values,sizeof(numofs));
    MeshFile->read(&tempBone.rotation.header,4);
    MeshFile->read(&tempBone.rotation.header.TimeStamp,sizeof(numofs));
    MeshFile->read(&tempBone.rotation.header.Values,sizeof(numofs));
    MeshFile->read(&tempBone.scaling.header,4);
    MeshFile->read(&tempBone.scaling.header.TimeStamp,sizeof(numofs));
    MeshFile->read(&tempBone.scaling.header.Values,sizeof(numofs));

    MeshFile->read(&tempBone.PivotPoint,sizeof(core::vector3df));
    tempBone.PivotPoint=fixCoordSystem(tempBone.PivotPoint);
    M2MBones.push_back(tempBone);
    DEBUG(logdebug("Bone %u Parent %u PP %f %f %f Flags %X",i,tempBone.parentBone,tempBone.PivotPoint.X,tempBone.PivotPoint.Y,tempBone.PivotPoint.Z, tempBone.flags));
}
//Fill in values referenced in Bones. local to each bone
//Interpolation Ranges are not used
u32 tempBoneTS;
numofs tempBoneNumOfs;
core::array<numofs> tempNumOfsList;
float tempBoneValue;
s16 tempBoneShort;
//Animations can now be stored in anim files. To read quickly we need to swap the order of things.
//We are not loading stuff by bone but by animation - every bone has data for all the animations (I hope...)
//Animations MUST BE LOADED FIRST!!!!!
bool use_animfile;
io::IReadFile* AnimFile;
for(u32 i=0; i<M2MAnimations.size(); i++)
{
  use_animfile = (M2MAnimations[i].flags & 0x20) == 0;
  if(use_animfile)
  {
    std::string AnimName = MeshFile->getFileName();
    c8 ext[13];
    sprintf(ext,"%04d-%02d.anim",M2MAnimations[i].animationID,M2MAnimations[i].subanimationID);
    AnimName = AnimName.substr(0, AnimName.length()-3) + ext;
    AnimFile = io::IrrCreateIReadFileBasic(Device, AnimName.c_str());
    if (!AnimFile)
    {
        logerror("Error! Anim file not found: %s", AnimName.c_str());
        if(M2MAnimations[i].flags & 0x40)
        {
          logerror("We actually expected this error - Where is the data for this animation???");
        }
        continue;
    }
  }
  else
  {
    AnimFile = MeshFile;
  }
  for(u32 j=0; j<M2MBones.size(); j++)
  {
    if(M2MBones[j].translation.header.TimeStamp.num>i && M2MBones[j].translation.header.Values.num>i)
    {
      //Timestamps
      MeshFile->seek(M2MBones[j].translation.header.TimeStamp.ofs+8*i);
      MeshFile->read(&tempBoneNumOfs, sizeof(numofs));
      if(tempBoneNumOfs.num > 0)
      {
        AnimFile->seek(tempBoneNumOfs.ofs);
        for(u32 k=0; k<tempBoneNumOfs.num;k++)
        {
          AnimFile->read(&tempBoneTS, sizeof(u32));
          M2MBones[j].translation.timestamps.push_back(tempBoneTS+M2MAnimations[i].start);//Bit of a HACK squash Multitimeline into single timeline.
        }
      }
      //Values
      MeshFile->seek(M2MBones[j].translation.header.Values.ofs+8*i);
      MeshFile->read(&tempBoneNumOfs, sizeof(numofs));
      if(tempBoneNumOfs.num > 0)
      {
        AnimFile->seek(tempBoneNumOfs.ofs);
        for(u32 k=0; k<tempBoneNumOfs.num*3;k++)
        {
          AnimFile->read(&tempBoneValue, sizeof(float));
          M2MBones[j].translation.values.push_back(tempBoneValue);
        }
      }
    }
    if(M2MBones[j].rotation.header.TimeStamp.num>i && M2MBones[j].rotation.header.Values.num>i)
    {
      //Timestamps
      MeshFile->seek(M2MBones[j].rotation.header.TimeStamp.ofs+8*i);
      MeshFile->read(&tempBoneNumOfs, sizeof(numofs));
      if(tempBoneNumOfs.num > 0)
      {
        AnimFile->seek(tempBoneNumOfs.ofs);
        for(u32 k=0; k<tempBoneNumOfs.num;k++)
        {
          AnimFile->read(&tempBoneTS, sizeof(u32));
          M2MBones[j].rotation.timestamps.push_back(tempBoneTS+M2MAnimations[i].start);//Bit of a HACK squash Multitimeline into single timeline.
        }
      }
      //Values
      MeshFile->seek(M2MBones[j].rotation.header.Values.ofs+8*i);
      MeshFile->read(&tempBoneNumOfs, sizeof(numofs));
      if(tempBoneNumOfs.num > 0)
      {
        AnimFile->seek(tempBoneNumOfs.ofs);
        for(u32 k=0; k<tempBoneNumOfs.num*4;k++)
        {
          AnimFile->read(&tempBoneShort, sizeof(s16));
          tempBoneValue=(tempBoneShort>0?tempBoneShort-32767:tempBoneShort+32767)/32767.0f;
          M2MBones[j].rotation.values.push_back(tempBoneValue);
        }
      }
    }
    if(M2MBones[j].scaling.header.TimeStamp.num>i && M2MBones[j].scaling.header.Values.num>i)
    {
      //Timestamps
      MeshFile->seek(M2MBones[j].scaling.header.TimeStamp.ofs+8*i);
      MeshFile->read(&tempBoneNumOfs, sizeof(numofs));
      if(tempBoneNumOfs.num > 0)
      {
        AnimFile->seek(tempBoneNumOfs.ofs);
        for(u32 k=0; k<tempBoneNumOfs.num;k++)
        {
          AnimFile->read(&tempBoneTS, sizeof(u32));
          M2MBones[j].scaling.timestamps.push_back(tempBoneTS+M2MAnimations[i].start);//Bit of a HACK squash Multitimeline into single timeline.
        }
      }
      //Values
      MeshFile->seek(M2MBones[j].scaling.header.Values.ofs+8*i);
      MeshFile->read(&tempBoneNumOfs, sizeof(numofs));
      if(tempBoneNumOfs.num > 0)
      {
        AnimFile->seek(tempBoneNumOfs.ofs);
        for(u32 k=0; k<tempBoneNumOfs.num*3;k++)
        {
          AnimFile->read(&tempBoneValue, sizeof(float));
          M2MBones[j].scaling.values.push_back(tempBoneValue);
        }
      }
    }
  }
  if(use_animfile)
  {
    AnimFile->drop();
  }
  AnimFile=0;
}
DEBUG(logdebug("Read %u Bones",M2MBones.size()));


}


void CM2MeshFileLoader::ReadAnimationData()
{
//     //Global Sequences. This is global data
//     u32 tempGlobalSeq;
//     if(!M2MGlobalSequences.empty())
//     {
//         M2MGlobalSequences.clear();
//     }
//     MeshFile->seek(header.GlobalSequences.ofs);
//     for(u32 i=0;i<header.GlobalSequences.num;i++)
//     {
//         MeshFile->read(&tempGlobalSeq,sizeof(u32));
//         M2MGlobalSequences.push_back(tempGlobalSeq);
//         DEBUG(logdebug("Global Sequence %u End %u",i,tempGlobalSeq));
//     }
//     DEBUG(logdebug("Read %u Global Sequence entries",M2MGlobalSequences.size()));
// 
//     //BoneLookupTable. This is global data
//     u16 tempBoneLookup;
//     if(!M2MBoneLookupTable.empty())
//     {
//         M2MBoneLookupTable.clear();
//     }
//     MeshFile->seek(header.BoneLookupTable.ofs);
//     for(u32 i=0;i<header.BoneLookupTable.num;i++)
//     {
//         MeshFile->read(&tempBoneLookup,sizeof(u16));
//         M2MBoneLookupTable.push_back(tempBoneLookup);
//         DEBUG(logdebug("BoneLookupTable %u Value %u",i,tempBoneLookup));
//     }
//     DEBUG(logdebug("Read %u BoneLookupTable entries",M2MBoneLookupTable.size()));
// 
//     //SkeleBoneLookupTable. This is global data
//     u16 tempSkeleBoneLookup;
//     if(!M2MSkeleBoneLookupTable.empty())
//     {
//         M2MSkeleBoneLookupTable.clear();
//     }
//     MeshFile->seek(header.SkelBoneLookup.ofs);
//     for(u32 i=0;i<header.SkelBoneLookup.num;i++)
//     {
//         MeshFile->read(&tempSkeleBoneLookup,sizeof(u16));
//         M2MSkeleBoneLookupTable.push_back(tempSkeleBoneLookup);
//         DEBUG(logdebug("SkeleBoneLookupTable %u Value %u",i,tempSkeleBoneLookup));
//     }
//     DEBUG(logdebug("Read %u SkeleBoneLookupTable entries",M2MSkeleBoneLookupTable.size()));

    //Animations. This is global data
    u32 laststart = 0;
    if(!M2MAnimations.empty())
    {
        M2MAnimations.clear();
    }
    MeshFile->seek(header.Animations.ofs);
    for(u32 i=0;i<header.Animations.num;i++)
    {
        Animation tempAnimation;
        if(header.version < 0x108)
        {
          RawAnimation tempRaw;
          MeshFile->read(&tempRaw,sizeof(RawAnimation));

          tempAnimation.animationID = tempRaw.animationID;
          tempAnimation.probability = tempRaw.probability / 32767.0f;
          tempAnimation.start = tempRaw.start;
          tempAnimation.end = tempRaw.end;
        }
        else
        {
          RawAnimationWOTLK tempRaw;
          MeshFile->read(&tempRaw,sizeof(RawAnimationWOTLK));
          tempAnimation.animationID = tempRaw.animationID;
          tempAnimation.subanimationID = tempRaw.subanimationID;
          tempAnimation.flags = tempRaw.flags;
          tempAnimation.probability = tempRaw.probability / 32767.0f;
          tempAnimation.start = laststart;
          tempAnimation.end = laststart + tempRaw.length;
          laststart += tempRaw.length + 1000;
        }
        M2MAnimations.push_back(tempAnimation);
        DEBUG(logdebug("Animation %u Id %u Start %u End %u",i,tempAnimation.animationID,tempAnimation.start,tempAnimation.end));
    }
    DEBUG(logdebug("Read %u Animations",M2MAnimations.size()));
//     //Animation Lookup. This is global data
//     s16 tempAniLookup;
//     if(!M2MAnimationLookup.empty())
//     {
//         M2MAnimationLookup.clear();
//     }
//     MeshFile->seek(header.AnimationLookup.ofs);
//     for(u32 i=0;i<header.AnimationLookup.num;i++)
//     {
//         MeshFile->read(&tempAniLookup,sizeof(s16));
//         M2MAnimationLookup.push_back(tempAniLookup);
//         DEBUG(logdebug("Animation Lookup %u Id %u",i,tempAniLookup));
//     }
//     DEBUG(logdebug("Read %u AnimationLookup",M2MAnimationLookup.size()));
}

void CM2MeshFileLoader::ReadTextureDefinitions()
{
    //Texture Lookup table. This is global data
    u16 tempM2TexLookup;
    if(!M2MTextureLookup.empty())
    {
        M2MTextureLookup.clear();
    }
    MeshFile->seek(header.TexLookup.ofs);
    for(u32 i=0;i<header.TexLookup.num;i++)
    {
        MeshFile->read(&tempM2TexLookup,sizeof(u16));
        M2MTextureLookup.push_back(tempM2TexLookup);
        DEBUG(logdebug("Texture %u Type %u",i,tempM2TexLookup));
    }
    DEBUG(logdebug("Read %u Texture lookup entries",M2MTextureLookup.size()));

    //Texture Definitions table. This is global data
    TextureDefinition tempM2TexDef;
    if(!M2MTextureDef.empty())
    {
        M2MTextureDef.clear();
    }
    MeshFile->seek(header.Textures.ofs);
    for(u32 i=0;i<header.Textures.num;i++)
    {
        MeshFile->read(&tempM2TexDef,sizeof(TextureDefinition));
        M2MTextureDef.push_back(tempM2TexDef);
        DEBUG(logdebug("Texture %u Type %u",i,tempM2TexDef.texType));
    }
    DEBUG(logdebug("Read %u Texture Definition entries",M2MTextureDef.size()));

    //Render Flags table. This is global data
    RenderFlags tempM2RF;
    if(!M2MRenderFlags.empty())
    {
        M2MRenderFlags.clear();
    }
    MeshFile->seek(header.TexFlags.ofs);
    for(u32 i=0;i<header.TexFlags.num;i++)
    {
        MeshFile->read(&tempM2RF,sizeof(RenderFlags));
        M2MRenderFlags.push_back(tempM2RF);
        DEBUG(logdebug("Flag %u: (%u, %u)",i,tempM2RF.blending,tempM2RF.flags));
    }
    DEBUG(logdebug("Read %u Renderflags",M2MRenderFlags.size()));

    if(!M2MTextureFiles.empty())
        M2MTextureFiles.clear();

    std::string tempTexFileName="";
    M2MTextureFiles.reallocate(M2MTextureDef.size());
    for(u32 i=0; i<M2MTextureDef.size(); i++)
    {
        tempTexFileName.resize(M2MTextureDef[i].texFileLen + 1);
        MeshFile->seek(M2MTextureDef[i].texFileOfs);
        MeshFile->read((void*)tempTexFileName.data(),M2MTextureDef[i].texFileLen);
        M2MTextureFiles.push_back("");
        M2MTextureFiles[i]=tempTexFileName.c_str();
        DEBUG(logdebug("Texture: %u %u (%s/%s) @ %u(%u)",i,M2MTextureFiles.size(),M2MTextureFiles[i].c_str(),tempTexFileName.c_str(),M2MTextureDef[i].texFileOfs,M2MTextureDef[i].texFileLen));
    }
}


bool CM2MeshFileLoader::load()
{
DEBUG(logdebug("Trying to open file %s",MeshFile->getFileName()));
logdebug("Trying to open file %s",MeshFile->getFileName());

MeshFile->read(&header,20);
DEBUG(logdebug("M2 Version %X",header.version));

switch(header.version)
{
  case 0x100:
  case 0x104://NEED CHECK
  case 0x105://NEED CHECK
  case 0x106://NEED CHECK
  case 0x107://NEED CHECK
  {
    MeshFile->read((u8*)&header+20,sizeof(ModelHeader)-20);
    MeshFile->seek(header.Views.ofs);
    MeshFile->read(&currentView,sizeof(ModelView));
    ReadViewData(MeshFile);
    ReadVertices();

    ReadTextureDefinitions();
    ReadAnimationData();

    ReadBones();
    break;
  }
  case 0x108:
  {
    //This is pretty ugly... Any suggestions how to make this nicer?
    MeshFile->read((u8*)&header+0x14,24);//nGlobalSequences - ofsAnimationLookup
    MeshFile->read((u8*)&header+0x34,28);//nBones - nViews
    MeshFile->read((u8*)&header+0x54,24);//nColors - nTransparency
    MeshFile->read((u8*)&header+0x74,sizeof(ModelHeader)-0x74);//nTexAnims - END

    std::string SkinName = MeshFile->getFileName();
    SkinName = SkinName.substr(0, SkinName.length()-3) + "00.skin"; // FIX ME if we need more skins
    io::IReadFile* SkinFile = io::IrrCreateIReadFileBasic(Device, SkinName.c_str());
    if (!SkinFile)
    {
        logerror("Error! Skin file not found: %s", SkinName.c_str());
        return 0;
    }
    SkinFile->seek(4); // Header of Skin Files is always SKIN
    SkinFile->read(&currentView,sizeof(ModelView));
    ReadViewData(SkinFile);
    SkinFile->drop();

    ReadVertices();

    ReadTextureDefinitions();
    ReadAnimationData();

    ReadBonesWOTLK();
    break;
  }
  default:
  {
    logerror("M2: [%s] Wrong header %0X! File version doesn't match or file is not a M2 file.",MeshFile->getFileName(),header.version);
    return 0;
  }
}

///////////////////////////
// EVERYTHING IS READ
///////////////////////////


///////////////////////////////////////
//      Animation related stuff      //
///////////////////////////////////////
for(u32 i=0;i<M2MAnimations.size();i++)
{
  AnimatedMesh->newAnimation(M2MAnimations[i].animationID,M2MAnimations[i].start,M2MAnimations[i].end,M2MAnimations[i].probability);
}


scene::CM2Mesh::SJoint* Joint;
for(u32 i=0;i<M2MBones.size();i++)
{
  if(M2MBones[i].parentBone == -1)
  {
      ParentJoint=(scene::CM2Mesh::SJoint*)0;
  }
  else
  {
      ParentJoint=AnimatedMesh->getAllJoints()[M2MBones[i].parentBone];
  }
  Joint=AnimatedMesh->createJoint(ParentJoint);

  if(M2MBones[i].translation.timestamps.size()>0)
  {
    for(u32 j=0;j<M2MBones[i].translation.timestamps.size();j++)
    {
      scene::CM2Mesh::SPositionKey* pos=AnimatedMesh->createPositionKey(Joint);
      pos->frame=M2MBones[i].translation.timestamps[j];
      pos->position=fixCoordSystem(core::vector3df(M2MBones[i].translation.values[j*3],M2MBones[i].translation.values[j*3+1],M2MBones[i].translation.values[j*3+2]));
    }
  }
  if(M2MBones[i].rotation.timestamps.size()>0)
  {
    for(u32 j=0;j<M2MBones[i].rotation.timestamps.size();j++)
    {
      scene::CM2Mesh::SRotationKey* rot=AnimatedMesh->createRotationKey(Joint);
      rot->frame=M2MBones[i].rotation.timestamps[j];
      core::quaternion tempQ=core::quaternion(M2MBones[i].rotation.values[j*4+0],M2MBones[i].rotation.values[j*4+1],M2MBones[i].rotation.values[j*4+2],M2MBones[i].rotation.values[j*4+3]);
      tempQ = fixQuaternion(tempQ);
      tempQ.normalize();
      rot->rotation=tempQ;
    }
  }

  if(M2MBones[i].scaling.timestamps.size()>0)
  {
    for(u32 j=0;j<M2MBones[i].scaling.timestamps.size();j++)
    {
    scene::CM2Mesh::SScaleKey* scale=AnimatedMesh->createScaleKey(Joint);
    scale->frame=M2MBones[i].scaling.timestamps[j];
    scale->scale=core::vector3df(M2MBones[i].scaling.values[j*3],M2MBones[i].scaling.values[j*3+1],M2MBones[i].scaling.values[j*3+2]);
    }
  }

  Joint->Animatedposition=M2MBones[i].PivotPoint;
  Joint->Animatedscale=core::vector3df(1.0f,1.0f,1.0f);
  Joint->Animatedrotation=core::quaternion(0.0f,0.0f,0.0f,1.0f);

  core::matrix4 positionMatrix;
  positionMatrix.setTranslation( Joint->Animatedposition );

  core::matrix4 rotationMatrix = Joint->Animatedrotation.getMatrix();

  core::matrix4 scaleMatrix;
  scaleMatrix.setScale( Joint->Animatedscale );

  Joint->GlobalMatrix = positionMatrix * rotationMatrix * scaleMatrix;
}





//And M2MVertices are not usable like this. Thus we put it into an irrlicht S3DVertex

if(M2Vertices.size()>0)
    M2Vertices.clear();

for(u32 i=0;i<M2MVertices.size();i++)
{
    M2Vertices.push_back(video::S3DVertex(core::vector3df(M2MVertices[i].pos.X,M2MVertices[i].pos.Y,M2MVertices[i].pos.Z),core::vector3df(M2MVertices[i].normal.X,M2MVertices[i].normal.Y,M2MVertices[i].normal.Z), video::SColor(255,100,100,100),M2MVertices[i].texcoords));
}
//Loop through the submeshes
for(u32 i=0; i < currentView.Submesh.num;i++)//
{
    //Now, M2MTriangles refers to M2MIndices and not to M2MVertices.
    scene::SSkinMeshBuffer *MeshBuffer = AnimatedMesh->createBuffer(M2MSubmeshes[i].meshpartId);

    //Put the Indices and Vertices of the Submesh into a mesh buffer
    //Each Submesh contains only the Indices and Vertices that belong to it.
    //Because of this the Index values for the Submeshes must be corrected by the Vertex offset of the Submesh
    for(u32 j=M2MSubmeshes[i].ofsTris;j<M2MSubmeshes[i].ofsTris+M2MSubmeshes[i].nTris;j++)
    {
        MeshBuffer->Indices.push_back(M2MIndices[M2MTriangles[j]]-M2MSubmeshes[i].ofsVertex);
    }

    for(u32 j=M2MSubmeshes[i].ofsVertex;j<M2MSubmeshes[i].ofsVertex+M2MSubmeshes[i].nVertex;j++)
    {
        MeshBuffer->Vertices_Standard.push_back(M2Vertices[j]);
        for(u32 k=0; k<4; k++)
        {
            if((M2MVertices[j].weights[k])>0)
            {
            u32 boneIndex = M2MVertices[j].bones[k];
            scene::CM2Mesh::SWeight* weight = AnimatedMesh->createWeight(AnimatedMesh->getAllJoints()[boneIndex]);
            weight->strength=M2MVertices[j].weights[k];
            weight->vertex_id=MeshBuffer->Vertices_Standard.size()-1;
            weight->buffer_id=i;
            }

        }
    }


    MeshBuffer->recalculateBoundingBox();
    for(u32 j=0;j<M2MTextureUnit.size();j++)//Loop through texture units
    {
        if(M2MTextureUnit[j].submeshIndex1==i && !M2MTextureFiles[M2MTextureLookup[M2MTextureUnit[j].textureIndex]].empty())//if a texture unit belongs to this submesh
        {
            char buf[1000];
            MemoryDataHolder::MakeTextureFilename(buf,M2MTextureFiles[M2MTextureLookup[M2MTextureUnit[j].textureIndex]].c_str());
            video::ITexture* tex = Device->getVideoDriver()->findTexture(buf);
            if(!tex)
            {
              io::IReadFile* TexFile = io::IrrCreateIReadFileBasic(Device, buf);
              if (!TexFile)
              {
                  logerror("CM2MeshFileLoader: Texture file not found: %s", buf);
                  continue;
              }
            tex = Device->getVideoDriver()->getTexture(TexFile);
            TexFile->drop();
            }
            MeshBuffer->getMaterial().setTexture(M2MTextureUnit[j].TextureUnitNumber,tex);

            DEBUG(logdebug("Render Flags: %u %u",M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].flags,M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].blending));
            MeshBuffer->getMaterial().BackfaceCulling=(M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].flags & 0x04)?false:true;
            switch(M2MRenderFlags[M2MTextureUnit[j].renderFlagsIndex].blending)
            {
              case 1:
                MeshBuffer->getMaterial().MaterialType=video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF ;
                break;
              case 2:
              case 4:
              MeshBuffer->getMaterial().MaterialType=video::EMT_TRANSPARENT_ALPHA_CHANNEL;
              DEBUG(logdebug("Alpha Channel Transparency on"));
              break;
            }
        }

    }

    MeshBuffer->recalculateBoundingBox();
    if(header.Animations.num==0 && header.GlobalSequences.num == 0)
      MeshBuffer->setHardwareMappingHint(EHM_STATIC);
    else
      MeshBuffer->setHardwareMappingHint(EHM_STREAM);

}
Device->getSceneManager()->getMeshManipulator()->flipSurfaces(AnimatedMesh);

M2MTriangles.clear();
M2Vertices.clear();
M2Indices.clear();
M2MIndices.clear();
M2MVertices.clear();
M2MRenderFlags.clear();
M2MTextureUnit.clear();
M2MTextureDef.clear();
M2MSubmeshes.clear();
M2MTextureFiles.clear();
M2MTextureLookup.clear();
return true;
}

}
}
