#include "StdAfx.h"
#include <irrlicht.h>
#include ".\mineprofil.h"

using namespace irr;
using namespace core;

MineProfil::MineProfil(void)
{
}

MineProfil::MineProfil(float largeurRadier, float largeurCiel,float hauteurGalerie, vector3df * vecteur)
{
	getProfilVertices(largeurRadier, largeurCiel,hauteurGalerie, &m_s1, &m_s2, &m_s3, &m_s4, vecteur);
}

MineProfil::MineProfil(vector3df * s1, vector3df * s2, vector3df * s3, vector3df * s4, vector3df* pVecteur)
{
	m_s1=*s1;
	m_s2=*s2;
	m_s3=*s3;
	m_s4=*s4;
	offset(pVecteur);
}

void MineProfil::offset(vector3df* pVecteur)
{
	m_s1+=*pVecteur;
	m_s2+=*pVecteur;
	m_s3+=*pVecteur;
	m_s4+=*pVecteur;
}

void MineProfil::reverse()
{ 
	vector3df s(m_s1);
	m_s1=m_s2;
	m_s2=s;
	s=m_s4;
	m_s4=m_s3;
	m_s3=s;
}

void MineProfil::rotate(f32 a)
{ 
	vector3df s1(m_s1),s2(m_s2),s3(m_s3),s4(m_s4);
	f32 ca=cos(a);
	f32 sa=sin(a);
	s1.X=m_s1.X*ca+m_s1.Z*sa;
	s1.Z=-m_s1.X*sa+m_s1.Z*ca;
	s2.X=m_s2.X*ca+m_s2.Z*sa;
	s2.Z=-m_s2.X*sa+m_s2.Z*ca;
	s3.X=m_s3.X*ca+m_s3.Z*sa;
	s3.Z=-m_s3.X*sa+m_s3.Z*ca;
	s4.X=m_s4.X*ca+m_s4.Z*sa;
	s4.Z=-m_s4.X*sa+m_s4.Z*ca;
	m_s1=s1;
	m_s2=s2;
	m_s3=s3;
	m_s4=s4;
}

MineProfil::~MineProfil(void)
{
}

// rend la position 3D 4 vertices du profil en fonction du vecteur portant le profil
bool MineProfil::getProfilVertices(float largeurRadier, float largeurCiel,float hauteurGalerie, vector3df * s1, vector3df * s2, vector3df * s3, vector3df * s4, vector3df* vecteur)
{
	vector3df pos;
	pos=*vecteur;
	SColor c(255,255,255,255);
	vector2d< f32 > t(0,0);

	pos.X+=largeurRadier/2;
	vector3df n(0,-1,0);
	*s1=pos;
	pos.X-=largeurRadier;
	*s2=pos;

	pos=*vecteur;
	pos.Y+=hauteurGalerie;
	pos.X-=largeurCiel/2;
	*s3=pos;
	pos.X+=largeurCiel;
	*s4=pos;

	return true;
}

vector3df MineProfil::getCenter(void)
{
	vector3df center;
	center.X=(m_s1.X+m_s2.X+m_s3.X+m_s4.X)/4.0f;
	center.Y=(m_s1.Y+m_s2.Y+m_s3.Y+m_s4.Y)/4.0f;
	center.Z=(m_s1.Z+m_s2.Z+m_s3.Z+m_s4.Z)/4.0f;
	return center;
}
