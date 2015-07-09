#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include <irrlicht.h>

#include "landManager.h"
#include "landSceneNode.h"
#include "etiquette.h"
#include "terrain.h"
#include "../GlobalData.h"

//#include "realTerrain.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

extern  IrrlichtDevice * device;
extern globalData g_data;

CLandManager::CLandManager(scene::ISceneNode* parent,scene::ISceneManager* smgr,video::IVideoDriver* driver)
{
	vector3df offset; //(-NBS/2*XZSCALE,0.0,-NBS/2*XZSCALE);

	m_itineraire=NULL;
	m_nameDataBase=NULL;
	m_placeDataBase=NULL;

	m_node= new CLandSceneNode *[NB_BLK*NB_BLK];
	memset(m_node,0,NB_BLK*NB_BLK*sizeof(CLandSceneNode*));
	m_nbLandSceneNodes=0; 
	m_mipMapArray = new mipmapLevelManager[NB_BLK*NB_BLK];
	oldCameraPosition = vector3df(0.0,0.0,0.0);

	// génère le terrain d'origine
	int i;

	scene::IAnimatedMesh* mesh;
	SMaterial waterMat;
	waterMat.Wireframe=false;
	waterMat.BackfaceCulling=false;
	waterMat.FogEnable=true;
	waterMat.Shininess=20.0f;
	waterMat.SpecularColor=SColor(255,255,255,255);
	waterMat.Lighting=true;
	mesh = smgr->addHillPlaneMesh("myHill",
		core::dimension2d<f32>(20,20),
		core::dimension2d<s32>(20,20), &waterMat, 0,
		core::dimension2d<f32>(10,10),
		core::dimension2d<f32>(200,200));
	m_waterNode= smgr->addWaterSurfaceSceneNode(mesh->getMesh(0), 0.5f, 400.0f, 0.01f);
	m_waterNode->setPosition(core::vector3df(0,0,0));
	m_waterNode->setScale(core::vector3df(10.0,1.0,10.0));
	m_waterNode->setMaterialTexture(0,	driver->getTexture("media/up.jpg"));
	m_waterNode->setMaterialTexture(1,	driver->getTexture("media/water.jpg"));
	m_waterNode->setMaterialType(video::EMT_TRANSPARENT_REFLECTION_2_LAYER);	// EMT_REFLECTION_2_LAYER
	m_waterNode->setMaterialFlag(video::EMF_LIGHTING, false);
	m_waterNode->setMaterialFlag(EMF_FOG_ENABLE, true); 

	vector3df centerOfTheWorld(CENTEROFTHEWORLD_X, 0.0f, CENTEROFTHEWORLD_Z);
	m_baseU=(int)(CENTEROFTHEWORLD_X/XZSCALE/(NBS-1)-NB_BLK/2)+1;
	m_baseV=int(CENTEROFTHEWORLD_Z/XZSCALE/(NBS-1)-NB_BLK/2)+1;
	
	m_baseU=((int)(m_baseU/NB_BLK))*NB_BLK+NB_BLK;
	m_baseV=((int)(m_baseV/NB_BLK))*NB_BLK+NB_BLK;

	// load les éléments 3D à poser dans notre world
	m_chemineeMesh = smgr->getMesh("media/cheminee.obj");
	m_chemineeTexture = driver->getTexture("media/cheminee.jpg");
	m_churchMesh = smgr->getMesh("media/church.obj"); 
	m_churchTexture = driver->getTexture("media/church.jpg");
	m_fanionMesh = smgr->getMesh("media/fanion.obj");
	m_fanionTexture = driver->getTexture("media/fanion.jpg");

	// on créé des étiquette "vides"
	for(i =0; i<NB_MAX_ETIQUETTES; i++)
	{
		m_pEtiquette[i] = new CEtiquette(parent, smgr, 667);
		m_pEtiquette[i]->setMaterialFlag(EMF_BACK_FACE_CULLING, false); 
		m_pEtiquette[i]->setMaterialFlag(EMF_WIREFRAME, true);
		m_pEtiquette[i]->setVisible(false);
		m_pEtiquette[i]->m_label=L"<unused>";
		m_pEtiquette[i]->drop();

		m_3DSceneNode[i] = NULL;
	}
	
	// on crée les blocs initiaux autour de la caméra
	irr::video::ITexture *terrainTexture = driver->getTexture("media/textland.jpg");
	for(int u=0; u<NB_BLK; u++)
		for(int v=0; v<NB_BLK; v++)
		{
			i = getNodeIndex(u,v);
			offset.X = (f32)((float)m_baseU+u-NB_BLK/2.0) * XZSCALE * (NBS-1);
			offset.Z = (f32)((float)m_baseV+v-NB_BLK/2.0) * XZSCALE * (NBS-1);
			// constuire le terrain à un offset donné 
			m_node[i] = new CLandSceneNode(parent, smgr, 666);
			m_node[i]->setOffset(&offset);
			m_node[i] ->generateMesh(this,&centerOfTheWorld);
			m_nbLandSceneNodes++;
			//application des paramètres de rendu à chaque objet m_node[u][v]
			m_node[i]->setMaterialTexture(0, terrainTexture);
			m_node[i]->setMaterialFlag(EMF_FOG_ENABLE,true); 
			//m_node[i]->setMaterialFlag(EMF_WIREFRAME, true);
			m_node[i]->setMaterialFlag(EMF_BACK_FACE_CULLING, true); 
			m_node[i]->drop();
		}

	// chargement des noms des lieux
	m_nameDataBaseSize=0;
	m_nameDataBase=0;
	/*
	FILE * fp;
	if( (fp= fopen("data\\basetext.ciel","rb"))!= NULL)
	{
		fseek(fp,0,SEEK_END);
		m_nameDataBaseSize=ftell(fp)+1;
		fseek(fp,0,SEEK_SET);
		m_nameDataBase = (char *) malloc(m_nameDataBaseSize);

		if( m_nameDataBase != NULL)
			fread(m_nameDataBase, 1, m_nameDataBaseSize, fp);
		fclose(fp);
	}

	if( (fp= fopen("data\\base.ciel","rb"))!= NULL)
	{
		fseek(fp,0,SEEK_END);
		m_placeDataBaseSize=ftell(fp)+1;
		fseek(fp,0,SEEK_SET);
		m_placeDataBase = (enreg*) malloc(m_placeDataBaseSize);

		if(m_placeDataBase != NULL)
		{
			fread(m_placeDataBase, 1, m_placeDataBaseSize, fp);
			fclose(fp);
		}
	}
	*/
	m_itineraire = new CItineraire(parent, smgr, 668);
	m_itineraire->init(&m_terrain);
	m_itineraire->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA );
	m_itineraire->setMaterialFlag(EMF_BACK_FACE_CULLING, false);

	// on met de la végétation dans notre world
	// billboard stuff
	/*
	f32 xBillB,yBillB;
	scene::ISceneNode* node[100];
	video::ITexture* plant = driver->getTexture("media/arbre.psd");
	driver->makeColorKeyTexture(plant, core::position2d<s32>(0,0));
	for(i=0; i<100; i++)
	{
		node[i]=0;
		node[i] = smgr->addBillboardSceneNode(node[i], core::dimension2d<f32>(10.0f, 20.0f));
		node[i]->setMaterialFlag(video::EMF_LIGHTING, false);
		xBillB = (rand()%(int)(NB_BLK*XZSCALE*NBS))-NB_BLK*XZSCALE*NBS/2;
		yBillB = (rand()%(int)(NB_BLK*XZSCALE*NBS))-NB_BLK*XZSCALE*NBS/2;
		node[i]->setMaterialTexture(0,	plant);
		node[i]->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL); //EMT_TRANSPARENT_VERTEX_ALPHA
		node[i]->setPosition(core::vector3df(xBillB, getAltitude(xBillB, yBillB) + 5.f, yBillB));
	}
	*/
}

