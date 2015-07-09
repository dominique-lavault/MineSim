#include "StdAfx.h"

#include <irrlicht.h>

#include ".\galerie.h"
#include "mineBloc.h"
#include "mineBoisage.h"
#include "GlobalData.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

extern globalData g_data;

Galerie::Galerie(void)
{
	m_oldBlocContainingCamera=-1;
	m_nbBloc=0;
}

Galerie::~Galerie(void)
{
	for(int i=0; i<m_nbBloc; i++)
		delete m_blocCollection[i];
}

// retourne un profil rotaté selon l'angle entre les axes des blocs portant les profils (newProfil est donné dans le répère myBlocVector)
MineProfil Galerie::makeMatchingProfil(vector2d<f32> myBlocVector, vector2d<f32> foreignBlocVector, MineProfil newProfil)
{
	vector3df tmp;
	myBlocVector.normalize();
	foreignBlocVector.normalize();
	double myBlocAngle=acos(myBlocVector.X);
	if( myBlocVector.Y<0 )
		myBlocAngle=-myBlocAngle;
	double foreignBlocAngle=acos(foreignBlocVector.X);
	if( foreignBlocVector.Y<0 )
		foreignBlocAngle=-foreignBlocAngle;

	f64 angle = (foreignBlocAngle-myBlocAngle)/PI*180.0;

	if( angle > 45.0 && angle <= 135.0)
	{
		// on tourne le newProfil de 90 ° vers la gauche
		tmp=newProfil.m_s1;
		newProfil.m_s1=newProfil.m_s4;
		newProfil.m_s4=newProfil.m_s3;
		newProfil.m_s3=newProfil.m_s2;
		newProfil.m_s2=tmp;
	}
	else if( angle < -45.0 && angle >= -135.0)
	{
		// on tourne le newProfil de 90 ° vers la droite
		tmp=newProfil.m_s1;
		newProfil.m_s1=newProfil.m_s2;
		newProfil.m_s2=newProfil.m_s3;
		newProfil.m_s3=newProfil.m_s4;
		newProfil.m_s4=tmp;
	}
	else if( angle < -135.0 || angle > 135.0)
	{
		// on tourne le newProfil de 180 °
		tmp=newProfil.m_s1;
		newProfil.m_s1=newProfil.m_s3;
		newProfil.m_s3=tmp;
		tmp=newProfil.m_s4;
		newProfil.m_s4=newProfil.m_s2;
		newProfil.m_s2=tmp;
	}
	return newProfil;
}

