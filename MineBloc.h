#pragma once

#include <irrlicht.h>
#include "MineProfil.h"
#include "MineWall.h"

using namespace irr;
#define NO_MINEWALL (MineWall*)-1
#define NO_MINEPROFIL (WALL_ID)-1

class MineBloc
{
public:
	// public props
	core::aabbox3d<f32> m_BBox;

	// public methods
	MineBloc::MineBloc(int noBloc, MineBloc * parentBloc, vector3df *pVecteur, vector3df *pAbsVecteur, WALL_ID parentWallId, WALL_ID bForcedProfil=NO_MINEPROFIL, MineProfil * forcedProfil=NULL);
	~MineBloc(void);
	
	MineProfil getProfil(WALL_ID noWall);
	void MineBloc::setProfil(MineProfil * profil, WALL_ID wallId);
	MineWall * getWallSceneNode(WALL_ID wallID);
	MineProfil MineBloc::computeProfil(WALL_ID wallID);
	vector3df MineBloc::getBlocAbsoluteVector(void);
	int getNoBloc(void);
	MineBloc * getParentBloc(void);
	MineWall * getSelectedMineWall(line3d<f32> *pLine,vector3df * pIntersectionPoint=NULL,bool onlyActiveWalls=true);
	vector3df getBlocVector(void);
	MineBloc * MineBloc::getChildMineBloc(WALL_ID targetWallId);
	void MineBloc::setChildMineBloc(WALL_ID noWall, MineBloc * childBloc);
	void MineBloc::setVisible(bool bVisible);
	void MineBloc::setLOD(LOD lod);

protected :
	int m_noBloc;
	MineBloc * m_childBloc[NB_WALL_ID];
	vector3df m_blocVecteur;	// vecteur du bloc (du profil d'entrée au profil de sortie)
	vector3df m_absoluteVecteur;	// vecteur absolu de la position depuis l'entrée de la mine
	MineWall * m_wallSceneNodes[NB_WALL_ID];
	ITriangleSelector * selector[NB_WALL_ID];
	MineProfil m_profil[2];	// profil du bloc WALL_ID_FDTFRONT [1]  et WALL_ID_FDTBACK [0]
	MineBloc * m_pParentBloc;

	MineProfil MineBloc::getBillBoardedWall(vector3df * vecteur, MineProfil  * pProfil);
	void MineBloc::resetBBox();
};
