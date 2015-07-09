#include "StdAfx.h"
#include ".\minebloc.h"
#include ".\globaldata.h"

extern globalData g_data;

MineBloc::MineBloc(int noBloc, MineBloc * parentBloc, vector3df *pVecteur, vector3df *pAbsoluteVecteur, WALL_ID parentWallId, WALL_ID idForcedProfil, MineProfil * forcedProfil)
{
	for(int i=0; i<NB_WALL_ID; i++)
	{
		selector[i]=NULL;
		m_wallSceneNodes[i]=(MineWall*)NO_PARENT_BLOCK;
		m_childBloc[i]=(MineBloc*)NO_PARENT_BLOCK;
	}

	ISceneNode * parentSceneNode = g_data.smgr->getRootSceneNode();
	m_pParentBloc=parentBloc;
	m_noBloc=noBloc;
	m_blocVecteur=*pVecteur;
	m_absoluteVecteur = *pAbsoluteVecteur;
	WALL_ID discardedWall=WALL_ID_FDTBACK;	// par défaut, on ne crééera pas le FDTBACK

	if( parentBloc == (MineBloc *)NO_PARENT_BLOCK )
	{
		// --------------- root bloc creation -----------------------
		if( idForcedProfil==-1)
			m_profil[0]=MineProfil(40,30,40,&vector3df(0,0,0));
		else
			m_profil[0]=*forcedProfil;

		m_profil[1]=getBillBoardedWall(pVecteur,&m_profil[0]);
		m_profil[1].offset(&m_absoluteVecteur);
		/* on redresse le bouzin */
		m_profil[1].m_s4.X=m_profil[1].m_s1.X;
		m_profil[1].m_s4.Z=m_profil[1].m_s1.Z;
		m_profil[1].m_s3.X=m_profil[1].m_s2.X;
		m_profil[1].m_s3.Z=m_profil[1].m_s2.Z;
		// on réduit la taille du linteau pour trapézoider le profil
		vector3df mid((m_profil[1].m_s3.X+m_profil[1].m_s4.X)/2, m_profil[1].m_s3.Y, (m_profil[1].m_s3.Z+m_profil[1].m_s4.Z)/2);
		vector3df axe((m_profil[1].m_s4.X-mid.X)*2/3, 0, (m_profil[1].m_s4.Z-mid.Z)*2/3);
		m_profil[1].m_s4 = mid+axe;
		m_profil[1].m_s3 = mid-axe;

		if( idForcedProfil==-1)
		{
			m_profil[0]=m_profil[1];
			m_profil[0].reverse();
			pVecteur->invert();
			m_profil[0].offset(pVecteur);
		}
	}
	else	// c'est un bloc "fils" d'un autre
	{
		m_profil[0]=parentBloc->getProfil(parentWallId);

		// on billboarde le FDT
		if( parentWallId==WALL_ID_LEFTWALL || parentWallId== WALL_ID_RIGHTWALL || parentWallId== WALL_ID_FDTFRONT || parentWallId== WALL_ID_FDTBACK)
		{
			m_profil[1]=getBillBoardedWall(pVecteur,&m_profil[0]);
			m_profil[0].reverse();
		}
		else	if (parentWallId==WALL_ID_RADIER)
		{
			pVecteur->X=0;
			pVecteur->Z=0;
			vector3df null(0,0,0);
			MineProfil p=parentBloc->getProfil(WALL_ID_FDTBACK);
			m_profil[0]=MineProfil( &(p.m_s1+*pVecteur), &(p.m_s2+*pVecteur), &p.m_s2, &p.m_s1, &null);
			p=parentBloc->getProfil(WALL_ID_FDTFRONT);
			m_profil[1]=MineProfil( &(p.m_s1+*pVecteur), &(p.m_s2+*pVecteur), &p.m_s2, &p.m_s1, &null);
			discardedWall=WALL_ID_CIEL;
		}
		else	if( parentWallId==WALL_ID_CIEL)
		{
			pVecteur->X=0;
			pVecteur->Z=0;
			vector3df null(0,0,0);
			MineProfil p=parentBloc->getProfil(WALL_ID_FDTBACK);
			m_profil[0]=MineProfil( &p.m_s4, &p.m_s3, &(p.m_s3+*pVecteur), &(p.m_s4+*pVecteur), &null);
			p=parentBloc->getProfil(WALL_ID_FDTFRONT);
			m_profil[1]=MineProfil( &p.m_s4, &p.m_s3, &(p.m_s3+*pVecteur), &(p.m_s4+*pVecteur), &null);
			discardedWall=WALL_ID_RADIER;
		}
	}

	// création des 6 (moins 1) blocs remplaçant le wallmine retiré
	for(int i=0; i<NB_WALL_ID; i++)
	{
		MineProfil profil = computeProfil((WALL_ID)i);
		m_wallSceneNodes[i]=new MineWall(parentSceneNode, g_data.smgr, 127, &profil, this, (WALL_ID)i);
		if( i==discardedWall)
			m_wallSceneNodes[i]->remove();	// nécessaire de le créer quand meme pour avoir sa BBox pour le boisage / les calculs / son profil
	}
	resetBBox();
}

