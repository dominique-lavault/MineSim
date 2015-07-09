#pragma once

#include <irrlicht.h>
#include "terrain.h"
#include "itineraire.h"
#include "../MineProfil.h"

class CLandSceneNode;
class CEtiquette;

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#define NB_BLK 12	// nombre de block sur un coté de l'area visible
#define NB_MAX_ETIQUETTES 20
#define CENTEROFTHEWORLD_X  0.0f
#define CENTEROFTHEWORLD_Z 0.0f 

#define MINIMUM_DISTANCE_FOR_SETALTITUDE 150.0	// distance de sécurité pour modifier le terrain (pour ne pas le faire trop près de structure liées au terrain)
#define DISTANCE2D(x1,y1,x2,y2) sqrt( ((x2)-(x1))*((x2)-(x1)) + ((y2)-(y1))*((y2)-(y1)) )

enum MIPMAP {MIPMAP_NOTDIRTY, MIPMAP_DIRTY, MIPMAP_DIRTYNEAR }; 

struct enreg{
	float lat,lon;
	unsigned int label;
};

struct mipmapLevelManager
{
	int iLevel;
	int  iDirty;
};

class CLandManager
{
public:
	CLandManager::CLandManager(scene::ISceneNode* parent=NULL, scene::ISceneManager* smgr=NULL,video::IVideoDriver* driver=NULL);
	CLandManager::~CLandManager();
	void CLandManager::updateLand();
	f32 CLandManager::getAltitude(f32 x, f32 z);
	short int CLandManager::getAltitude(int x, int z);
	int CLandManager::getMipMapLevel(vector3df * cameraPosition,f32 X, f32 Z);
	const char * CLandManager::getPlaceName(int i);
	int CLandManager::getNbPlace();
	coordWGS84 CLandManager::getPlacePosition(int i);

	CEtiquette * m_pEtiquette[NB_MAX_ETIQUETTES];
	ISceneNode*m_3DSceneNode[NB_MAX_ETIQUETTES];
	int CLandManager::getNbEtiquette();
	
	// gestion des patches & layers delta d'altitude
	void CLandManager::setPatch(vector3df * zonePatch);
	int CLandManager::getBlocIndexByDirection(int baseIndex, int dirX, int dirY);
	bool CLandManager::patchLand();
	int CLandManager::getIndexSceneNodeByCoords(vector3df * m_zone);
	bool setDeltaAltitude(vector3df * position, float deltaAltitude);
	f32 CLandManager::getAltitudeWithDelta(f32 x, f32 z);
	bool CLandManager::IsItPossibleToCreateGalerieJonction(vector3df * pos);
	MineProfil CLandManager::createGalerieJonction(float x, float z, vector3df * axeMine, bool bTerrassement=true);
	void CLandManager::placeMineEntry( vector3df * pos, vector3df * axeMine);

	CTerrain m_terrain;

	int m_baseU, m_baseV;	// offset virtuel dans la table des blocks

protected:
	int m_nbLandSceneNodes;
	vector3df oldCameraPosition;
	CItineraire * m_itineraire;
	IAnimatedMesh*  m_churchMesh, *m_chemineeMesh, *m_fanionMesh; 
	ITexture * m_churchTexture, *m_chemineeTexture, *m_fanionTexture;

	// gestion des patches
	array<vector3df> m_zonePatchArray;

	// structures de données pour les lieux
	int m_nameDataBaseSize;
	char * m_nameDataBase;
	int m_placeDataBaseSize;
	enreg * m_placeDataBase;

	CLandSceneNode ** m_node;		// table de pointeurs vers les CLandSceneNode gérés
	mipmapLevelManager * m_mipMapArray;
	scene::ISceneNode * m_waterNode;

	int CLandManager::setEtiquette(f32 xEtiq, f32 yEtiq, wchar_t * label, char type);
	int CLandManager::getNodeIndex(int u,int v);
};