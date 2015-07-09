#include <irrlicht.h>

class CLandManager;

#define NBBLOCKSAMPLES 17
#define NBS NBBLOCKSAMPLES	/* nombre d'échantillons sur une ligne d'un block */
#define NBBLOCKVERTICES (NBS*NBS)
#define NBBLOCKFACES (2*(NBS-1)*(NBS-1))

#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)

#define XZSCALE 35.0f
#define YSCALE 1.0f	// good for perlin data
#define PERLINSCALE 400.0f	// for perlin data
//#define PERLINSCALE 1.f;	// for real data
//#define YSCALE 0.3f	// good for real data
#define ALTITUDE_COLORTABLE_RATIO 0.25f
#define CLIFF_COLOR 0xffb0b0b0
#define SEA_COLOR 0xff7070ff
#define CLIFF_STEEP 30.f		// has to be >10.f

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CLandSceneNode : public scene::ISceneNode
{
public:
	core::aabbox3d<f32> Box;
	video::S3DVertex * Vertices; // [NBBLOCKVERTICES];
	u16 * m_indices;
	vector3df m_offset;	// offset du coin "haut gauche" au centre du monde
	video::SMaterial Material;
	int m_mipmapLevel;

	CLandSceneNode::CLandSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
	CLandSceneNode::~CLandSceneNode();
	
	void CLandSceneNode::generateMesh( CLandManager * pLandManager, vector3df * cameraPosition);
	void CLandSceneNode::updateTesslation( CLandManager * pLandManager, vector3df * cameraPosition, int mml_N, int mml_S, int mml_O, int mml_E);
	void CLandSceneNode::setOffset(vector3df * pOffset);
	float CLandSceneNode::getDeltaAltitude(int idx);
	void CLandSceneNode::setDeltaAltitude(int idx, float value);

protected:
	video::SColor heightColors[256];
	int m_nbFaceLevel;
	bool m_doU0;
	bool m_doUM;
	bool m_doV0;
	bool m_doVM;
	float * m_deltaAltitude; //[NBBLOCKVERTICES];

	size_t CLandSceneNode::loadFile(const char * filename, void * buf, size_t size);
	virtual video::SMaterial& CLandSceneNode::getMaterial(s32 i);
	virtual s32 CLandSceneNode::getMaterialCount();
	virtual const core::aabbox3d<f32>& CLandSceneNode::getBoundingBox() const;
	virtual void CLandSceneNode::render();
	virtual void CLandSceneNode::OnPreRender();

	// tesslation primitives
	void CLandSceneNode::tesslateLevel(int * m_nbFaceLevel, int level, u16 * m_indices1);
	int CLandSceneNode::tesslateFaceALevel(int u, int v, int i, int level, u16 * m_indices, int * m_nbFaceLevel);
	int CLandSceneNode::tesslateFaceBLevel(int u, int v, int i, int level, u16 * m_indices, int * m_nbFaceLevel);
};