MineProfil MineBloc::getBillBoardedWall(vector3df * vecteur, MineProfil  * pProfil)
{
	vector3df up(0,1.0,0);
	vector3df billboard = -up.crossProduct(*vecteur);
	billboard.normalize();

	vector3df base((pProfil->m_s1.X+pProfil->m_s2.X)/2,
		(pProfil->m_s1.Y+pProfil->m_s2.Y)/2,
		(pProfil->m_s1.Z+pProfil->m_s2.Z)/2);

	vector3df base2((pProfil->m_s3.X+pProfil->m_s4.X)/2,
		(pProfil->m_s3.Y+pProfil->m_s4.Y)/2,
		(pProfil->m_s3.Z+pProfil->m_s4.Z)/2);

	// recréée un profil basé sur la normale au vecteur du bloc
	vector3df radier=pProfil->m_s2-pProfil->m_s1;
	f32 lRadier=(f32)radier.getLength()/2.0f;
	vector3df ciel=pProfil->m_s3-pProfil->m_s4;
	f32 lCiel=(f32)ciel.getLength()/2.0f;
	vector3df s1(billboard.X*lRadier+base.X, base.Y, billboard.Z*lRadier+base.Z);
	vector3df s2(-billboard.X*lRadier+base.X, base.Y, -billboard.Z*lRadier+base.Z);
	vector3df s3(-billboard.X*lCiel+base2.X, base2.Y,- billboard.Z*lCiel+base2.Z);
	vector3df s4(billboard.X*lCiel+base2.X, base2.Y, billboard.Z*lCiel+base2.Z);
	return MineProfil(&s1,&s2,&s3,&s4,vecteur);
}

void MineBloc::resetBBox()
{
	m_BBox.reset(m_profil[0].m_s1);
	m_BBox.addInternalPoint(m_profil[0].m_s2);
	m_BBox.addInternalPoint(m_profil[0].m_s3);
	m_BBox.addInternalPoint(m_profil[0].m_s4);
	m_BBox.addInternalPoint(m_profil[1].m_s1);
	m_BBox.addInternalPoint(m_profil[1].m_s2);
	m_BBox.addInternalPoint(m_profil[1].m_s3);
	m_BBox.addInternalPoint(m_profil[1].m_s4);
}

MineWall * MineBloc::getSelectedMineWall(line3d<f32> *pLine, vector3df * pIntersectionPoint, bool onlyActiveWalls)
{
	for(int i=0; i<NB_WALL_ID; i++)
	{
		if( m_wallSceneNodes[i]!=(MineWall*)NO_PARENT_BLOCK )
			if( m_wallSceneNodes[i]->intersects(pLine,pIntersectionPoint,onlyActiveWalls))
				return m_wallSceneNodes[i];
	}
	return NULL;
}


MineBloc * MineBloc::getChildMineBloc(WALL_ID targetWallId)
{
	return m_childBloc[targetWallId];
}

void MineBloc::setChildMineBloc(WALL_ID noWall, MineBloc * childBloc)
{
	m_childBloc[noWall]=childBloc;
}

//------------------------- calcul un profil quelconque à partir du profil d'entrée et de sortie du bloc -------------------------------------
MineProfil MineBloc::computeProfil(WALL_ID wallId)
{
	if( wallId==WALL_ID_FDTBACK)
	{
		return m_profil[0];
	}
	if( wallId==WALL_ID_FDTFRONT)
	{
		return m_profil[1];
	}
	vector3df nullVector(0,0,0);
	if( wallId==WALL_ID_RADIER)
	{
		// attention, le profil[0] est tourné vers l'intérieur
		MineProfil profil(&m_profil[0].m_s2,&m_profil[0].m_s1,&m_profil[1].m_s2,&m_profil[1].m_s1,&nullVector);
		return profil;
	}
	if( wallId==WALL_ID_CIEL)
	{
		MineProfil profil(&m_profil[0].m_s4,&m_profil[0].m_s3,&m_profil[1].m_s4,&m_profil[1].m_s3,&nullVector);
		return profil;
	}
	if( wallId==WALL_ID_LEFTWALL)
	{
		MineProfil profil(&m_profil[0].m_s2,&m_profil[1].m_s1,&m_profil[1].m_s4,&m_profil[0].m_s3,&nullVector);
		return profil;
	}
//	if( wallId==WALL_ID_RIGHTWALL)
//	{
		MineProfil profil(&m_profil[1].m_s2,&m_profil[0].m_s1,&m_profil[0].m_s4,&m_profil[1].m_s3,&nullVector);
		return profil;
	//}
}

