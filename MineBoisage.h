#pragma once

#include <irrlicht.h>
#include "mineProfil.h"
#include "mineBloc.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class MineBoisage : public scene::ISceneNode
{
public:
	MineBoisage::MineBoisage(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, MineProfil * profil, MineBloc * pMineBloc, vector3df vecteurGalerie);
	~MineBoisage(void);

	MineProfil * getProfil();
	void setProfil(MineProfil * pProfil);
	MineBloc * MineBoisage::getMineBloc(void);

	// irrlicht iSceneNode overloads
	SMesh * MineBoisage::getMesh();
	virtual video::SMaterial& getMaterial(s32 i);
	virtual s32 getMaterialCount();
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual void render();
	virtual void OnPreRender();
	virtual void remove();

protected:
	SMesh* m_mesh;
	core::aabbox3d<f32> Box;
	SMeshBuffer* m_buffer;
	MineBloc * m_pMineBloc;
	MineProfil m_profil;

	vector3df MineBoisage::computePoint(int noPt, int nbPt, float diameter, vector3df center, float angleY, bool bVertical);
	int MineBoisage::generateCylinder(int faces, vector3df &pt1, vector3df &pt2, SMeshBuffer * buffer, float angleY, bool bHorizontal);
	
	S3DVertex MineBoisage::makeS3DVertex(vector3df pos, vector3df offset, dimension2d<f32> tcoords, vector3df &center);
	void MineBoisage::pushPoly(SMeshBuffer * buffer, int v1, int v2, int v3);
	int MineBoisage::generatePoutre(float diameter, vector3df &pt1, vector3df &pt2, SMeshBuffer * buffer, vector3df vecteurGalerie);
};