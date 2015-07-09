#include <stdlib.h>
#include <stdio.h>

#include <irrlicht.h>

#include "itineraire.h"
#include "LandManager.h"
#include "LandSceneNode.h"

#include <memory.h>

using namespace irr;

CItineraire::CItineraire(scene::ISceneNode* parent, scene::ISceneManager* smgr, s32 id)
	: scene::ISceneNode(parent, smgr, id)
{
	m_material.Wireframe = false;
	m_material.Lighting = false;
	// NULL object until init()
	m_nbVertices =0;
	m_vertices=NULL;
	m_indices=NULL;
}

CItineraire::~CItineraire()
{
	if( m_indices )
		free(m_indices);
	if( m_vertices)
		free(m_vertices);
}

//-------------------------------------------------------------------------------------------
// public methods
#define NBFACES 400
#define NBVERTICES (4 + NBFACES)
#define MUR_HEIGHT 30

void CItineraire::init(CTerrain * pTerrain)
{
	position2d<f32> itineraireControlPts[4]; //={ position2d<f32>(32142.0,30296.0), position2d<f32>(32842.0,30296.0), position2d<f32>(32842.0,30996.0), position2d<f32>(32142.0,30996.0) };
	FILE * fp;
	memset(itineraireControlPts,0, sizeof(itineraireControlPts));
	if( (fp=fopen("media/itineraire.txt","rb"))!=NULL)
	{
		int nbControlPts=0;
		int x,y;
		position2d<f32> position;
		while( ! feof(fp))
		{
			fscanf(fp,"%d,%d\n", &x, &y);
			position.X=x;
			position.Y=y;
			itineraireControlPts[nbControlPts++]=position;
		}
	}
	position2d<f32> itinerairePts[200];
	int nbOutPts = 200;
	overSampleItineraire(itineraireControlPts, 3, itinerairePts, nbOutPts);

	// --- alloc buffers for vertices and indices
	m_indices = (u16 *) malloc(3*NBFACES*sizeof(u16));	// taille maxi de la liste de faces
	if(m_indices==NULL ) 
		throw(1);

	m_vertices = (video::S3DVertex *) malloc(NBVERTICES*sizeof(video::S3DVertex));
	if(m_vertices==NULL)
		throw(2);

	// --- génère la forme du mur
	vector3df offset(itinerairePts[0].X, 1.0, itinerairePts[0].Y);
	vector3df normal(0.0, 0.0, 1.0);
	vector2d<f32> tcoord(0.0, 0.0);

	m_nbVertices = 0;
	m_nbIndices=0;

	m_vertices[m_nbVertices++] = video::S3DVertex(offset, normal, SColor(255,0,0,255), tcoord);
	offset.Y+=MUR_HEIGHT;
	m_vertices[m_nbVertices++] = video::S3DVertex(offset, normal, SColor(255,0,0,255), tcoord);
	
	for(int i=2; i<NBVERTICES-2; i+=2)
	{
		m_vertices[m_nbVertices++] = video::S3DVertex(offset.X, offset.Y + MUR_HEIGHT/2,offset.Z, 0,1,0, SColor(255,0,0,255), 0, 0);
		m_vertices[m_nbVertices++] = video::S3DVertex(offset.X, offset.Y - MUR_HEIGHT/2,offset.Z, 0,1,0, SColor(0,0,255,255), 0, 0);
		addFace(i-2, i, i+1);
		addFace(i-2, i+1, i-1);

		offset.X=	itinerairePts[i/2].X;
		offset.Z=itinerairePts[i/2].Y;
		offset.Y=pTerrain->getAltitude( (int)offset.X/XZSCALE+NBS/2, (int)offset.Z/XZSCALE+NBS/2) * YSCALE;
	}

	// --- ajuste la bounding box
	m_box.reset(m_vertices[0].Pos);
	for (s32 i=1; i<m_nbVertices; ++i)
		m_box.addInternalPoint(m_vertices[i].Pos);	
}

