#pragma once

#include <irrlicht.h>
#include "Terrain.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


class CItineraire : public scene::ISceneNode
{
public:
	core::aabbox3d<f32> m_box;
	video::S3DVertex * m_vertices;
	u16 * m_indices;
	video::SMaterial m_material;

	CItineraire::CItineraire(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
	CItineraire::~CItineraire();

	void CItineraire::init(CTerrain * pTerrain);
	
protected:
	int m_nbVertices;
	int m_nbIndices;

	void CItineraire::addFace(u16 p1, u16 p2, u16 p3);

	void CItineraire::compute_intervals(int *u, int n, int t);
	double CItineraire::blend(int k, int t, int *u, double v);
	void CItineraire::compute_point(int *u, int n, int t, double v, position2d<f32> *control, position2d<f32> *output);
	void CItineraire::bspline(int n, int t, position2d<f32> *control, position2d<f32> *output, int num_output);
	void CItineraire::overSampleItineraire(position2d<f32>* itineraireControlPts, int itineraireControlNBPts, position2d<f32> * itinerairePts, int desiredNBPts);

	// ISceneNode virtuals overload
	virtual video::SMaterial& CItineraire::getMaterial(s32 i);
	virtual s32 CItineraire::getMaterialCount();
	virtual const core::aabbox3d<f32>& CItineraire::getBoundingBox() const;
	virtual void CItineraire::render();
	virtual void CItineraire::OnPreRender();
};