// ======================== rajoute un bloc à la collection========================================
MineBloc * Galerie::AddBloc(int noParentBloc, WALL_ID noParentWall, vector3df vecteur, WALL_ID bForcedProfil, MineProfil * forcedProfil)
{
	if(m_nbBloc<MAX_NBLOCS-1)
	{
		if( noParentBloc==NO_PARENT_BLOCK )
		{// create root bloc
//			MineBloc * mb = (MineBloc *)new int(0);
			MineBloc * mb = new MineBloc(m_nbBloc,(MineBloc *)NO_PARENT_BLOCK, &vecteur, &g_data.m_origin, noParentWall, bForcedProfil, forcedProfil);
			m_blocCollection[m_nbBloc]=mb;
			m_listeBlocEntree.push_back(0);
		}
		else
		{
			vector3df absVector;
			absVector=m_blocCollection[noParentBloc]->getBlocAbsoluteVector();

			// on teste si on va recouper le terrain et si oui, si c'est possible de faire une sortie là?
			MineProfil parentProfil=m_blocCollection[noParentBloc]->getProfil(noParentWall);
			parentProfil.offset(&vecteur);
			if( g_data.m_landManager->getAltitudeWithDelta(parentProfil.m_s3.X,parentProfil.m_s3.Z)<parentProfil.m_s3.Y
				|| g_data.m_landManager->getAltitudeWithDelta(parentProfil.m_s4.X,parentProfil.m_s4.Z)<parentProfil.m_s4.Y )
			{
				if( ! g_data.m_landManager->IsItPossibleToCreateGalerieJonction(&parentProfil.m_s3))
				{
					// afficher un message dans le log user
					s32 sel = g_data.m_messagelistbox->addItem(L"Impossible de faire une sortie si près d'une autre sans la fragiliser.");
					g_data.m_messagelistbox->setItemColor(sel, SColor(0,255,0,0));
					g_data.m_messagelistbox->setListTopItem(max(0,sel-3));
					return NULL;	// pas de bras, pas de chocolat
				}
			}

			// creation du nouveau bloc + on retrouve le bloc parent et on lui donne le pointeur sur le nouvel enfant
			m_blocCollection[m_nbBloc]=new MineBloc(m_nbBloc,m_blocCollection[noParentBloc], &vecteur, &absVector, noParentWall);
			m_blocCollection[noParentBloc]->setChildMineBloc(noParentWall, m_blocCollection[m_nbBloc]);

			WALL_ID destination=WALL_ID_FDTFRONT;
			MineProfil pfront = m_blocCollection[m_nbBloc]->getProfil(destination);	// le profil de notre FDT flambant neuf
			MineProfil pback = m_blocCollection[noParentBloc]->getProfil(noParentWall);	// le profil du mur qu'on descend (=celui cliqué)
			int noBloc=getBlocContainingCamera();

			// ===================== test de recoupement avec le terrain =======================
			if( g_data.m_landManager->getAltitudeWithDelta(pfront.m_s3.X,pfront.m_s3.Z)<pfront.m_s3.Y
				|| g_data.m_landManager->getAltitudeWithDelta(pfront.m_s4.X,pfront.m_s4.Z)<pfront.m_s4.Y )
			{
				bool bTerrassement=true;
				WALL_ID targetWall=WALL_ID_FDTFRONT;

				if( noParentWall == WALL_ID_CIEL )
				{
					targetWall = WALL_ID_CIEL;
					bTerrassement=false;
				}
				vector3df mid((pfront.m_s3.X+pfront.m_s4.X)/2,pfront.m_s3.Y, (pfront.m_s3.Z+pfront.m_s4.Z)/2);
				MineProfil profil = g_data.m_landManager->createGalerieJonction(mid.X, mid.Z, &vecteur, bTerrassement);
				profil.reverse();
				if( noParentWall == WALL_ID_CIEL )
					profil = makeMatchingProfil(vector2d<f32>(profil.m_s2.X-profil.m_s1.X,profil.m_s2.Z-profil.m_s1.Z), vector2d<f32>(-1,0), profil);

				// si c'est le ciel qu'on creuse-> on affecte le ciel, sinon, le front de taille
				m_blocCollection[m_nbBloc]->setProfil(&profil, targetWall);
				m_blocCollection[m_nbBloc]->getWallSceneNode(targetWall)->remove();
				g_data.m_landManager->patchLand();	// update patch

				m_listeBlocEntree.push_back(m_nbBloc);
				g_data.m_camera->setMode(CAMERA_MODE_INDOOR_NEAR_ENTRANCE);
			}

			// ============ si c'est un abattage vers le haut ou le bas, ajouter un échaffaudage de puits ===============
			if( noParentWall==WALL_ID_RADIER || noParentWall==WALL_ID_CIEL  )
			{
				destination=noParentWall;
				MineProfil pback = m_blocCollection[noParentBloc]->getProfil(noParentWall);
				vector3df pos ((pback.m_s1.X+pback.m_s3.X)/2, (pback.m_s1.Y+pback.m_s3.Y)/2, (pback.m_s1.Z+pback.m_s3.Z)/2);
				IAnimatedMeshSceneNode * asn = g_data.smgr->addAnimatedMeshSceneNode(g_data.pEtagePuits, NULL, -1, pos, vector3df(0,0,0), vector3df(20.0,20.0,20.0));
				//-> la var est à 0 !!
				g_data.m_etagePuitsSceneNodeArray.push_back(asn);
				int noSceneNode=g_data.m_etagePuitsSceneNodeArray.size()-1;
				ISceneNode * node = 	g_data.m_etagePuitsSceneNodeArray[noSceneNode];
				node->setMaterialFlag(EMF_LIGHTING,true);
				node->setMaterialType(EMT_SOLID);			
				node->setMaterialTexture(0,g_data.driver->getTexture("Media/wood.jpg"));
				node->getMaterial(0).EmissiveColor.set(255,0,0,0);
				node->getMaterial(0).AmbientColor.set(255,255,255,255);
				node->getMaterial(0).DiffuseColor=SColor(255,0,0,0);
				node->getMaterial(0).NormalizeNormals=true;
			}

			// =============== on teste si notre nouveau bloc va recouper un bloc existant ===============
			for(int i=0; i<m_nbBloc; i++)
			{
				if(	i!=noBloc &&	m_blocCollection[i]->m_BBox.intersectsWithBox(m_blocCollection[m_nbBloc]->m_BBox))
				{
					pback.offset(&(vecteur/10.0));	// eviter la collision avec les murs jointifs des blocs adjacents
					pfront.offset(&(vecteur/5.0));	// eviter les murs trop fins : on abat un peu large
					vector3df v13=(pfront.m_s3-pfront.m_s1)/4.0f;
					vector3df v24=(pfront.m_s4-pfront.m_s2)/4.0f;
					// il y a peut etre collision entre les blocs -> trouver lequel des walls est concerné
					line3d<f32> line11(pfront.m_s1,pback.m_s1);
					line3d<f32> line22(pfront.m_s2,pback.m_s2);
					line3d<f32> line33(pfront.m_s3,pback.m_s3);
					line3d<f32> line44(pfront.m_s4,pback.m_s4);
					// diagonales du cube
					line3d<f32> line13(pfront.m_s1,pback.m_s2);
					line3d<f32> line24(pfront.m_s2, pback.m_s3);
					line3d<f32> line42(pfront.m_s4, pback.m_s1);
					line3d<f32> line31(pfront.m_s3, pback.m_s2);
					// carré du FDT front
					line3d<f32> line12(pfront.m_s1-v13,pfront.m_s2-v24);
					line3d<f32> line23(pfront.m_s2-v24, pfront.m_s3+v13);
					line3d<f32> line34(pfront.m_s3+v13, pfront.m_s4+v24);
					line3d<f32> line41(pfront.m_s4+v24, pfront.m_s1-v13);

					bool coll;
					vector3df v;
					//vector3df refPos=g_data.m_camera->getAbsolutePosition();
					vector3df refPos=pback.getCenter();
					MineWall * candidateWall=NO_MINEWALL;
					f64 d, candidateDistance=999999.9;					

//					for(int j=0; j<NB_WALL_ID; j++)	// version on teste aussi les collisions radier/ciel
					for(int j=WALL_ID_LEFTWALL; j<NB_WALL_ID; j++)
					{
						MineWall * pMineWall = m_blocCollection[i]->getWallSceneNode((WALL_ID)j);	// on examine chacun des minewall de notre candidat
						if( pMineWall != (MineWall *)NO_PARENT_BLOCK)
						{
							coll = pMineWall->intersects(&line12);
							if( ! coll)	coll = pMineWall->intersects(&line23);
							if( ! coll)	coll = pMineWall->intersects(&line34);
							if( ! coll)	coll = pMineWall->intersects(&line41);
							if( ! coll)	coll = pMineWall->intersects(&line11);
							if( ! coll)	coll = pMineWall->intersects(&line22);
							if( ! coll)	coll = pMineWall->intersects(&line33);
							if( ! coll)	coll = pMineWall->intersects(&line44);
							if( ! coll)	coll = pMineWall->intersects(&line13);
							if( ! coll)	coll = pMineWall->intersects(&line24);
							if( ! coll)	coll = pMineWall->intersects(&line31);
							if( ! coll)	coll = pMineWall->intersects(&line42);
							if( coll)
							{
								d=pMineWall->Box.getCenter().getDistanceFrom(refPos);
								if( d<candidateDistance)
								{
									candidateDistance=d;
									candidateWall=pMineWall;
								}
							}
						}
					}

					if( candidateWall!=NO_MINEWALL)
					{
						// récupérer le profil du minewall candidat pour se faire descendre
						MineProfil * pProfil = candidateWall->getProfil();
						MineProfil newProfil=*pProfil;
						newProfil.reverse();

						// todo bug: quand on creuse ID_RADIER, suivant le sens où on creuse, le profil est "tourné"
						if( candidateWall->getWallId()==WALL_ID_RADIER || candidateWall->getWallId()==WALL_ID_CIEL)
						{
							vector3df tmp = m_blocCollection[noParentBloc]->getBlocVector();
							vector2d<f32> myBlocVector(tmp.X, tmp.Z);
							vector2d<f32> foreignBlocVector(m_blocCollection[i]->getBlocVector().X, m_blocCollection[i]->getBlocVector().Z);
							newProfil = makeMatchingProfil(myBlocVector, foreignBlocVector,newProfil);
						}

						// test (?)
						if( destination==WALL_ID_RADIER && candidateWall->getWallId()!=WALL_ID_CIEL)
						{
							vector3df tmp = m_blocCollection[noParentBloc]->getBlocVector();
							vector2d<f32> myBlocVector(tmp.X, tmp.Z);
							vector2d<f32> foreignBlocVector(m_blocCollection[i]->getBlocVector().X, m_blocCollection[i]->getBlocVector().Z);
							newProfil = makeMatchingProfil(myBlocVector, foreignBlocVector,newProfil);
						}

						m_blocCollection[m_nbBloc]->setProfil(&newProfil, destination);	// le profil de notre FDT flambant neuf
						// si à la suite de cette opération, on fusionne deux points (avec le FDTBACK), il faut supprimer le mur ainsi collapsé
						MineProfil currentBlocFDTBACK = m_blocCollection[m_nbBloc]->getProfil(WALL_ID_FDTBACK);
						if( currentBlocFDTBACK.m_s1 == newProfil.m_s1 || currentBlocFDTBACK.m_s1 == newProfil.m_s2 )
							m_blocCollection[m_nbBloc]->getWallSceneNode(WALL_ID_RIGHTWALL)->remove();
						if( currentBlocFDTBACK.m_s2 == newProfil.m_s2 || currentBlocFDTBACK.m_s2 == newProfil.m_s1)
							m_blocCollection[m_nbBloc]->getWallSceneNode(WALL_ID_LEFTWALL)->remove();
	
						// descendre le mur intersecté de l'ancien bloc.
						candidateWall->remove();

						// descendre le nouveau FDTFRONT						
						m_blocCollection[m_nbBloc]->getWallSceneNode(destination)->remove();
						goto found;
					}
				}
			}	// fin test recoupement de blocs

found:
			// ------------------------------ boisage auto ---------------------------
			// on ne boise pas quand on creuse haut et bas c'est déjà fait dans les puits de mine
			if( noParentWall!=WALL_ID_RADIER && noParentWall!=WALL_ID_CIEL)
			{
				MineProfil profil1=m_blocCollection[m_nbBloc]->getProfil(WALL_ID_FDTFRONT);
				ISceneNode* parentNode = g_data.smgr->getRootSceneNode();
				MineBoisage* b1 = new MineBoisage(parentNode, g_data.smgr, 1, &profil1, m_blocCollection[m_nbBloc], vecteur); 
				m_boisageCollection.push_back(b1);
			}

		}
		m_nbBloc++;
		return m_blocCollection[m_nbBloc-1];
	}
	return NULL;
}

