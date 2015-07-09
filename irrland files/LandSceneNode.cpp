#include <stdlib.h>
#include <stdio.h>

#include <irrlicht.h>

#include "LandSceneNode.h"
#include "LandManager.h"

using namespace irr;

CLandSceneNode::CLandSceneNode(scene::ISceneNode* parent, scene::ISceneManager* smgr, s32 id)
	: scene::ISceneNode(parent, smgr, id)
{
	Material.Wireframe = false	; // relief
	Material.Lighting = false;
	m_mipmapLevel=0;
	m_deltaAltitude=NULL;

	u8 tmpReliefColor[3][256];
	loadFile("media\\coul.raw", tmpReliefColor, 768);
	for(int i=0; i<256;i++)	
	{
		heightColors[i] = video::SColor(255,tmpReliefColor[0][i], tmpReliefColor[1][i], tmpReliefColor[2][i] );
	}

	m_indices = (u16 *) malloc(3*NBBLOCKFACES*sizeof(u16));	// taille maxi de la liste de faces
	if(m_indices==NULL ) throw(1);

	Vertices = (video::S3DVertex *) malloc(NBBLOCKVERTICES*sizeof(video::S3DVertex));
	if(Vertices==NULL)	 throw(2);
}



CLandSceneNode::~CLandSceneNode()
{
	free(m_indices);
	free(Vertices);
	if( m_deltaAltitude )
		free(m_deltaAltitude);
}



// ________________________________ génération du meshe du terrain ________________________________________________
void CLandSceneNode::generateMesh( CLandManager * pLandManager, vector3df * cameraPosition)
{
	int u,v;
	float x0,y0,z0,ySouth,yNorth,yWest,yEast;
	irr::f32 tu,tv,iv,iu;

	// texture du sol
	irr::f32 steptu = (irr::f32)0.25/2.0; // 4 polys pour warper dans la texture
	irr::f32 steptv = steptu;
	
	int i=0;
	int nv=0,idx;
	video::SColor c;
	float pente;

	// NBS = NB BLOCK SAMPLES = nombre de vertices sur un coté du block
	for(tu=0, u=0; u<NBS; u++, tu+=steptu)
	{	
		for(tv=0, v=0; v<NBS; v++,  tv+=steptv)
		{
			x0=v*XZSCALE + m_offset.X;
			z0=u*XZSCALE + m_offset.Z;
			y0=pLandManager->getAltitude(x0,z0);
			ySouth=pLandManager->getAltitude(x0,z0+XZSCALE); // optimiser en mémorisant la valeur précédente si u!=0 && v!=0
			yNorth=pLandManager->getAltitude(x0,z0-XZSCALE); // optimiser en mémorisant la valeur précédente si u!=0 && v!=0
			yEast=pLandManager->getAltitude(x0+XZSCALE,z0); // optimiser en mémorisant la valeur précédente si u!=0 && v!=0
			yWest=pLandManager->getAltitude(x0-XZSCALE,z0); //optimiser en mémorisant la valeur précédente si u!=0 && v!=0
			
			idx=(int)MAX(MIN(y0*ALTITUDE_COLORTABLE_RATIO,255),0);
			c=heightColors[idx];

			//tu=((f32)(rand()%1024))/2048.0f;
			//tv=((f32)(rand()%1024))/2048.0f;
			if( tu>0.24f )
				tu-=0.20f;
			if( tv>0.24f )
				tv-=0.20f;
			iu=0.5f;
			iv=0.0f;
			if( ySouth>2500)
				iv=0.75f;	// pierre/neige
			else if( ySouth>200)
				iu=0.75f;	// long grass

			if( (ySouth-yNorth)<-CLIFF_STEEP || (yWest-yEast)<-CLIFF_STEEP )
			{
				pente=0.7f;
				iv=0.25f;
				iu=0.75f;	// falaise
				c=CLIFF_COLOR;
			}
			else if( (yWest-yEast)<-10.0f )
				pente=0.7f;
			else if( (yWest-yEast)<-5.0f )
				pente=0.9f;
			else if( (ySouth-yNorth)>CLIFF_STEEP  || (yWest-yEast)>CLIFF_STEEP)
			{
				pente=1.3f;
				iv=0.25f;
				iu=0.75f;	// falaise
				c=0xffd0d0d0;
			}
			else if( (yWest-yEast)>10.0f )
				pente=1.1f;
			else if( (yWest-yEast)>5.0f )
				pente=1.1f;
			else
				pente=1.0f;

			c.setRed(MIN(MAX((int)((float)c.getRed()*pente), 0), 255)); 
			c.setGreen(MIN(MAX((int)((float)c.getGreen() *pente), 0), 255)); 
			c.setBlue(MIN(MAX((int)((float)c.getBlue() *((pente+1.0)/2.0)), 0), 255)); 

			if( (y0 + yWest + yEast + yNorth + ySouth)/5.0f <1.0f )
			{
				iv=0.0f;
				iu=0.0f;	// mer
				if( c!=CLIFF_COLOR)
					c=SEA_COLOR;
			}

			if(m_deltaAltitude)
				y0+=m_deltaAltitude[nv];
			Vertices[nv++] = video::S3DVertex(x0,y0,z0, 0,1,0, c, tu+iu, tv+iv);
		}
	}

	int levelN=pLandManager->getMipMapLevel(cameraPosition, m_offset.X+NBS*XZSCALE/2, m_offset.Z-NBS*XZSCALE/2);
	int levelE=pLandManager->getMipMapLevel(cameraPosition, m_offset.X+3*NBS*XZSCALE/2, m_offset.Z+NBS*XZSCALE/2);
	int levelO=pLandManager->getMipMapLevel(cameraPosition, m_offset.X-NBS*XZSCALE/2, m_offset.Z+NBS*XZSCALE/2);
	int levelS=pLandManager->getMipMapLevel(cameraPosition, m_offset.X+NBS*XZSCALE/2, m_offset.Z+3*NBS*XZSCALE/2);
	updateTesslation(pLandManager, cameraPosition, levelN, levelS, levelO, levelE);

	// --- ajuste la bounding box
	// todo : optimisation : ne mettre que les points extremes (4 coins, le + haut, le + bas)
	Box.reset(Vertices[0].Pos);
	for (s32 i=1; i<NBBLOCKVERTICES; ++i)
		Box.addInternalPoint(Vertices[i].Pos);	
}

