#include "StdAfx.h"
#include <irrlicht.h>
#include ".\MineBoisage.h"
#include ".\MineBloc.h"
#include "GlobalData.h"

using namespace irr;
using namespace video;
extern globalData g_data;

S3DVertex MineBoisage::makeS3DVertex(vector3df pos, vector3df offset, dimension2d<f32> tcoords, vector3df &center)
{
	S3DVertex v;
	v.Color.set(255,100,85,50);
	v.Pos.set(pos+offset);
	v.TCoords.set(tcoords.Width, tcoords.Height);
	v.Normal=v.Pos-center;
	v.Normal.normalize();
	return v;
}

void MineBoisage::pushPoly(SMeshBuffer * buffer, int v1, int v2, int v3)
{
	buffer->Indices.push_back(v1);
	buffer->Indices.push_back(v2);
	buffer->Indices.push_back(v3);
}
int MineBoisage::generatePoutre(float diameter, vector3df &pt1, vector3df &pt2, SMeshBuffer * buffer, vector3df vecteurGalerie)
{
	vecteurGalerie.normalize();
	vecteurGalerie*=diameter/2.0f;
	int n = buffer->Vertices.size();
	// vertices
	buffer->Vertices.push_back( makeS3DVertex(pt1+vecteurGalerie, vector3df(0,+diameter/2.0f, 0), dimension2d<f32>(0,1),pt1)); // 0
	buffer->Vertices.push_back( makeS3DVertex(pt1+vecteurGalerie, vector3df(0,-diameter/2.0f, 0), dimension2d<f32>(0,0),pt1));  // 1
	buffer->Vertices.push_back( makeS3DVertex(pt1-vecteurGalerie, vector3df(0,-diameter/2.0f, 0), dimension2d<f32>(0,1),pt1)); // 2
	buffer->Vertices.push_back( makeS3DVertex(pt1-vecteurGalerie, vector3df(0,+diameter/2.0f, 0), dimension2d<f32>(0,0),pt1)); // 3
	buffer->Vertices.push_back( makeS3DVertex(pt2+vecteurGalerie, vector3df(0,+diameter/2.0f, 0), dimension2d<f32>(1,1),pt2));  // 4
	buffer->Vertices.push_back( makeS3DVertex(pt2+vecteurGalerie, vector3df(0,-diameter/2.0f, 0), dimension2d<f32>(1,0),pt2));  // 5
	buffer->Vertices.push_back( makeS3DVertex(pt2-vecteurGalerie, vector3df(0,-diameter/2.0f, 0), dimension2d<f32>(1,1),pt2));  // 6
	buffer->Vertices.push_back( makeS3DVertex(pt2-vecteurGalerie, vector3df(0,+diameter/2.0f, 0), dimension2d<f32>(1,0),pt2));  // 7
	// indices
	pushPoly(buffer, n+0, n+2, n+1);
	pushPoly(buffer, n+0, n+3, n+1);
	pushPoly(buffer, n+0, n+4, n+7);
	pushPoly(buffer, n+0, n+7, n+3);
	pushPoly(buffer, n+0, n+1, n+4);
	pushPoly(buffer, n+1, n+5, n+4);
	pushPoly(buffer, n+5, n+6, n+7);
	pushPoly(buffer, n+4, n+5, n+7);
	pushPoly(buffer, n+3, n+7, n+2);
	pushPoly(buffer, n+7, n+6, n+2);
	pushPoly(buffer, n+6, n+1, n+2);
	pushPoly(buffer, n+6, n+5, n+1);
	return 0;
}