// retourne le MineWall intersectant la ligne passée en paramètre du bloc en cours (contenant la caméra)
// fonction destinée à l'abatage des murs
MineWall * Galerie::getSelectedMineWall(line3d<f32> * pLine,vector3df * pIntersectionPoint, bool onlyActiveWalls)
{
	MineWall * pMineWall;
	vector3df intersectionPoint;
	int noBloc=getBlocContainingCamera();
	if( noBloc != -1 )
		if( pMineWall = m_blocCollection[noBloc]->getSelectedMineWall(pLine, pIntersectionPoint, onlyActiveWalls))
			return pMineWall;
	return NULL;
}

// renvoie une ligne verticale centrée sur (pos) et traversant verticalement la bounding bloc du radier
// utile pour les recherche d'intersection camera / bloc
line3d<f32> Galerie::getBlocVerticalLine(int blocIdx, vector3df * pos)
{
	MineBloc * bloc = m_blocCollection[blocIdx];
	vector3df v1(*pos);
	vector3df v2(*pos);

	MineWall * mw = bloc->getWallSceneNode(WALL_ID_RADIER);
	v1.Y=mw->Box.MaxEdge.Y+1.0f;
	v2.Y=mw->Box.MinEdge.Y-1.0f;

	return line3d<f32>(v1,v2);
}

