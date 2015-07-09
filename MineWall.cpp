#include "StdAfx.h"
#include <irrlicht.h>
#include ".\MineWall.h"
#include ".\MineBloc.h"
#include "GlobalData.h"

using namespace irr;
extern globalData g_data;

//--------------------------------------------------------------------------------------------------------------------------------------------------
// perlin noise domified

//--------------------------------------------------------------------------------------------------------------------------------------------------

MineWall::MineWall(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, MineProfil * profil, MineBloc * pMineBloc, WALL_ID wallId)
		: scene::ISceneNode(parent, mgr, id)
{
	// perlin noise init
	start=1;
	init();
	m_wallId=wallId;
	m_profil=*profil;
	m_pMineBloc=pMineBloc;
	m_buffer=NULL;
	m_mesh=NULL;
	m_bActiveShape=true;
	m_LOD=LOD_HIGH;

	computeWallGeometry();

	ITriangleSelector * selector = g_data.smgr->createOctTreeTriangleSelector(getMesh(), this, 32);
	setTriangleSelector(selector);
	g_data.m_metaSelector->addTriangleSelector(selector);
}

MineWall::~MineWall(void)
{
	m_buffer->drop();
	m_mesh->drop();
}

void MineWall::computeWallGeometry()
{
	f32 hillHeight=5.0f;
	const core::dimension2d<f32> textureRepeatCount = dimension2d<f32>(1.0f,1.0f);
	core::dimension2d<s32> tileCount = core::dimension2d<s32>(5,5);
	tileCount.Height += 1;
	tileCount.Width += 1;

	if( m_buffer)
	{
		m_buffer->Indices.clear();
		m_buffer->Vertices.clear();
		m_buffer->drop();
	}
	if( m_mesh)
	{
		m_mesh->drop();
	}
	m_buffer = new SMeshBuffer();
	m_mesh = new SMesh();
	video::S3DVertex vtx;
	vtx.Color.set(255,250,250,250);
	vtx.Normal.set(0,1,0);

	// set shape
	vector3df abs = m_pMineBloc->getBlocAbsoluteVector();

	m_shapeTriangle[0].pointA=m_profil.m_s1;
	m_shapeTriangle[0].pointB=m_profil.m_s2;
	m_shapeTriangle[0].pointC=m_profil.m_s3;
	m_shapeTriangle[1].pointA=m_profil.m_s1;
	m_shapeTriangle[1].pointB=m_profil.m_s3;
	m_shapeTriangle[1].pointC=m_profil.m_s4;

	// create vertices
	s32 x = 0;
	s32 y = 0;

	core::dimension2d<f32> tx;
	tx.Width = 1.0f / (tileCount.Width / textureRepeatCount.Width);
	tx.Height = 1.0f / (tileCount.Height / textureRepeatCount.Height);

	vector3df s14,s23,s1423;
	vector3df ds14,ds23,ds1423;
	vector3df deltaX, deltaY;

	s14=m_profil.m_s1;
	s23=m_profil.m_s2;
	ds14=(m_profil.m_s4 - m_profil.m_s1)/(f32)(tileCount.Height-1);
	ds23=(m_profil.m_s3 - m_profil.m_s2)/(f32)(tileCount.Height-1);

	float noise, vec[3];
	for (y=0; y<tileCount.Height; ++y)
	{
		s1423=s14;
		ds1423=(s23-s14)/(f32)(tileCount.Width-1);

		for (x=0; x<tileCount.Width; ++x)
		{
			vtx.Pos.set(s1423);
			vtx.TCoords.set(-(f32)x * tx.Width, (f32)y * tx.Height);

			// gaffe au sens de l'évaluation des params :-/
			vec[0]=vtx.Pos.X+abs.X;
			vec[1]=vtx.Pos.Y+abs.Y;
			vec[2]=vtx.Pos.Z+abs.Z;

			if( m_wallId != WALL_ID_RADIER || x==0 || x==tileCount.Width-1 || y==0 || y==tileCount.Height-1)
				noise=noise3(vec)*6.0f;
			else
				noise=noise3(vec)*2.0f;

			vtx.Pos.X += noise;
			vtx.Pos.Y += noise;
			vtx.Pos.Z += noise;

			m_buffer->Vertices.push_back(vtx);
			s1423 += ds1423;
		}
		s14+=ds14;
		s23+=ds23;
	}

	// create indices
	for (x=0; x<tileCount.Width-1; ++x)
		for (y=0; y<tileCount.Height-1; ++y)
		{
			s32 current = y*tileCount.Width + x;

			m_buffer->Indices.push_back(current);
			m_buffer->Indices.push_back(current + tileCount.Width);
			m_buffer->Indices.push_back(current + 1);

			m_buffer->Indices.push_back(current + 1);
			m_buffer->Indices.push_back(current + tileCount.Width);
			m_buffer->Indices.push_back(current + 1 + tileCount.Width);
		}

	// recalculate normals
	int n=(s32)m_buffer->Indices.size();
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

	video::IVideoDriver* driver = g_data.driver;
	m_buffer->Material.Wireframe = false; //true;
	m_buffer->Material.BackfaceCulling = true;
	m_buffer->Material.Lighting = true;
	m_buffer->Material.AmbientColor=SColor(255,128,128,128);
	m_buffer->Material.DiffuseColor=SColor(255,128,128,128);
	m_buffer->Material.FogEnable=false;
	m_buffer->Material.MaterialType=EMT_SOLID;
	m_buffer->Material.Shininess=0.0f;
	if( m_wallId == WALL_ID_RADIER)
		m_buffer->Material.Texture1=	driver->getTexture("media/radier1.jpg");
	else
		m_buffer->Material.Texture1=	driver->getTexture("media/rock.jpg");
	m_buffer->recalculateBoundingBox();
	setDebugDataVisible(false);

	m_mesh->addMeshBuffer(m_buffer);
	m_mesh->recalculateBoundingBox();

	// creation du meshe lod low poly
	m_lowResMeshIndices[0]=1;
	m_lowResMeshIndices[1]=0;
	m_lowResMeshIndices[2]=3;
	m_lowResMeshIndices[3]=2;
	m_lowResMeshIndices[4]=1;
	m_lowResMeshIndices[5]=3;
	vtx.Pos=m_profil.m_s1;
	vtx.TCoords=vector2d<f32>(0.0f,1.0f);
	m_lowResMeshVertices[0]=vtx;
	vtx.Pos=m_profil.m_s2;
	vtx.TCoords=vector2d<f32>(1.0f,1.0f);
	m_lowResMeshVertices[1]=vtx;
	vtx.Pos=m_profil.m_s3;
	vtx.TCoords=vector2d<f32>(1.0f,0.0f);
	m_lowResMeshVertices[2]=vtx;
	vtx.Pos=m_profil.m_s4;
	vtx.TCoords=vector2d<f32>(0.0f,0.0f);
	m_lowResMeshVertices[3]=vtx;

	// creation de la bbox
	Box.reset(m_profil.m_s1);
	Box.addInternalPoint(m_profil.m_s2);
	Box.addInternalPoint(m_profil.m_s3);
	Box.addInternalPoint(m_profil.m_s4);
}