int MineBoisage::generateCylinder(int faces, vector3df &pt1, vector3df &pt2, SMeshBuffer * buffer, float angleY, bool bHorizontal)
{
	int i;
	int cntVertice, initialVertice,cap1, cap2;
	video::S3DVertex vtx;
	
	// caps centers
	cap1 = buffer->Vertices.size();
	vtx = makeS3DVertex(pt1, vector3df(0,0,0), dimension2d<f32>(pt1.Y/8, pt1.X/8),pt1);
	m_buffer->Vertices.push_back(vtx);

	cap2=cap1+1;
	vtx = makeS3DVertex(pt2, vector3df(0,0,0), dimension2d<f32>(pt2.Y/8, pt2.X/8),pt2);
	m_buffer->Vertices.push_back(vtx);

	initialVertice = cntVertice = buffer->Vertices.size();

	for( i=0; i<faces; i++)
	{
		// create vertices
		vtx.Pos.set( computePoint(i, faces, 2.0f, pt1,angleY+(f32)GRAD_PI/4.0f, bHorizontal));
		vtx.TCoords.set(vtx.Pos.Y/16, vtx.Pos.X/16);
		vtx.Normal=vtx.Pos-pt1;
		vtx.Normal.normalize();
		m_buffer->Vertices.push_back(vtx);
		cntVertice++;

		vtx.Pos.set( computePoint(i, faces, 2.0f, pt2,angleY+(f32)GRAD_PI/4.0f, bHorizontal));
		vtx.TCoords.set(vtx.Pos.Y/16, vtx.Pos.X/16);
		vtx.Normal=vtx.Pos-pt2;
		vtx.Normal.normalize();
		m_buffer->Vertices.push_back(vtx);

		// create indices
		if( i<faces-1)
		{
			m_buffer->Indices.push_back(cntVertice-1);
			m_buffer->Indices.push_back(cntVertice+1);
			m_buffer->Indices.push_back(cntVertice);
			m_buffer->Indices.push_back(cntVertice+2);
			m_buffer->Indices.push_back(cntVertice);
			m_buffer->Indices.push_back(cntVertice+1);
			// caps
			m_buffer->Indices.push_back(cntVertice+1);
			m_buffer->Indices.push_back(cntVertice-1);
			m_buffer->Indices.push_back(cap1);
			m_buffer->Indices.push_back(cntVertice);
			m_buffer->Indices.push_back(cntVertice+2);
			m_buffer->Indices.push_back(cap2);
		}
		cntVertice++;
	}
	// fermer le tour du cylindre
	m_buffer->Indices.push_back(cntVertice-1);
	m_buffer->Indices.push_back(cntVertice-2);
	m_buffer->Indices.push_back(initialVertice);
	m_buffer->Indices.push_back(cntVertice-1);
	m_buffer->Indices.push_back(initialVertice);
	m_buffer->Indices.push_back(initialVertice+1);
	// caps
	m_buffer->Indices.push_back(initialVertice);
	m_buffer->Indices.push_back(cntVertice-2);
	m_buffer->Indices.push_back(cap1);
	m_buffer->Indices.push_back(cntVertice-1);
	m_buffer->Indices.push_back(initialVertice+1);
	m_buffer->Indices.push_back(cap2);
	return cntVertice;
}

vector3df MineBoisage::computePoint(int noPt, int nbPt, float diameter, vector3df center, float angleY, bool bVertical)
{
	float angle=-(2.0f*PI * (float)noPt /(float)nbPt);
	if( ! bVertical )
	{
		angle+=angleY/GRAD_PI;
		vector3df pt(center.X + diameter * cos(angle), center.Y, center.Z + diameter * sin(angle));
		return pt;
	}
	else
	{
		vector3df pt(	center.X + diameter * cos(angle) + diameter * cos(angleY), 
								center.Y + diameter * sin(angle), 
								center.Z  + diameter * sin(angleY));
		return pt;
	}
}