// renvoie dans *altitude l'altitude du sol sous le point donné. renvoie false si l'altitude est indéfinie (ex: point donné
// en dehors de la galerie
bool Galerie::getAltitudeAt(vector3df * pos, float * altitude)
{
	int blocIdx = getBlocContainingCamera();
	if( blocIdx != -1 )
	{
		vector3df intersection;
		line3d<f32> line = getBlocVerticalLine(blocIdx, pos);
		if( line.start.Y>*altitude )
			line.start.Y=*altitude;
		if( m_blocCollection[blocIdx]->getSelectedMineWall(&line, &intersection,true))
		{
			*altitude = intersection.Y;
			return true;
		}
		else
		{
			// on quitte la galerie -> expédié à la surface (on a pas à être dans ce bout de code, en fait).
			return false;
		}
	}
	return false;
}

// renvoie le numéro du bloc dans lequel se trouve la caméra, -1 s'il n'y en a pas
int Galerie::getBlocContainingCamera()
{
	// souvent la caméra n'a pas bougé de bloc, on vérifie avant de tous se les taper
	vector3df camPos = g_data.m_camera->m_camPos;
	if( m_oldBlocContainingCamera != -1 )
	{
		if( m_blocCollection[m_oldBlocContainingCamera]->m_BBox.isPointInside(camPos) )
			if( m_blocCollection[m_oldBlocContainingCamera]->getSelectedMineWall(&getBlocVerticalLine(m_oldBlocContainingCamera, &camPos),NULL,false))
			{
				return m_oldBlocContainingCamera;
			}
	}
	// si ça a changé (ou si on le connait pas encore), bin on cherche.
	for(int i=0; i<m_nbBloc; i++)
	{
		if( m_blocCollection[i]->m_BBox.isPointInside(camPos) )
		{
			//ok, on a un candidat, maintenant on regarde plus en détail
			line3d<f32> line = getBlocVerticalLine(i, &camPos);
			if( m_blocCollection[i]->getSelectedMineWall(&line,NULL,false))
			{
				m_oldBlocContainingCamera=i;
				return i;
			}
		}
	}
	return -1; // not found.
}