//-------------------------------------------------------------------------------------------
// private methods
void CItineraire::addFace(u16 p1, u16 p2, u16 p3)
{
	m_indices[m_nbIndices++]=p1;
	m_indices[m_nbIndices++]=p2;
	m_indices[m_nbIndices++]=p3;
}

// --- Bspline computation
// figure out the knots
void CItineraire::compute_intervals(int *u, int n, int t)
{
  int j;

  for (j=0; j<=n+t; j++)
  {
    if (j<t)
      u[j]=0;
    else
    if ((t<=j) && (j<=n))
      u[j]=j-t+1;
    else
    if (j>n)
      u[j]=n-t+2;  // if n-t=-2 then we're screwed, everything goes to 0
  }
}

// calculate the blending value
double CItineraire::blend(int k, int t, int *u, double v)
{
  double value;

  if (t==1)			// base case for the recursion
  {
    if ((u[k]<=v) && (v<u[k+1]))
      value=1;
    else
      value=0;
  }
  else
  {
    if ((u[k+t-1]==u[k]) && (u[k+t]==u[k+1]))  // check for divide by zero
      value = 0;
    else
    if (u[k+t-1]==u[k]) // if a term's denominator is zero,use just the other
      value = (u[k+t] - v) / (u[k+t] - u[k+1]) * blend(k+1, t-1, u, v);
    else
    if (u[k+t]==u[k+1])
      value = (v - u[k]) / (u[k+t-1] - u[k]) * blend(k, t-1, u, v);
    else
      value = (v - u[k]) / (u[k+t-1] - u[k]) * blend(k, t-1, u, v) +
	      (u[k+t] - v) / (u[k+t] - u[k+1]) * blend(k+1, t-1, u, v);
  }
  return value;
}

void CItineraire::compute_point(int *u, int n, int t, double v, position2d<f32> *control, position2d<f32> *output)
{
  int k;
  double temp;

  // initialize the variables that will hold our outputted point
  output->X=0;
  output->Y=0;

  for (k=0; k<=n; k++)
  {
    temp = blend(k,t,u,v);  // same blend is used for each dimension coordinate
    output->X = output->X + (control[k]).X * temp;
    output->Y = output->Y + (control[k]).Y * temp;
  }
}

// 
void CItineraire::bspline(int n, int t, position2d<f32> *control, position2d<f32> *output, int num_output)
{
  int *u;
  double increment,interval;
  position2d<f32> calcxyz;
  int output_index;

  u=new int[n+t+1];
  compute_intervals(u, n, t);

  increment=(double) (n-t+2)/(num_output-1);  // how much parameter goes up each time
  interval=0;

  for (output_index=0; output_index<num_output-1; output_index++)
  {
    compute_point(u, n, t, interval, control, &calcxyz);
    output[output_index].X = calcxyz.X;
    output[output_index].Y = calcxyz.Y;
    interval=interval+increment;  // increment our parameter
  }
  output[num_output-1].X=control[n].X;   // put in the last point
  output[num_output-1].Y=control[n].Y;
  delete u;
}

// ----------------------------------------------------------------------------------------------
void CItineraire::overSampleItineraire(position2d<f32>* itineraireControlPts, int itineraireControlNBPts, position2d<f32>* itinerairePts, int desiredNBPts )
{
	int m_t=4;           // degree of polynomial = t-1
	bspline(itineraireControlNBPts, m_t, itineraireControlPts, itinerairePts, desiredNBPts);
}
//-------------------------------------------------------------------------------------------
// base class overrides
void CItineraire::OnPreRender()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this);

	ISceneNode::OnPreRender();
}

void CItineraire::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	driver->setMaterial(m_material);
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

	driver->drawIndexedTriangleList(m_vertices, m_nbVertices, m_indices, m_nbIndices/3);
}

const core::aabbox3d<f32>& CItineraire::getBoundingBox() const
{
	return m_box;
}

s32 CItineraire::getMaterialCount()
{
	return 1;
}

video::SMaterial& CItineraire::getMaterial(s32 i)
{
	return m_material;
}