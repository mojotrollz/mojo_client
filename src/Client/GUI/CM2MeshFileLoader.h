#include "irrlicht/irrlicht.h"
#include "irrlicht/IMeshLoader.h"
#include "SSkinnedMesh.h"
#include <string>
#include <vector>
#include <algorithm>

namespace irr
{
namespace scene
{

struct ModelHeader {
	c8 id[4];               //0x00
	u32 version;            
	u32 nameLength;
	u32 nameOfs;            
	u32 type;               //0x10

    //Anim Block @ 0x14
	u32 nGlobalSequences;
	u32 ofsGlobalSequences;
	u32 nAnimations;
	u32 ofsAnimations;      //0x20
	u32 nAnimationLookup;
	u32 ofsAnimationLookup;
    u32 nD;
    u32 ofsD;               //0x30
	u32 nBones;
	u32 ofsBones;
	u32 nSkelBoneLookup;
	u32 ofsSkelBoneLookup;  //0x40

	u32 nVertices;          //0x44
	u32 ofsVertices;
	u32 nViews; // number of skins ?
    u32 ofsViews;           //0x50

	u32 nColors;
	u32 ofsColors;

	u32 nTextures;
	u32 ofsTextures;        //0x60

	u32 nTransparency;
	u32 ofsTransparency;
    u32 nI;
    u32 ofsI;               //0x70
	u32 nTexAnims;
	u32 ofsTexAnims;
	u32 nTexReplace;
	u32 ofsTexReplace;      //0x80

	u32 nTexFlags;
	u32 ofsTexFlags;
	u32 nY;
	u32 ofsY;               //0x90

	u32 nTexLookup;
	u32 ofsTexLookup;

	u32 nTexUnitLookup;
	u32 ofsTexUnitLookup;   //0xa0
	u32 nTransparencyLookup;
	u32 ofsTransparencyLookup;
	u32 nTexAnimLookup;
	u32 ofsTexAnimLookup;   //0xb0

	f32 floats[14];

	u32 nBoundingTriangles; 
	u32 ofsBoundingTriangles; //0xf0
	u32 nBoundingVertices;
	u32 ofsBoundingVertices;
	u32 nBoundingNormals;
	u32 ofsBoundingNormals; //0x100

	u32 nAttachments;
	u32 ofsAttachments;
	u32 nAttachLookup;
	u32 ofsAttachLookup;    //0x110
	u32 nAttachments_2;
	u32 ofsAttachments_2;
	u32 nLights;
	u32 ofsLights;          //0x120
	u32 nCameras;
	u32 ofsCameras;
	u32 nCameraLookup;
	u32 ofsnCameraLookup;   //0x130
	u32 nRibbonEmitters;
	u32 ofsRibbonEmitters;
	u32 nParticleEmitters;
	u32 ofsParticleEmitters;//0x140

};

struct TextureDefinition {
    u32 texType;
    u16 unk;
    u16 texFlags;
    u32 texFileLen;
    u32 texFileOfs;
};

struct ModelVertex {
	core::vector3df pos;
	u8 weights[4];
	u8 bones[4];
	core::vector3df normal;
	core::vector2df texcoords;
	u32 unk1, unk2; // always 0,0 so this is probably unused
};

struct ModelView {
//     c8 id[4]; // always "SKIN"
    u32 nIndex, ofsIndex; // Vertices in this model (index into vertices[])
    u32 nTris, ofsTris;	 // indices
    u32 nProps, ofsProps; // additional vtx properties
    u32 nSub, ofsSub;	 // materials/renderops/submeshes
    u32 nTex, ofsTex;	 // material properties/textures
	u32 lod;				 // LOD bias?
};

struct ModelViewSubmesh {
    u32 meshpartId;
    u16 ofsVertex;//Starting vertex number for this submesh
    u16 nVertex;
    u16 ofsTris;//Starting Triangle index
    u16 nTris;
    u16 unk1, unk2, unk3, unk4;
    core::vector3df v;
    float unkf[4];
};

struct TextureUnit{
    u16 Flags;
    s16 renderOrder;
    u16 submeshIndex1, submeshIndex2;
    s16 colorIndex;
    u16 renderFlagsIndex;
    u16 TextureUnitNumber;
    u16 unk1;
    u16 textureIndex;
    u16 TextureUnitNumber2;
    u16 transparencyIndex;
    u16 texAnimIndex;
};

struct RenderFlags{
    u16 flags;
    u16 blending;
};

struct Animation{
    u32 animationID;
    u32 start, end;
    float movespeed;
    u32 loop, flags, unk1, unk2;
    u32 playbackspeed;
    float bbox[6];
    float radius;
    s16 indexSameID;
    u16 unk3;
};

struct AnimBlockHead{
    s16 interpolationType;
    s16 globalSequenceID;
    u32 nInterpolationRange;
    u32 ofsInterpolationRange;
    u32 nTimeStamp;
    u32 ofsTimeStamp;
    u32 nValues;
    u32 ofsValues;
};

struct InterpolationRange{
    u32 start, end;
};

struct AnimBlock{
    AnimBlockHead header;
    core::array<InterpolationRange> keyframes;
    core::array<u32> timestamps;
    core::array<float> values;
};

struct Bone{
    s32 indexF;
    u32 flags;
    s16 parentBone;
    u16 unk1;
    u32 unk2;
    AnimBlock translation, rotation, scaling;
    core::vector3df PivotPoint;
};


class CM2MeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CM2MeshFileLoader(IrrlichtDevice* device);

	//! destructor
	virtual ~CM2MeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".cob")
	virtual bool isALoadableFileExtension(const c8* fileName)const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IUnknown::drop() for more information.
	virtual scene::IAnimatedMesh* createMesh(io::IReadFile* file);
private:

	bool load();
    void ReadVertices();
    void ReadTextureDefinitions();
    void ReadViewData(io::IReadFile* file);

	IrrlichtDevice *Device;
    core::stringc Texdir;
    io::IReadFile *MeshFile, *SkinFile;

    CSkinnedMesh *AnimatedMesh;
    scene::CSkinnedMesh::SJoint *ParentJoint;



    ModelHeader header;
    ModelView currentView;
    core::stringc M2MeshName;
    SMesh* Mesh;
    //SSkinMeshBuffer* MeshBuffer;
    //Taken from the Model file, thus m2M*
    core::array<ModelVertex> M2MVertices;
    core::array<u16> M2MIndices;
    core::array<u16> M2MTriangles;
    core::array<ModelViewSubmesh> M2MSubmeshes;
    core::array<u16> M2MTextureLookup;
    core::array<TextureDefinition> M2MTextureDef;
    core::array<std::string> M2MTextureFiles;
    core::array<TextureUnit> M2MTextureUnit;
    core::array<RenderFlags> M2MRenderFlags;
    core::array<Animation> M2MAnimations;
    core::array<Bone> M2MBones;
    //Used for the Mesh, thus m2_noM_*
    core::array<video::S3DVertex> M2Vertices;
    core::array<u16> M2Indices;
    core::array<scene::ISkinnedMesh::SJoint> M2Joints;


};
}//namespace scene
}//namespace irr