int Galerie::GetBlocCount(void)
{
	return m_nbBloc;
}

MineBloc * Galerie::getBloc(int noBloc)
{
	if( noBloc >=0 && noBloc< GetBlocCount())
		return m_blocCollection[noBloc];
	else
		return NULL;
}

void Galerie::setBlocMaterial(int noBloc, E_MATERIAL_FLAG material, bool activate)
{
	for(int i=0; i<6; i++)
	{
		MineWall * pMineWall = m_blocCollection[noBloc]->getWallSceneNode((WALL_ID)i);
		if( pMineWall != (MineWall *)NO_MINEWALL)
			pMineWall->setMaterialFlag(material,activate);
	}
}

// rend visible (ou non) les polygones de la mine
void Galerie::setUndergroundPartVisible(bool bVisible)
{
	unsigned int i;
	for(i=0; i<(unsigned int)m_nbBloc; i++)
		if( ! isEntrance(i))
			m_blocCollection[i]->setVisible(bVisible);
	unsigned int n = m_boisageCollection.size();
	for(i=0; i<n; i++)
		m_boisageCollection[i]->setVisible(bVisible);
		
}

// renvoie true si le bloc est une des entrées de la mine
// param : noBloc : numéro du bloc
bool Galerie::isEntrance(unsigned int noBloc)
{
	for(unsigned int i=0; i<m_listeBlocEntree.size(); i++)
	{
		if( m_listeBlocEntree[i]==noBloc)
			return true;
	}
	return false;
}

// recalcule le niveau de détail (LOD) de chaque bloc de la galerie en fonction de la distance au point de référence passé en paramètre
void Galerie::setLOD(vector3df & refPos)
{
	#define DISTANCE_LOD_LOWEST_SQUARED	(2000*2000)
	#define DISTANCE_LOD_LOW_SQUARED	(300*300)
	#define DISTANCE_LOD_JOINT_LOWHIGH_SQUARED	(260*260)

	LOD lod;
	f64 d;
	unsigned int i;
	for(i=0; i<(unsigned int)m_nbBloc; i++)
	{
		d = refPos.getDistanceFromSQ(m_blocCollection[i]->m_BBox.getCenter());
		if( d>DISTANCE_LOD_LOWEST_SQUARED)
			lod=LOD_LOWEST;
		else	if( d>DISTANCE_LOD_LOW_SQUARED)
			lod=LOD_LOW;
		else	if( d>DISTANCE_LOD_JOINT_LOWHIGH_SQUARED)
			lod=LOD_JOINT_LOWHIGH;
		else
			lod=LOD_HIGH;	
		m_blocCollection[i]->setLOD(lod);
	}
}