CLandManager::~CLandManager()
{
	// les m_node[i] sont des iscenenode, il seront deletés par irrlicht
	delete m_node;
	delete m_mipMapArray;
	if( m_itineraire )
		delete m_itineraire;
	if( m_placeDataBase)
		free(m_placeDataBase);
	if( m_nameDataBase )
		free(m_nameDataBase);
}

// créée une étiquette, retourne -1 si toutes les étiquettes sont utilisées
int CLandManager::setEtiquette(f32 xEtiq, f32 zEtiq, wchar_t* label, char type)
{
	int noEtiquette;
	int rc=0;
	wchar_t signs[38]=L"AABCDEEEFGHIIJKLMNOOOPQRSTUUUVWXYZ -'";
	
	//recherche un slot libre
	for(noEtiquette=0; noEtiquette<NB_MAX_ETIQUETTES; noEtiquette++)
	{
		if( m_pEtiquette[noEtiquette]->getStatus()==CEtiquette::OFF)
			break;
	}

	scene::ISceneManager* smgr = device->getSceneManager();
	if( noEtiquette<NB_MAX_ETIQUETTES)
	{
		f32 yEtiq = getAltitude(xEtiq,zEtiq);
		m_pEtiquette[noEtiquette]->setOffset(&vector3df(xEtiq, yEtiq+110.0f, zEtiq));
		if( label==NULL )
		{
			// on lui donne un nom bidon
			m_pEtiquette[noEtiquette]->m_label=L"";
			for(int r=rand()%12, kl=0; kl<r; kl++)
			{
				m_pEtiquette[noEtiquette]->m_label.append(signs[rand()%37]);
			}
		}
		else
			m_pEtiquette[noEtiquette]->m_label = label;
		
		coordWGS84 coords = m_terrain.getCoordsFromXY(xEtiq/XZSCALE, zEtiq/XZSCALE);
		m_pEtiquette[noEtiquette]->m_type = type; /*
												  Feature Classification:
													A = Administrative region;
													P = Populated place;
													V = Vegetation;
													L = Locality or area;
													U = Undersea;
													R = Streets, highways, roads, or railroad;
													T = Hypsographic;
													H = Hydrographic;
													S = Spot feature.
												  */
		m_pEtiquette[noEtiquette]->m_lat = coords.lat;
		m_pEtiquette[noEtiquette]->m_lon = coords.lon;
		m_pEtiquette[noEtiquette]->switchOn();

		if( type=='P' || type=='S' || type=='T')
		{
			if(m_3DSceneNode[noEtiquette]!=NULL)
				m_3DSceneNode[noEtiquette]->remove();

			if( type=='P' )
			{
				m_3DSceneNode[noEtiquette] = smgr->addMeshSceneNode( m_churchMesh->getMesh(0) ); 
				m_3DSceneNode[noEtiquette]->setMaterialTexture(0, m_churchTexture);
				m_3DSceneNode[noEtiquette]->setRotation(vector3df(0.0f, -90.0f, 0.0f));	// nef tournée vers l'est! :)
			}
			else  if( type=='S' )
			{
				m_3DSceneNode[noEtiquette] = smgr->addMeshSceneNode( m_chemineeMesh->getMesh(0) ); 
				m_3DSceneNode[noEtiquette]->setMaterialTexture(0, m_chemineeTexture);
			}
			else //if( type=='T' )
			{
				m_3DSceneNode[noEtiquette] = smgr->addMeshSceneNode( m_fanionMesh->getMesh(0) ); 
				m_3DSceneNode[noEtiquette]->setMaterialTexture(0, m_fanionTexture);
			}
			m_3DSceneNode[noEtiquette]->setPosition(vector3df(xEtiq, yEtiq, zEtiq));
			m_3DSceneNode[noEtiquette]->setMaterialFlag(EMF_LIGHTING, false); 
			m_3DSceneNode[noEtiquette]->setMaterialType(video::EMT_SOLID);
			m_3DSceneNode[noEtiquette]->setMaterialFlag(EMF_FOG_ENABLE, true); 
			m_3DSceneNode[noEtiquette]->setVisible(true);
			m_3DSceneNode[noEtiquette]->setScale(vector3df(4.0f, 4.0f, 4.0f));
		}
	}
	else
		rc=-1;
	return rc;
}

