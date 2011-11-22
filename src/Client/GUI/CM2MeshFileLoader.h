#include "irrlicht/irrlicht.h"
#include "irrlicht/IMeshLoader.h"
#include "CM2Mesh.h"
#include <string>
#include <vector>
#include <algorithm>

namespace irr
{
namespace scene
{

struct numofs {
    u32 num;
    u32 ofs;
};


struct ModelHeader {
    c8 id[4];               //0x00
    u32 version;
    numofs name;
    u32 type;               //0x10

    //Anim Block @ 0x14
    numofs GlobalSequences;
    numofs Animations;
    numofs AnimationLookup;
    numofs D;                   //Absent in WOTLK
    numofs Bones;
    numofs SkelBoneLookup;
    numofs Vertices;
    numofs Views;               //ofs Absent in WOTLK
    numofs Colors;
    numofs Textures;
    numofs Transparency;
    numofs I;                   //Absent in WoTLK
    numofs TexAnims;
    numofs TexReplace;
    numofs TexFlags;
    numofs BoneLookupTable;
    numofs TexLookup;
    numofs TexUnitLookup;
    numofs TransparencyLookup;
    numofs TexAnimLookup;

	f32 floats[14];

    numofs BoundingTriangles;
    numofs BoundingVertices;
    numofs BoundingNormals;

    numofs Attachments;
    numofs AttachLookup;
    numofs Events;
    numofs Lights;
    numofs Cameras;
    numofs CameraLookup;
    numofs RibbonEmitters;
    numofs ParticleEmitters;

	//WOTLK has one more field which is only set under specific conditions.

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
    numofs Index;       // Vertices in this model (index into vertices[])
    numofs Triangle;    // indices
    numofs Properties;  // additional vtx properties (mostly 0?)
    numofs Submesh;     // submeshes
    numofs Tex;         // material properties/textures
    u32 lod;            // LOD bias? unknown
};

struct ModelViewSubmesh { //Curse you BLIZZ for not using numofs here
    u32 meshpartId;
    u16 ofsVertex;//Starting vertex number for this submesh
    u16 nVertex;
    u16 ofsTris;//Starting Triangle index
    u16 nTris;
    u16 nBone, ofsBone, unk3, unk4;
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

struct RawAnimation{
    u32 animationID;
    u32 start, end;
    float movespeed;
    u32 loop, probability, unk1, unk2;
    u32 playbackspeed;
    float bbox[6];
    float radius;
    s16 indexSameID;
    u16 unk3;
};

struct RawAnimationWOTLK{
    u16 animationID, subanimationID;
    u32 length;
    float movespeed;
    u32 flags;
    u16 probability, unused;
    u32 unk1, unk2, playbackspeed;
    float bbox[6];
    float radius;
    s16 indexSameID;
    u16 index;
};

struct Animation{
  u32 animationID;
  u32 subanimationID;
  u32 start, end;
  u32 flags;
  f32 probability;
};

struct AnimBlockHead{
    s16 interpolationType;
    s16 globalSequenceID;
    numofs InterpolationRanges;    //Missing in WotLK
    numofs TimeStamp;
    numofs Values;
};

// struct InterpolationRange{
//     u32 start, end;
// };

struct AnimBlock{
    AnimBlockHead header;
//     core::array<InterpolationRange> keyframes;  // We are not using this
    core::array<u32> timestamps;
    core::array<float> values;
};

struct Bone{
    s32 SkelBoneIndex;
    u32 flags;
    s16 parentBone;
    u16 unk1;
    u16 unk2;
    u16 unk3;
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
    void ReadBones();
    void ReadBonesWOTLK();
    void ReadVertices();
    void ReadTextureDefinitions();
    void ReadAnimationData();
    void ReadViewData(io::IReadFile* file);

	IrrlichtDevice *Device;
    core::stringc Texdir;
    io::IReadFile *MeshFile, *SkinFile;

    CM2Mesh *AnimatedMesh;
    scene::CM2Mesh::SJoint *ParentJoint;



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
    core::array<u32> M2MGlobalSequences;
    core::array<Animation> M2MAnimations;
    core::array<s16> M2MAnimationLookup;
    core::array<Bone> M2MBones;
    core::array<u16> M2MBoneLookupTable;
    core::array<u16> M2MSkeleBoneLookupTable;
    //Used for the Mesh, thus m2_noM_*
    core::array<video::S3DVertex> M2Vertices;
    core::array<u16> M2Indices;
    core::array<scene::ISkinnedMesh::SJoint> M2Joints;


};
}//namespace scene
}//namespace irr