MineBoisage::MineBoisage(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, MineProfil * profil, MineBloc * pMineBloc, vector3df vecteurGalerie)
		: scene::ISceneNode(parent, mgr, id)
{
	m_profil=*profil;
	m_pMineBloc=pMineBloc;
	m_buffer=NULL;
	m_mesh=NULL;
	vecteurGalerie.Y=0;
	vecteurGalerie.normalize();
	vector3df verticale(0.0, 1.0, 0.0);
	vector3df vecteurNormal = vecteurGalerie.crossProduct(verticale);

	m_buffer = new SMeshBuffer();
	m_mesh = new SMesh();

	// base du bois gauche : 
	generateCylinder(10, m_profil.m_s1, m_profil.m_s4, m_buffer,0,false);
	generateCylinder(10, m_profil.m_s2, m_profil.m_s3, m_buffer,0,false);
	generatePoutre(3.0, m_profil.m_s4, m_profil.m_s3, m_buffer,vecteurGalerie);
/*
	// recalculate normals
	s32 n = (s32)m_buffer->Indices.size();
	for (s32 i=0; i<n-3; i+=3)
	{
		core::plane3d<f32> p(
			m_buffer->Vertices[m_buffer->Indices[i+0]].Pos,
			m_buffer->Vertices[m_buffer->Indices[i+1]].Pos,
			m_buffer->Vertices[m_buffer->Indices[i+2]].Pos);
		p.Normal.normalize();

		m_buffer->Vertices[m_buffer->Indices[i+0]].Normal = p.Normal;
		m_buffer->Vertices[m_buffer->Indices[i+1]].Normal = p.Normal;
		m_buffer->Vertices[m_buffer->Indices[i+2]].Normal = p.Normal;
	}
*/
	video::IVideoDriver* driver = g_data.driver;
	m_buffer->Material.Wireframe = false; //true;
	m_buffer->Material.BackfaceCulling = true;
	m_buffer->Material.Lighting = true;
	m_buffer->Material.EmissiveColor=SColor(255,0,0,0);
	m_buffer->Material.AmbientColor=SColor(255,255,255,255);
	m_buffer->Material.DiffuseColor=SColor(255,255,255,255);
	//m_buffer->Material.NormalizeNormals=true;
	m_buffer->Material.FogEnable=false;
	m_buffer->Material.MaterialType=EMT_SOLID;
	m_buffer->Material.Shininess=0.0f;
	m_buffer->Material.Texture1=	driver->getTexture("media/wood.jpg");
//	m_buffer->recalculateBoundingBox();
	setDebugDataVisible(false);

	m_mesh->addMeshBuffer(m_buffer);
//	m_mesh->recalculateBoundingBox();

	// creation de la bbox
	Box.reset(m_profil.m_s1);
	Box.addInternalPoint(m_profil.m_s2);
	Box.addInternalPoint(m_profil.m_s3);
	Box.addInternalPoint(m_profil.m_s4);
}

MineBoisage::~MineBoisage(void)
{
	m_buffer->drop();
	m_mesh->drop();
}


SMesh * MineBoisage::getMesh()
{
	return m_mesh;
}

void MineBoisage::OnPreRender()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnPreRender();
}

void MineBoisage::render()
{
	video::IVideoDriver* driver = g_data.driver;
	driver->setMaterial(m_buffer->Material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
	driver->drawIndexedTriangleList((S3DVertex *)m_buffer->getVertices(), (s32)m_buffer->getVertexCount(), (u16*)m_buffer->getIndices(), (s32)m_buffer->getIndexCount()/3);	

	if (DebugDataVisible)
	{
		video::SMaterial m;
		m.Lighting = false;
		driver->setMaterial(m);
		driver->draw3DBox(Box, video::SColor(0,255,255,255));
	}

}

const core::aabbox3d<f32>& MineBoisage::getBoundingBox() const
{
	return Box;
}

s32 MineBoisage::getMaterialCount()
{
	return 1;
}

video::SMaterial& MineBoisage::getMaterial(s32 i)
{
	return m_buffer->Material;
}	

MineBloc * MineBoisage::getMineBloc(void)
{
	return m_pMineBloc;
}

MineProfil * MineBoisage::getProfil()
{
	return &m_profil;
}

void MineBoisage::remove() 
{
	ISceneNode::remove();
}