int CLandManager::getNodeIndex(int u,int v)
{
	return ((u+2048*2048)%NB_BLK) * NB_BLK + ((v + 2048*2048)%NB_BLK);
}

f32 CLandManager::getAltitude(f32 x, f32 z)
{
	float vec[2];
	vec[0]=(x/XZSCALE+NBS/2)/PERLINSCALE;
	vec[1]=(z/XZSCALE+NBS/2)/PERLINSCALE;
	return (m_terrain.getAltitude(vec))*YSCALE;	
}

f32 CLandManager::getAltitudeWithDelta(f32 x, f32 z)
{
	f32 alt = getAltitude(x,z);
	// 1 - retrouver le delta local
	// 1.1 - retrouver le bloc concerné
	int idx = getIndexSceneNodeByCoords(&vector3df(x,0,z));
	// 1.2 - si y'en a un retrouver le delta à l'emplacement concerné
 	if( idx!=-1)
	{
		int col=(int)((x-m_node[idx]->m_offset.X)/XZSCALE);
		int row=(int)((z-m_node[idx]->m_offset.Z)/XZSCALE);
		alt+=m_node[idx]->getDeltaAltitude(col+row*NBS);
	}
	return alt;
}

short int CLandManager::getAltitude(int x, int z)
{
	x=(int)(x/XZSCALE+NBS/2);
	z=(int)(z/XZSCALE+NBS/2);
	return (short int)((float)(m_terrain.getAltitude((float)x,(float)z)*YSCALE));
}

// rend le mipmaplevel à utiliser en fonction de la caméra et de l'objet considéré (x,z)
int CLandManager::getMipMapLevel(vector3df * cameraPosition,f32 X, f32 Z)
{
	double distance = DISTANCE2D(cameraPosition->X,cameraPosition->Z, X, Z);
	if( distance<NBS * XZSCALE*2)
		return 0;
	else if( distance<NBS * XZSCALE *3 )
		return 1;
	else  if( distance<NBS * XZSCALE *4 )
		return 2;
	else
		return 3;
}

