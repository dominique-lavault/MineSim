#pragma once

#include <irrlicht.h>
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
/*
   s4    s3
	*----*
	|      |
	|      |
	*----*
	s1    s2

*/
class MineProfil
{
public:
	vector3df m_s1,m_s2,m_s3,m_s4;

	MineProfil(void);
	MineProfil::MineProfil(vector3df * s1, vector3df * s2, vector3df* s3, vector3df* s4, vector3df* pVecteur);
	MineProfil::MineProfil(float largeurRadier, float largeurCiel,float hauteurGalerie, vector3df * vecteur);
	~MineProfil(void);
	void MineProfil::offset(vector3df* pVecteur);
	void MineProfil::rotate(f32 a);
	void MineProfil::reverse();

protected:
	// rend la position 3D 4 vertices du profil en fonction du vecteur portant le profil
	bool getProfilVertices(f32 largeurRadier, f32 largeurCiel,f32 hauteurGalerie, vector3df * s1, vector3df * s2, vector3df * s3, vector3df * s4, vector3df* vecteur);
public:
	vector3df MineProfil::getCenter(void);
};