MineBloc::~MineBloc(void)
{
	for(int i=0; i<5; i++)
		if( selector[i]!=NULL)
			selector[i]->drop();
}

MineProfil MineBloc::getProfil(WALL_ID noWall)
{
	return MineBloc::computeProfil(noWall);
}

// set le profil du bloc : WALL_ID_FDTBACK ou WALL_ID_FDTFRONT ou radier ou ciel
void MineBloc::setProfil(MineProfil * profil, WALL_ID wallId)
{
	if( wallId == WALL_ID_FDTBACK)
		m_profil[0]=*profil;
	else  if( wallId == WALL_ID_FDTFRONT)
		m_profil[1]=*profil;
	else  if( wallId == WALL_ID_RADIER)
	{
		m_profil[0].m_s1=profil->m_s2;
		m_profil[0].m_s2=profil->m_s1;
		m_profil[1].m_s1=profil->m_s4;
		m_profil[1].m_s2=profil->m_s3;
	}
	else  if( wallId == WALL_ID_CIEL)
	{
		m_profil[0].m_s4=profil->m_s1;
		m_profil[0].m_s3=profil->m_s2;
		m_profil[1].m_s3=profil->m_s3;	//¤	m_profil[1].m_s3=profil->m_s4;
		m_profil[1].m_s4=profil->m_s4;	//¤ m_profil[1].m_s4=profil->m_s3;
	}
	else 
		return;

	if( m_wallSceneNodes[WALL_ID_FDTBACK]!=NO_MINEWALL)
	{
		m_wallSceneNodes[WALL_ID_FDTBACK]->setProfil(&m_profil[0]);
	}
	if( m_wallSceneNodes[WALL_ID_FDTFRONT]!=NO_MINEWALL)
		m_wallSceneNodes[WALL_ID_FDTFRONT]->setProfil(&m_profil[1]);
	if( m_wallSceneNodes[WALL_ID_LEFTWALL]!=NO_MINEWALL)
		m_wallSceneNodes[WALL_ID_LEFTWALL]->setProfil(&computeProfil(WALL_ID_LEFTWALL));
	if( m_wallSceneNodes[WALL_ID_RIGHTWALL]!=NO_MINEWALL)
		m_wallSceneNodes[WALL_ID_RIGHTWALL]->setProfil(&computeProfil(WALL_ID_RIGHTWALL));
	if( m_wallSceneNodes[WALL_ID_RADIER]!=NO_MINEWALL)
		m_wallSceneNodes[WALL_ID_RADIER]->setProfil(&computeProfil(WALL_ID_RADIER));
	if( m_wallSceneNodes[WALL_ID_CIEL]!=NO_MINEWALL)
		m_wallSceneNodes[WALL_ID_CIEL]->setProfil(&computeProfil(WALL_ID_CIEL));	
	resetBBox();
}

MineWall * MineBloc::getWallSceneNode(WALL_ID wallID)
{
	if(wallID<0 || wallID>=NB_WALL_ID)
		return NULL;
	return m_wallSceneNodes[wallID];
}

int MineBloc::getNoBloc(void)
{
	return m_noBloc;
}

MineBloc * MineBloc::getParentBloc(void)
{
	return m_pParentBloc;
}

vector3df MineBloc::getBlocVector(void)
{
	return m_blocVecteur;
}
vector3df MineBloc::getBlocAbsoluteVector(void)
{
	return m_absoluteVecteur;
}

void MineBloc::setVisible(bool bVisible)
{
	for(int i=0; i<NB_WALL_ID; i++)
	{
		if( m_wallSceneNodes[i]!=NULL )
			m_wallSceneNodes[i]->setVisible(bVisible);
	}
}

void MineBloc::setLOD(LOD lod)
{
	for(int i=0; i<NB_WALL_ID; i++)
	{
		if( m_wallSceneNodes[i]!=NULL )
			m_wallSceneNodes[i]->m_LOD=lod;
	}
}
