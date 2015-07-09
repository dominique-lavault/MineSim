#pragma once

#include <irrlicht.h>
#include "SimMine.h"
#include "galerie.h"
#include "CGUIExtListBox.h"
#include "irrland files/LandManager.h"
#include "irrland files/irrLandCam.h"
class BillBoardSceneNode;
class IShakeItAnimator;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class globalData
{
public:
	globalData(void);
	~globalData(void);

	irrLandCam* m_camera;
	ISceneManager* smgr;
	IVideoDriver* driver;
	IMetaTriangleSelector * m_metaSelector;
	Galerie m_galerie;
	CLandManager * m_landManager;
	vector3df m_origin;	// départ de la galerie
	// meshes dispo un peu partout tout le temps
	IAnimatedMesh * pCarreau;
	IAnimatedMesh * pPorche;
	IAnimatedMesh * pEtagePuits, *m_pOreRock;
	array<IAnimatedMeshSceneNode*> m_carreauSceneNodeArray, m_porcheSceneNodeArray, m_etagePuitsSceneNodeArray,m_oreRockSceneNodeArray;
	CGUIExtListBox * m_messagelistbox;
	bool bMouseCursorCaptured;
	BillBoardSceneNode *myBillboardNode;
	IShakeItAnimator * anim;
	scene::IParticleSystemSceneNode* ps;
	int particleFXFPS;
};
