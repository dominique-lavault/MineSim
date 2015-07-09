#include <stdlib.h>
#include <stdio.h>

#include <irrlicht.h>

#include "Etiquette.h"
#include "LandManager.h"

using namespace irr;

#define NBETIQUETTEFACES 5
#define NBETIQUETTEVERTICES 7

CEtiquette::CEtiquette(scene::ISceneNode* parent, scene::ISceneManager* smgr, s32 id)
	: scene::ISceneNode(parent, smgr, id)
{
	Material.Wireframe = true;
	Material.Lighting = false;

	m_indices = (u16 *) malloc(3*NBETIQUETTEFACES*sizeof(u16));	// taille maxi de la liste de faces
	if(m_indices==NULL ) throw(1);

	Vertices = (video::S3DVertex *) malloc(NBETIQUETTEVERTICES*sizeof(video::S3DVertex));
	if(Vertices==NULL)	 throw(2);

	// --- génère la forme de la flèche
	m_nbVertices = 0;
	Vertices[m_nbVertices++] = video::S3DVertex(0,0,0, 0,0,1, SColor(255,255,0,0), 0, 0);
	Vertices[m_nbVertices++] = video::S3DVertex(0,-20,0,	0,0,1, SColor(255,255,255,255), 0, 0);
	Vertices[m_nbVertices++] = video::S3DVertex(0,-30,0, 0,0,1, SColor(255,255,0,0), 0, 0);
	Vertices[m_nbVertices++] = video::S3DVertex(0,-20,10, 0,0,1, SColor(255,255,0,0), 0, 0);
	Vertices[m_nbVertices++] = video::S3DVertex(10,-20,0, 0,0,1, SColor(255,255,0,0), 0, 0);
	Vertices[m_nbVertices++] = video::S3DVertex(0,-20,-10, 0,0,1, SColor(255,255,0,0), 0, 0);
	Vertices[m_nbVertices++] = video::S3DVertex(-10,-20,0, 0,0,1, SColor(255,255,0,0), 0, 0);

	m_nbIndices=0;
	addFace(0,0,1);
	addFace(1,2,3);
	addFace(1,2,4);
	addFace(1,2,5);
	addFace(1,2,6);

	// --- ajuste la bounding box
	Box.reset(Vertices[0].Pos);
	for (s32 i=1; i<m_nbVertices; ++i)
		Box.addInternalPoint(Vertices[i].Pos);	

	switchOff();
}

void CEtiquette::addFace(u16 p1, u16 p2, u16 p3)
{
	m_indices[m_nbIndices++]=p1;
	m_indices[m_nbIndices++]=p2;
	m_indices[m_nbIndices++]=p3;
}

CEtiquette::~CEtiquette()
{
	free(m_indices);
	free(Vertices);
}

//-------------------------------------------------------------------------------------------
size_t CEtiquette::loadFile(const char * filename, void * buf, size_t size)
{
	FILE * fp;
	if( (fp=fopen(filename, "rb"))==NULL) 
		return 0;
	size = fread(buf,1,size,fp);
	fclose(fp);
	return size;
}

void CEtiquette::OnPreRender()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnPreRender();
}

void CEtiquette::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	driver->setMaterial(Material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	driver->drawIndexedTriangleList(Vertices, m_nbVertices, m_indices, m_nbIndices/3);
}

const core::aabbox3d<f32>& CEtiquette::getBoundingBox() const
{
	return Box;
}

s32 CEtiquette::getMaterialCount()
{
	return 1;
}

video::SMaterial& CEtiquette::getMaterial(s32 i)
{
	return Material;
}	

void CEtiquette::setOffset(vector3df * pOffset)
{
	m_offset = *pOffset;
	setPosition(m_offset);
}

void CEtiquette::switchOn()
{
	m_status=ON;
	m_updateStatus=UPDATING;
	setVisible(true);
}

void CEtiquette::switchOff()
{
	m_status=OFF;
	m_updateStatus=UPDATING;
	setVisible(false);
}

CEtiquette::ETIQUETTE_STATUS CEtiquette::getStatus()
{
	return m_status;
}

CEtiquette::ETIQUETTE_UPDATE CEtiquette::getUpdateStatus()
{
	return m_updateStatus;
}

void CEtiquette::setUpdateStatus(CEtiquette::ETIQUETTE_UPDATE updateStatus)
{
	m_updateStatus = updateStatus;
}

const wchar_t * CEtiquette::getLibelleType()
{
	const wchar_t *wType;
	switch( m_type )
	{
	case 'P' :
		wType=L"Ville ou Lieu habité";
		break;
	case  'T' :
		wType=L"Relief (Lieu Hypsographique)";
		break;
	case  'V' :
		wType=L"Végétation";
		break;
	case  'H' :
		wType=L"Elément hydrographique";
		break;
	case  'S' :
		wType=L"Lieu remarquable";
		break;
	case  'R' :
		wType=L"Constructions";
		break;
	case  'L' :
		wType=L"Localité";
		break;
	case  'A' :
		wType=L"Région Administrative";
		break;
	default:
		wType=L"?";
	}
	return wType;
}