float CLandSceneNode::getDeltaAltitude(int idx)
{
	if( m_deltaAltitude != NULL)
		return m_deltaAltitude[idx];
	else
		return 0;
}

void CLandSceneNode::setDeltaAltitude(int idx, float value)
{
	if( m_deltaAltitude==NULL)
	{
		m_deltaAltitude=(float*)malloc(NBBLOCKVERTICES * sizeof(float));
		for(int a=0; a<NBBLOCKVERTICES; a++)	// optimisation : voir pour un memset
			m_deltaAltitude[a]=0;
	}
	m_deltaAltitude[idx]=value;

}

void CLandSceneNode::updateTesslation( CLandManager * pLandManager, vector3df * cameraPosition, int levelN, int levelS, int levelO, int levelE)
{
	int u,v,i=0;
	// level mipmap des voisins?
	int level = m_mipmapLevel;

	m_doU0=(levelO<level);
	m_doUM=(levelE<level);
	m_doV0=(levelN<level);
	m_doVM=(levelS<level);

	// génère la liste des faces en fonction de la liste de vertices
	m_nbFaceLevel=0;

	if( level==0 )
	{
		for(u=0; u<NBS-1; u+=1)
		{	
			for(v=0; v<NBS-1; v+=1)
			{
				// face a
				m_indices[i++]=NBS*v+u;
				m_indices[i++]=NBS*(v+1)+u;
				m_indices[i++]=NBS*v+u+1;

				// face b
				m_indices[i++]=NBS*v+u+1;
				m_indices[i++]=NBS*(v+1)+u;
				m_indices[i++]=NBS*(v+1)+u+1;
				m_nbFaceLevel+=2;
			}
		}
	}
	else if( level==1 )
	{
		// mipmap level 1
		tesslateLevel(&m_nbFaceLevel, 2, m_indices);
	}
	else if (level==2 )
	{
		// --- mipmap level 2
		tesslateLevel(&m_nbFaceLevel, 4, m_indices);
	}
	else if (level==3 )
	{
		// --- mipmap level 3
		tesslateLevel(&m_nbFaceLevel, 8, m_indices);
	}
}