void CLandManager::updateLand()
{
	vector3df * cameraPosition = &g_data.m_camera->m_camPos;
	IVideoDriver* driver=g_data.driver;
	int idx, camIdx;
	int tmpU, tmpV;
	f32 minX,minZ,maxX,maxZ;

	if( *cameraPosition == oldCameraPosition)
		return;
	oldCameraPosition=*cameraPosition;

	// ======== update le niveau de mipmap des blocks en fonction de la distance à la caméra ==========
	int i, newMipmapLevel;

	// on parcourt le tableau
	for(int u=0; u<NB_BLK; u++)
	{
		for(int v=0; v<NB_BLK; v++)
		{
			i = getNodeIndex(u,v);
			m_mipMapArray[u+v*NB_BLK].iDirty=MIPMAP_NOTDIRTY;
		}
	}

	//  --- quels sont les blocs dont le mipmap level change?
	for(int u=0; u<NB_BLK; u++)
	{
		for(int v=0; v<NB_BLK; v++)
		{
			//position centre du bloc
			i = getNodeIndex(u,v);
			minX = m_node[i]->m_offset.X +NBS * XZSCALE /2;
			minZ = m_node[i]->m_offset.Z +NBS * XZSCALE /2;
			newMipmapLevel = getMipMapLevel(cameraPosition, minX, minZ);

			if( m_node[i]->m_mipmapLevel != newMipmapLevel)
			{
				m_node[i]->m_mipmapLevel=newMipmapLevel;
				m_mipMapArray[u+v*NB_BLK].iDirty=MIPMAP_DIRTY;
				
				// optimisation possible : c'est pas la peine de mettre à jour MIPMAP_DIRTYNEAR à travers m_baseU/m_baseV
				if( u>0)
					m_mipMapArray[u-1+v*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;
				else 
					m_mipMapArray[u+NB_BLK-1+v*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;

				if( v>0)
					m_mipMapArray[u+(v-1)*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;
				else
					m_mipMapArray[u+(NB_BLK-1)*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;

				if( u<NB_BLK-1 )	
					m_mipMapArray[u+1+v*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;
				else
					m_mipMapArray[0+v*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;

				if( v<NB_BLK-1 )	
					m_mipMapArray[u+(v+1)*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;
				else
					m_mipMapArray[u+0*NB_BLK].iDirty=MIPMAP_DIRTYNEAR;
			}
		}
	}

	// --- on update ceux qui le nécessitent
	int iN, iS, iO, iE;
	int mml_N,mml_S,mml_O,mml_E;
	for(int u=0; u<NB_BLK; u++)
	{
		for(int v=0; v<NB_BLK; v++)
		{
			if( m_mipMapArray[u+v*NB_BLK].iDirty!=MIPMAP_NOTDIRTY)
			{
				i = getNodeIndex(u,v);
				iN = getNodeIndex(u,v-1);
				iS = getNodeIndex(u,v+1);
				iO = getNodeIndex(u-1,v);
				iE = getNodeIndex(u+1,v);
				mml_N = m_node[iN]->m_mipmapLevel;
				mml_S = m_node[iS]->m_mipmapLevel;
				mml_O = m_node[iO]->m_mipmapLevel;
				mml_E = m_node[iE]->m_mipmapLevel;
				m_node[i]->updateTesslation(this, cameraPosition, mml_N, mml_S, mml_O, mml_E	);
			}
		}
	}
	
	//========= génére les nouveaux blocs de relief si besoin ==================
	bool worldChanged=false;
	camIdx = getNodeIndex((m_baseU+(NB_BLK-1)/2), (m_baseV+(NB_BLK-1)/2));
	minX = m_node[camIdx]->m_offset.X;
	minZ = m_node[camIdx]->m_offset.Z;
	maxX=minX+NBS * XZSCALE;
	maxZ=minZ+NBS * XZSCALE;
	vector3df offset;
	offset.Y=0;
	f32 newBlocksMinX;
	f32 newBlocksMinZ;
	f32 newBlocksMaxX;
	f32 newBlocksMaxZ;

	if( cameraPosition->X < minX || cameraPosition->X > maxX )
	{
		if( cameraPosition->X < minX )
		{
			tmpU=m_baseU-1;
			m_baseU--;
		}
		else //if( cameraPosition.X > maxX )
		{
			tmpU=m_baseU+NB_BLK;
			m_baseU++;
		}

		offset.X = ((float)tmpU-NB_BLK/2.0f) * XZSCALE * (NBS-1);
		newBlocksMinX = offset.X;
		newBlocksMaxX = newBlocksMinX + NBS * XZSCALE;
		newBlocksMinZ =((float)m_baseV-NB_BLK/2.0f) * XZSCALE * (NBS-1);
		newBlocksMaxZ = newBlocksMinZ + NB_BLK * NBS * XZSCALE;

		for(tmpV=m_baseV; tmpV<m_baseV+NB_BLK; tmpV++)
		{
			idx = getNodeIndex(tmpU%NB_BLK, tmpV%NB_BLK);
			offset.Z = ((float)tmpV-NB_BLK/2.0f) * XZSCALE * (NBS-1);
			m_node[idx]->setOffset(&offset);
			m_node[idx]->generateMesh(this, cameraPosition);

			worldChanged=true;
		}

		idx = getNodeIndex(m_baseU+NB_BLK/2, m_baseV+NB_BLK/2);
		if( m_waterNode)
			m_waterNode->setPosition(core::vector3df(m_node[idx]->m_offset.X,20,m_node[idx]->m_offset.Z));

	}

	if( cameraPosition->Z < minZ || cameraPosition->Z > maxZ )
	{
		if( cameraPosition->Z < minZ )
		{
			tmpV=m_baseV-1;
			m_baseV--;
		}
		else // if( cameraPosition.Z > maxZ )
		{
			tmpV=m_baseV+NB_BLK;
			m_baseV++;
		}

		offset.Z = ((float)tmpV-NB_BLK/2.0f) * XZSCALE * (NBS-1);
		newBlocksMinX =((float)m_baseU-NB_BLK/2.0f) * XZSCALE * (NBS-1);
		newBlocksMaxX = newBlocksMinX + NB_BLK * NBS * XZSCALE;
		newBlocksMinZ = offset.Z;
		newBlocksMaxZ = newBlocksMinZ + NBS * XZSCALE;

		for(tmpU=m_baseU; tmpU<m_baseU+NB_BLK; tmpU++)
		{
			idx = getNodeIndex(tmpU%NB_BLK, tmpV%NB_BLK);
			offset.X = ((float)tmpU-NB_BLK/2.0f) * XZSCALE * (NBS-1);
			m_node[idx]->setOffset(&offset);
			m_node[idx]->generateMesh(this, cameraPosition);

			worldChanged=true;
		}

		idx = getNodeIndex(m_baseU+NB_BLK/2, m_baseV+NB_BLK/2);
		if( m_waterNode )
			m_waterNode->setPosition(core::vector3df(m_node[idx]->m_offset.X,20,m_node[idx]->m_offset.Z));
	}

	//si il y a eu du changement
	if(	worldChanged)
	{
		// il faut updater les étiquetes qui ont pu quitter notre monde (amen)
		vector3df position;
		int minIdx = getNodeIndex(m_baseU, m_baseV);
		f32 worldMinX = m_node[minIdx]->m_offset.X;
		f32 worldMinZ = m_node[minIdx]->m_offset.Z;
		f32 worldMaxX = worldMinX + NB_BLK * NBS * XZSCALE;
		f32 worldMaxZ = worldMinZ + NB_BLK * NBS * XZSCALE;

		for(i=0; i<NB_MAX_ETIQUETTES; i++)
		{
			m_pEtiquette[i]->switchOff();
			if(m_3DSceneNode[i] != NULL)
				m_3DSceneNode[i]->setVisible(false);
		}

		// et rajouter les nouvelles étiquettes! 
		Point p;
		wchar_t str[128];
		char * ptr;
		char * charptr;
		for(i=0; i<m_placeDataBaseSize/12; i++)
		{
			p=m_terrain.getXYFromWGS84(m_placeDataBase[i].lon, m_placeDataBase[i].lat);
			p.x*=(int)XZSCALE;
			p.y*=(int)XZSCALE;
			if( p.y>worldMinZ && p.y<worldMaxZ && p.x>worldMinX && p.x<worldMaxX)
			{
				ptr = (char*)(m_nameDataBase+m_placeDataBase[i].label+1);
				int l;
				for(l=0; ptr[l]!=0 && l<64; l++)
					str[l] = ptr[l];
				str[l]=0;
				 /*correction*/
				charptr = ptr-1;
				setEtiquette(p.x-8.0f*NBS, p.y+3.0f*NBS, str, *charptr);
			}
		}

		// le terrain a changé, il faut potentiellement refaire les patches
		patchLand();
	}

}

//----------------------------------------------------------------------------------------------------------------------
//------------------------------------------ gestion des patches -----------------------------------------------------
// renvoie l'index dans la table des LandSceneNode ** de celui contenant le point passé en paramètre
int CLandManager::getIndexSceneNodeByCoords(vector3df * m_zone)
{
	int idx;
	// retrouver la grille contenant les coords demandées
	int minIdx = getNodeIndex(m_baseU, m_baseV);
	f32 worldMinX = m_node[minIdx]->m_offset.X;
	f32 worldMinZ = m_node[minIdx]->m_offset.Z;
	f32 worldMaxX = worldMinX + NB_BLK * NBS * XZSCALE;
	f32 worldMaxZ = worldMinZ + NB_BLK * NBS * XZSCALE;
	if( worldMinX<m_zone->X && worldMaxX>m_zone->X 
		&& worldMinZ<m_zone->Z	&& worldMaxZ>m_zone->Z )
	{
		for(int u=0; u<NB_BLK; u++)
		{
			for(int v=0; v<NB_BLK; v++)
			{
				idx = getNodeIndex(u,v);
				float offx = m_node[idx]->m_offset.X;
				float offz = m_node[idx]->m_offset.Z;
				if( offx<=m_zone->X && offx +(NBS-1) * XZSCALE >m_zone->X 
					&& offz<=m_zone->Z && offz +(NBS-1) * XZSCALE>m_zone->Z )
				{
					return idx;
				}
			}
		}
	}
	return -1;
}

// renvoie l'index du bloc à partir d'un bloc, dans une direction donnée (inférieure à NB_BLK)
int CLandManager::getBlocIndexByDirection(int baseIndex, int dirX, int dirY)
{
	int u,v;
	v=baseIndex/NB_BLK;
	u=baseIndex%NB_BLK;
	u+=dirX;
	v+=dirY;
	// n'a jamais été testé : 
	// se produit lorsqu'on essaye de modifier un bloc en warpant à travers les boundaries du tableau des landscenenodes*
	if( v<0 )
		v=NB_BLK-1;
	if( v>=NB_BLK )
		v=0;
	if( u<0 )
		u=NB_BLK-1;
	if( u>=NB_BLK )
		u=0;

	return u+v*NB_BLK;
}

// -------- modifie l'altitude d'un point -----------------------------------------------------------------------------------------------------
bool CLandManager::setDeltaAltitude(vector3df * position, float deltaAltitude)
{
	int idx=getIndexSceneNodeByCoords(position);
	// on teste qu'il n'y a pas de patch à coté
	int n = m_zonePatchArray.size();
	for (int i=0; i<n ;i++)
	{
		vector3df pos(*position);
		pos.Y=0;
		double dist = m_zonePatchArray[i].getDistanceFrom(pos);
		if( dist<MINIMUM_DISTANCE_FOR_SETALTITUDE)
			return false;		// refusé
	}
	if( idx!=-1)
	{
		// ----- retouver l'index des du vertice impacté -------
		int col = (int)((position->X - m_node[idx]->m_offset.X) / XZSCALE);
		int row = (int)((position->Z - m_node[idx]->m_offset.Z) / XZSCALE);
		int altidx=col+row*NBS;
		m_node[idx]->setDeltaAltitude(altidx, m_node[idx]->getDeltaAltitude(altidx)+deltaAltitude);
		m_node[idx]->generateMesh(this,position);	// regénère le mesh avec la modif

		// ------- test impact sur les blocs d'à coté
		int foreignIdx=-1;
		int foreignDeltaIndex=-1;
		if( row==0 )
		{
			foreignIdx=getBlocIndexByDirection(idx, -1, 0);
			foreignDeltaIndex=col+(NBS-1)*NBS;
		}
		if( row==NBS-1 )
		{
			foreignIdx=getBlocIndexByDirection(idx, 1, 0);
			foreignDeltaIndex=col;
		}
		if( foreignIdx!=-1)
		{
			m_node[foreignIdx]->setDeltaAltitude(foreignDeltaIndex, m_node[foreignIdx]->getDeltaAltitude(foreignDeltaIndex)+deltaAltitude);
			m_node[foreignIdx]->generateMesh(this,position);	// regénère le mesh avec la modif
			foreignIdx=-1;
		}

		if( col==0 )
		{
			foreignIdx=getBlocIndexByDirection(idx, 0, -1);
			foreignDeltaIndex=(NBS-1)+row*NBS;
		}
		if( col==NBS-1 )
		{
			foreignIdx=getBlocIndexByDirection(idx, 0, 1);
			foreignDeltaIndex=row*NBS;
		}
		if( foreignIdx!=-1)
		{
			m_node[foreignIdx]->setDeltaAltitude(foreignDeltaIndex, m_node[foreignIdx]->getDeltaAltitude(foreignDeltaIndex)+deltaAltitude);
			m_node[foreignIdx]->generateMesh(this,position);	// regénère le mesh avec la modif
			foreignIdx=-1;
		}
		// ---- traitement de bloc supplémentaires à impacter (dans les angles)
		if( row==0)
		{
			if( col==0 )
			{
				foreignIdx=getBlocIndexByDirection(idx, -1, -1);
				foreignDeltaIndex=NBS*NBS-1;	//= (NBS-1)+(NBS-1)*NBS;
			}
			if( col==NBS-1 )
			{
				foreignIdx=getBlocIndexByDirection(idx, -1, +1);
				foreignDeltaIndex= 0 + (NBS-1)*NBS;
			}
		}
		if( row==NBS-1)
		{
			if( col==0 )
			{
				foreignIdx=getBlocIndexByDirection(idx, +1, -1);
				foreignDeltaIndex=NBS-1;
			}
			if( col==NBS-1 )
			{
				foreignIdx=getBlocIndexByDirection(idx, +1, +1);
				foreignDeltaIndex= 0;
			}
		}
		if( foreignIdx!=-1)
		{
			m_node[foreignIdx]->setDeltaAltitude(foreignDeltaIndex, m_node[foreignIdx]->getDeltaAltitude(foreignDeltaIndex)+deltaAltitude);
			m_node[foreignIdx]->generateMesh(this,position);	// regénère le mesh avec la modif
		}
	}
	return true;
}

bool CLandManager::patchLand()
{
	int idx;
	bool patched=false;
	// -------------------------------- patche le terrain ainsi généré -------------------------------------
	// ------ enlève les triangles des zones creusées
	int nbPatches=m_zonePatchArray.size();
	if( nbPatches>0 )
	{
		for(int patchIdx=0; patchIdx<nbPatches; patchIdx++)
		{
			idx=getIndexSceneNodeByCoords(&m_zonePatchArray[patchIdx]);
			if( idx != -1 )
			{
				// retouver l'index des indices du triangle
				int col = (int)((m_zonePatchArray[patchIdx].X - m_node[idx]->m_offset.X) / XZSCALE);
				int row = (int)((m_zonePatchArray[patchIdx].Z - m_node[idx]->m_offset.Z) / XZSCALE);
				int idxIndices = (row + col*(NBS-1))*6;

				if( idxIndices > NBBLOCKFACES*3-6 || idxIndices <0 )
				{
					idxIndices =idxIndices ;
				}
				else
				{
					// --- patch it
					// on retire les 2 triangles
					m_node[idx]->m_indices[idxIndices]=0;
					m_node[idx]->m_indices[idxIndices+1]=0;
					m_node[idx]->m_indices[idxIndices+2]=0;
					m_node[idx]->m_indices[idxIndices+3]=0;
					m_node[idx]->m_indices[idxIndices+4]=0;
					m_node[idx]->m_indices[idxIndices+5]=0;
					patched=true;
				}
			}
		}
	}
	return patched; 
}


bool CLandManager::IsItPossibleToCreateGalerieJonction(vector3df * pos)
{
	pos->Y=0;
	// on teste si on est pas trop près d'une autre entrée
	int n = m_zonePatchArray.size();
	for (int i=0; i<n ;i++)
	{
		double dist = m_zonePatchArray[i].getDistanceFrom(*pos);
		if( dist<MINIMUM_DISTANCE_FOR_SETALTITUDE)
			return false;
	}
	return true;
}

// modifie le terrain au point demandé pour le faire jointer avec une galerie sortant à cet endroit
// renvoie le profil du trou dans le sol ainsi créé
MineProfil CLandManager::createGalerieJonction(float x, float z, vector3df * axeMine, bool bTerrassement)
{
	vector3df axeMineNormed=*axeMine;
	axeMineNormed.Y=0;
	if( abs(axeMineNormed.X)>abs(axeMineNormed.Z))
		axeMineNormed.Z=0;
	else
		axeMineNormed.X=0;
	axeMineNormed.normalize();

	x=floor(x/XZSCALE)*XZSCALE;
	z=floor(z/XZSCALE)*XZSCALE;

	// création du profil de l'entrée :
	vector3df c1(x,					getAltitudeWithDelta(x,z),							z);
	vector3df c2(x+XZSCALE,	getAltitudeWithDelta(x+XZSCALE,z),				z);
	vector3df c3(x+XZSCALE,	getAltitudeWithDelta(x+XZSCALE,z+XZSCALE),	z+XZSCALE);
	vector3df c4(x,					getAltitudeWithDelta(x,z+XZSCALE),				z+XZSCALE);

	// on matche les points en surface au plus près des points du profil à rendre
	// todo : apparement des fois ça buggue copieusement
	vector3df s1,s2,s3,s4;
	if( axeMineNormed.Z<0)
	{
		s1=c1;
		s2=c2;
		s3=c3;
		s4=c4;
	}
	else	if( axeMineNormed.Z>0)
	{
		s1=c3;
		s2=c4;
		s3=c1;
		s4=c2;
	}
	else	if( axeMineNormed.X>0)
	{
		s1=c2;
		s2=c3;
		s3=c4;
		s4=c1;
	}
	else	if( axeMineNormed.X<0)
	{
		s1=c4;
		s2=c1;
		s3=c2;
		s4=c3;
	}

	// on nivelle le ciel et le radier, on relève le linteau de 40 unités
	if( bTerrassement )
	{
		setDeltaAltitude(&vector3df(s2.X,0,s2.Z), -s2.Y+s1.Y);
		s2.Y=s1.Y;
		setDeltaAltitude(&vector3df(s4.X,0,s4.Z), -s4.Y+s1.Y+40.0f);
		s4.Y=s1.Y+40.0f;
		setDeltaAltitude(&vector3df(s3.X,0,s3.Z), -s3.Y+s4.Y);
		s3.Y=s4.Y;

		// crée une butte pour de l'entrée de mine + patche le trou de l'entrée
		for(int fond=1; fond<4; fond++)
		{
			setDeltaAltitude(&vector3df(s4.X-axeMineNormed.X*XZSCALE*fond, 0, s4.Z-axeMineNormed.Z*XZSCALE*fond), +(5.0f-fond)*10.0f);
			setDeltaAltitude(&vector3df(s3.X-axeMineNormed.X*XZSCALE*fond, 0, s3.Z-axeMineNormed.Z*XZSCALE*fond), +(5.0f-fond)*10.0f);
		}

		placeMineEntry(&vector3df((s1.X+s2.X)/2,s1.Y,(s1.Z+s2.Z)/2), axeMine);
	}

	setPatch(&vector3df(x,0,z));

	return MineProfil(&s1,&s2,&s3,&s4, &vector3df(0,0,0));
}

// place de nouvelles instances des meshes d'entrée de mine à la position / selon l'axe spécifié
void CLandManager::placeMineEntry( vector3df * pos, vector3df * axeMine)
{
	double a;
	vector3df axeMineNormalized(*axeMine);
	axeMineNormalized.normalize();
	if( g_data.pPorche && g_data.pCarreau)
	{
		if( abs(axeMineNormalized.X)>abs(axeMineNormalized.Z))
		{
			a=-acos(axeMineNormalized.X)-PI/2;
			if( axeMineNormalized.Z<0 )
				a=-a+PI;
		}
		else
		{
			a=-asin(axeMineNormalized.Z)-PI/2;
			if( axeMineNormalized.X<0 )
				a=-a;
		}

		vector3df rot(0.0f, (irr::f32)(a/PI*180.0f), 0.0f);
		vector3df translatedPos = *pos;
		// déplacement perdendiculaire à l'axe de la mine (le meshe est centré sur 0)
		translatedPos += vector3df (-axeMineNormalized.X*XZSCALE/2, 0, -axeMineNormalized.Z*XZSCALE/2);
		//translatedPos += vector3df (abs(axeMineNormalized.Z)*XZSCALE/2, 0, abs(axeMineNormalized.X)*XZSCALE/2);

		// new SceneNodes out of loaded meshes
		int noSceneNode = g_data.m_carreauSceneNodeArray.size(); // indice of the future element
		g_data.m_carreauSceneNodeArray.push_back(g_data.smgr->addAnimatedMeshSceneNode(g_data.pCarreau, NULL, -1, translatedPos, rot, vector3df(15.0,15.0,15.0)));
		g_data.m_carreauSceneNodeArray[noSceneNode]->setMaterialFlag(EMF_FOG_ENABLE,true);
		if( axeMine->X==0 || axeMine->Z==0)
		{
			g_data.m_porcheSceneNodeArray.push_back(g_data.smgr->addAnimatedMeshSceneNode(g_data.pPorche, NULL, -1, translatedPos, rot, vector3df(20.0,20.0,20.0)));
			g_data.m_porcheSceneNodeArray[noSceneNode]->setMaterialFlag(EMF_FOG_ENABLE,true);
		}
	}
}

// ajoute une zone de patch
// retourne false si refusé (trop proche d'une autre)
void CLandManager::setPatch(vector3df * zonePatch)
{
	m_zonePatchArray.push_back(*zonePatch);
}

const char * CLandManager::getPlaceName(int i)
{
	return (const char*)(m_nameDataBase+m_placeDataBase[i].label+1);
}

coordWGS84 CLandManager::getPlacePosition(int i)
{
	coordWGS84 pos;
	pos.lat  = m_placeDataBase[i].lat;
	pos.lon  = m_placeDataBase[i].lon;
	return pos;
}

int CLandManager::getNbPlace()
{
	return m_placeDataBaseSize/12;
}