// revoie true si le segment de ligne donné en paramètre intersecte le mur
// le point d'intersection est renvoyé dans *pIntersectionPoint
// onlyActiveWalls détermine si il faut prendre en compte les minewalls dejà descendus, typiquement, selon
// qu'on veuile le descendre à nouveau ou tester une collision (true); ou selon qu'on veuille tester si on est
// d'un coté ou l'autre du mur (false)
bool MineWall::intersects(line3d<f32>* pLine, vector3df * pIntersectionPoint, bool onlyActiveWalls)
{
	vector3d<f32>  outIntersection;
	if( onlyActiveWalls && ! m_bActiveShape)
		return false;
	bool r1,r2;
	r1=m_shapeTriangle[0].getIntersectionWithLimitedLine (*pLine, outIntersection);
	if( ! r1 )
		r2=m_shapeTriangle[1].getIntersectionWithLimitedLine (*pLine, outIntersection);
	if( r1 || r2)
	{
		if( pIntersectionPoint )
			*pIntersectionPoint=outIntersection;
		return true;
	}
	return false;
}

SMesh * MineWall::getMesh()
{
	return m_mesh;
}
void MineWall::OnPreRender()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnPreRender();
}

void MineWall::render()
{
	video::IVideoDriver* driver = g_data.driver;
	driver->setMaterial(m_buffer->Material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	if( m_LOD==LOD_HIGH || m_LOD==LOD_JOINT_LOWHIGH)
		driver->drawIndexedTriangleList((S3DVertex *)m_buffer->getVertices(), (s32)m_buffer->getVertexCount(), (u16*)m_buffer->getIndices(), (s32)m_buffer->getIndexCount()/3);	
	if( m_LOD==LOD_LOW || m_LOD==LOD_JOINT_LOWHIGH)
		driver->drawIndexedTriangleList(m_lowResMeshVertices, 4, m_lowResMeshIndices, 2);

	if (DebugDataVisible)
	{
		video::SMaterial m;
		m.Lighting = false;
		driver->setMaterial(m);
		driver->draw3DBox(Box, video::SColor(0,255,255,255));

		driver->draw3DTriangle(m_shapeTriangle[0], video::SColor(0,255,255,0));
		driver->draw3DTriangle(m_shapeTriangle[1], video::SColor(0,255,255,0));
	}

}

const core::aabbox3d<f32>& MineWall::getBoundingBox() const
{
	return Box;
}

s32 MineWall::getMaterialCount()
{
	return 1;
}

video::SMaterial& MineWall::getMaterial(s32 i)
{
	return m_buffer->Material;
}	

MineBloc * MineWall::getMineBloc(void)
{
	return m_pMineBloc;
}

MineProfil * MineWall::getProfil()
{
	return &m_profil;
}

// change le profil d'un MineWall. Pour s'en servir, mieux vaut passer par MineBloc::setprofil qui impacte aussi les autres wall attachés
void MineWall::setProfil(MineProfil * pProfil)
{
	m_profil=*pProfil;
	g_data.m_metaSelector->removeTriangleSelector(getTriangleSelector());
	computeWallGeometry();
	ITriangleSelector * selector = g_data.smgr->createOctTreeTriangleSelector(getMesh(), this, 32);
	setTriangleSelector(selector);
	g_data.m_metaSelector->addTriangleSelector(selector);
}

//---------------------------------
/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */
double MineWall::noise1(double arg)
{
	int bx0, bx1;
	float rx0, rx1, sx, t, u, v, vec[1];
	vec[0] = (float)arg;	// optimisation le cast est pas gratuit! à virer 

	setup(0, bx0,bx1, rx0,rx1);

	sx = s_curve(rx0);

	u = rx0 * g1[ p[ bx0 ] ];
	v = rx1 * g1[ p[ bx1 ] ];

	return lerp(sx, u, v);
}
/*
void MineWall::reset(void)
{
	start=1;
	init();
}
*/
float MineWall::noise2(float vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

float MineWall::noise3(float vec[3])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

	#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);

	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);

	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}

