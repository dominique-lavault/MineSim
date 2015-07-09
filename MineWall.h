#pragma once

#include <irrlicht.h>
#include "mineProfil.h"
class MineBloc;

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

enum WALL_ID {WALL_ID_RADIER,WALL_ID_CIEL, WALL_ID_LEFTWALL, WALL_ID_RIGHTWALL, WALL_ID_FDTFRONT, WALL_ID_FDTBACK, NB_WALL_ID };
static WALL_ID m_correspondingWall[NB_WALL_ID]={WALL_ID_CIEL,WALL_ID_RADIER, WALL_ID_RIGHTWALL, WALL_ID_LEFTWALL, WALL_ID_FDTBACK, WALL_ID_FDTFRONT};
enum LOD {LOD_LOWEST, LOD_LOW,  LOD_JOINT_LOWHIGH, LOD_HIGH};

//--- perlin noise domified ---
#define NBHARMONIC 32
#define BB 0x100
#define BM 0xff
#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff
#define s_curve(t) ( t * t * (3.f - 2.f * t) )
#define lerp(t, a, b) ( a + t * (b - a) )
#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.f;

class MineWall : public scene::ISceneNode
{
public:
	SMesh* m_mesh;
	core::aabbox3d<f32> Box;	// chose relativement inutile, sauf peut etre pour en déterminer la visibilité?
	LOD m_LOD;	// current LOD

	MineWall::MineWall(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, MineProfil * profil, MineBloc * pMineBloc, WALL_ID wallId);
	~MineWall(void);

	MineProfil * getProfil();
	void setProfil(MineProfil * pProfil);
	MineBloc * MineWall::getMineBloc(void);
	WALL_ID MineWall::getWallId();
	bool MineWall::intersects(line3d<f32>* pLine, vector3df * pIntersectionPoint=NULL, bool onlyActiveWalls=true);

	// irrlicht iSceneNode overloads
	SMesh * MineWall::getMesh();
	virtual video::SMaterial& getMaterial(s32 i);
	virtual s32 getMaterialCount();
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual void render();
	virtual void OnPreRender();
	virtual void remove();
	bool isActive();

protected:
	SMeshBuffer* m_buffer;
	MineBloc * m_pMineBloc;
	MineProfil m_profil;
	WALL_ID m_wallId;
	triangle3df m_shapeTriangle[2];
	bool	m_bActiveShape;		// est ce que le wall existe / est cliquable?
	u16 m_lowResMeshIndices[6];
	S3DVertex m_lowResMeshVertices[4];

	unsigned int m_randSeed;
	int start;
	int p[BB + BB + 2];
	float g3[BB + BB + 2][3];
	float g2[BB + BB + 2][2];
	float g1[BB + BB + 2];

	void MineWall::computeWallGeometry();
	double MineWall::noise1(double arg);
	float MineWall::noise2(float vec[2]);
	float MineWall::noise3(float vec[3]);
	void MineWall::normalize2(float v[2]);
	void MineWall::normalize3(float v[3]);
	void MineWall::init(void);
	void MineWall::mySRand(unsigned int seed);
	int MineWall::myRand();
};