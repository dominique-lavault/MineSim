#pragma once

#include <irrlicht.h>
#include "MineBloc.h"
#include "MineBoisage.h"

#define MAX_NBLOCS 1500

#define ROOT_BLOCK 0
#define NO_PARENT_BLOCK -1

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class Galerie
{
public:
	Galerie(void);
	~Galerie(void);
	MineBloc * AddBloc(int parentBloc, WALL_ID noParentWall, vector3df vecteur, WALL_ID bForcedProfil=NO_MINEPROFIL, MineProfil * forcedProfil=NULL);
	MineWall * getSelectedMineWall(line3d<f32> *pLine,vector3df * pIntersectionPoint=NULL, bool onlyActiveWalls=true);
	int GetBlocCount(void);
	int getNoBloc(void);
	void setNoBloc(int noBloc);
	MineBloc * getBloc(int noBloc);
	int Galerie::getBlocContainingCamera();
	bool Galerie::getAltitudeAt(vector3df * pos, float * altitude);
	void setBlocMaterial(int noBloc, E_MATERIAL_FLAG material, bool activate);
	void setUndergroundPartVisible(bool visible);
	void Galerie::setLOD(vector3df & refPos);

	array<int> m_listeBlocEntree;	// liste des numéro de blocs sortant à l'extérieur

protected:
	int m_nbBloc;
	MineBloc * m_blocCollection[MAX_NBLOCS];
	int m_oldBlocContainingCamera;
	array<MineBoisage *>m_boisageCollection;

	line3d<f32> Galerie::getBlocVerticalLine(int blocIdx, vector3df * pos);
	MineProfil Galerie::makeMatchingProfil(vector2d<f32> myBlocVector, vector2d<f32> foreignBlocVector, MineProfil newProfil);
	bool Galerie::isEntrance(unsigned int noBloc);
};