void CLandSceneNode::tesslateLevel(int * nbFaceLevel, int level, u16 * indices)
{
	int i=0,u,v;
	int halfLevel=level>>1;
	int limit = NBS-level-1;

	for(u=0; u<NBS-level; u+=level)
	{	
		for(v=0; v<NBS-level; v+=level)
		{
			// face 'a' (triangle haut gauche)
			if( u!=0 && v!=0)
			{
				i=tesslateFaceALevel(u, v, i, level, indices,nbFaceLevel);
			}
			else if( u!=0 && v==0)
			{
				if( m_doV0 )
				{
					indices[i++]=NBS*v+u;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+halfLevel;

					indices[i++]=NBS*v+u+halfLevel;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+level;

					*nbFaceLevel+=2;
				}
				else
					i=tesslateFaceALevel(u, v, i, level, indices, nbFaceLevel);
			}
			else if( u==0 && v!=0)
			{
				if( m_doU0 )
				{
					indices[i++]=NBS*v+u;
					indices[i++]=NBS*(v+halfLevel)+u;
					indices[i++]=NBS*v+u+level;

					indices[i++]=NBS*(v+halfLevel)+u;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+level;

					*nbFaceLevel+=2;
				}
				else
					i=tesslateFaceALevel(u, v, i, level, indices, nbFaceLevel);
			}
			else if( u==0 && v==0 )
			{
				if( m_doU0 && m_doV0 )
				{
					indices[i++]=NBS*v+u;
					indices[i++]=NBS*(v+halfLevel)+u;
					indices[i++]=NBS*v+u+halfLevel;

					indices[i++]=NBS*v+u+halfLevel;
					indices[i++]=NBS*(v+halfLevel)+u;
					indices[i++]=NBS*(v+level)+u;

					indices[i++]=NBS*v+u+halfLevel;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+level;

					*nbFaceLevel+=3;
				}
				else	if( m_doU0 && !m_doV0 )	// surtesslation metamerdique partielle d'un angle = 
				{
					indices[i++]=NBS*v+u;
					indices[i++]=NBS*(v+halfLevel)+u;
					indices[i++]=NBS*v+u+level;

					indices[i++]=NBS*(v+halfLevel)+u;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+level;

					*nbFaceLevel+=2;
				}
				else	if( ! m_doU0 && m_doV0 )	// surtesslation metamerdique partielle d'un angle
				{
					indices[i++]=NBS*v+u;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+halfLevel;

					indices[i++]=NBS*v+u+halfLevel;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*v+u+level;

					*nbFaceLevel+=2;
				}
				else
					i=tesslateFaceALevel(u, v, i, level, indices, nbFaceLevel);
			}

			// face 'b' (triangle bas droit)
			if( u!=limit && v!=limit)
			{
				i=tesslateFaceBLevel(u,v,i,level, indices, nbFaceLevel);
			}
			else 	if( u==limit && v!=limit)
			{
				if( m_doUM )
				{
					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+halfLevel)+u+level;

					indices[i++]=NBS*(v+halfLevel)+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+level)+u+level;
					*nbFaceLevel+=2;
				}
				else
					i=tesslateFaceBLevel(u,v,i,level, indices, nbFaceLevel);
			}
			else if( u!=limit && v==limit)
			{
				if( m_doVM )
				{
					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+level)+u+halfLevel;

					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u+halfLevel;
					indices[i++]=NBS*(v+level)+u+level;
					*nbFaceLevel+=2;
				}
				else
					i=tesslateFaceBLevel(u,v,i,level, indices, nbFaceLevel);
			}
			else 	if( u==limit && v==limit)
			{
				if( m_doUM && m_doVM )
				{
					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+halfLevel)+u+level;

					indices[i++]=NBS*(v+halfLevel)+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+level)+u+halfLevel;

					indices[i++]=NBS*(v+halfLevel)+u+level;
					indices[i++]=NBS*(v+level)+u+halfLevel;
					indices[i++]=NBS*(v+level)+u+level;
					*nbFaceLevel+=3;
				}
				else 	if( m_doUM && !m_doVM )
				{
					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+halfLevel)+u+level;

					indices[i++]=NBS*(v+halfLevel)+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+level)+u+level;
					*nbFaceLevel+=2;
				}
				else 	if( ! m_doUM && m_doVM )
				{
					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u;
					indices[i++]=NBS*(v+level)+u+halfLevel;

					indices[i++]=NBS*v+u+level;
					indices[i++]=NBS*(v+level)+u+halfLevel;
					indices[i++]=NBS*(v+level)+u+level;
					*nbFaceLevel+=2;
				}
				else
					i=tesslateFaceBLevel(u,v,i,level, indices, nbFaceLevel);
			}
		
		}
	}
}

int CLandSceneNode::tesslateFaceALevel(int u, int v, int i, int level, u16 * m_indices, int * m_nbFaceLevel)
{
	m_indices[i++]=NBS*v+u;
	m_indices[i++]=NBS*(v+level)+u;
	m_indices[i++]=NBS*v+u+level;
	*m_nbFaceLevel+=1;
	return i;
}

int CLandSceneNode::tesslateFaceBLevel(int u, int v, int i, int level, u16 * m_indices, int * m_nbFaceLevel)
{
	m_indices[i++]=NBS*v+u+level;
	m_indices[i++]=NBS*(v+level)+u;
	m_indices[i++]=NBS*(v+level)+u+level;
	*m_nbFaceLevel+=1;
	return i;
}

//-------------------------------------------------------------------------------------------
size_t CLandSceneNode::loadFile(const char * filename, void * buf, size_t size)
{
	FILE * fp;
	if( (fp=fopen(filename, "rb"))==NULL) 
		return 0;
	size = fread(buf,1,size,fp);
	fclose(fp);
	return size;
}

void CLandSceneNode::OnPreRender()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnPreRender();
}

void CLandSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	driver->setMaterial(Material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	driver->drawIndexedTriangleList(Vertices, NBBLOCKVERTICES, m_indices, m_nbFaceLevel);
}

const core::aabbox3d<f32>& CLandSceneNode::getBoundingBox() const
{
	return Box;
}

s32 CLandSceneNode::getMaterialCount()
{
	return 1;
}

video::SMaterial& CLandSceneNode::getMaterial(s32 i)
{
	return Material;
}	

void CLandSceneNode::setOffset(vector3df * pOffset)
{
	m_offset = *pOffset;
}