void MineWall::normalize2(float v[2])
{
	float s;

	s = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

void MineWall::normalize3(float v[3])
{
	float s;

	s = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

void MineWall::mySRand(unsigned int seed)
{
	m_randSeed=seed;
}

inline int MineWall::myRand()
{
	// -- dom random
	//return (m_randSeed = (1531664525L * m_randSeed + 1013904223L) % RAND_MAX);

	// Mr bee's random
	return (m_randSeed = 1664525L * m_randSeed + 1013904223L) >> 5;

	/* -- Pierre Larbier's random --
	long haut , bas , inter , q;
	bas = 16807 * (m_randSeed & 0xffffL);
	haut = 16807 * (m_randSeed >> 16);
	inter = (bas >> 16) + haut;
	bas = ((bas & 0xffff) | ((inter & 0x7fff)<<16)) + (inter >> 15);
	if ((bas & 0x80000000L) != 0)
		bas = (bas + 1) & 0x7fffffffL;
	m_randSeed = bas;
	return bas;
	*/

	//-- msvc++ random
	//return rand();
}

void  MineWall::init(void)
{
	int i, j, k;

	for (i = 0 ; i < BB ; i++) {
		p[i] = i;
		g1[i] = (float)((myRand() % (BB + BB)) - BB) / BB;
		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((myRand() % (BB + BB)) - BB) / BB;

		normalize2(g2[i]);

		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((myRand() % (BB + BB)) - BB) / BB;
		normalize3(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = myRand() % BB];
		p[j] = k;
	}

	for (i = 0 ; i < BB + 2 ; i++) {
		p[BB + i] = p[i];
		g1[BB + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[BB + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[BB + i][j] = g3[i][j];
	}
}

WALL_ID MineWall::getWallId()
{
	return m_wallId;
}

void MineWall::remove() 
{
	ISceneNode::remove();
	ITriangleSelector * mTS = getTriangleSelector();
	g_data.m_metaSelector->removeTriangleSelector(mTS);
	m_bActiveShape=false;
}

bool MineWall::isActive()
{
	return m_bActiveShape;
}