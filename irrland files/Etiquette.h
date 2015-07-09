#include <irrlicht.h>

class CLandManager;

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CEtiquette : public scene::ISceneNode
{
public:
	enum ETIQUETTE_STATUS {ON, OFF};
	enum ETIQUETTE_UPDATE {UPDATING, UPDATED};

	float m_lat, m_lon;
	char m_type;

	core::aabbox3d<f32> Box;
	video::S3DVertex * Vertices;
	u16 * m_indices;
	vector3df m_offset;	// offset du coin "haut gauche" au centre du monde
	video::SMaterial Material;

	CEtiquette::CEtiquette(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
	CEtiquette::~CEtiquette();
	
	void CEtiquette::setOffset(vector3df * pOffset);
	string<wchar_t> m_label;

	void CEtiquette::switchOn();
	void CEtiquette::switchOff();
	ETIQUETTE_STATUS CEtiquette::getStatus();
	CEtiquette::ETIQUETTE_UPDATE CEtiquette::getUpdateStatus();
	void CEtiquette::setUpdateStatus(CEtiquette::ETIQUETTE_UPDATE status);
	const wchar_t * CEtiquette::getLibelleType();
protected:
	int m_nbVertices;
	int m_nbIndices;
	ETIQUETTE_STATUS m_status;
	ETIQUETTE_UPDATE m_updateStatus;

	void CEtiquette::addFace(u16 p1, u16 p2, u16 p3);
	size_t CEtiquette::loadFile(const char * filename, void * buf, size_t size);
	virtual video::SMaterial& CEtiquette::getMaterial(s32 i);
	virtual s32 CEtiquette::getMaterialCount();
	virtual const core::aabbox3d<f32>& CEtiquette::getBoundingBox() const;
	virtual void CEtiquette::render();
	virtual void CEtiquette::OnPreRender();
};
