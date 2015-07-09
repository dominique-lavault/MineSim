#include <irrlicht.h>
#include "globalData.h"

using namespace irr;
extern globalData g_data;
/*

		billboard avec texture animée	

*/
class BillBoardSceneNode : public scene::ISceneNode
{
	core::aabbox3d<f32> Box;
	video::S3DVertex Vertices[4];
	video::SMaterial Material;
	ICameraSceneNode * m_pCamera;
	dimension2d<f32> Size;
	int tick;

public:

	BillBoardSceneNode(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, ICameraSceneNode * pCamera)
		: scene::ISceneNode(parent, mgr, id)
	{
		Material.Wireframe = false;
		Material.Lighting = false;
		tick=0;

		m_pCamera=pCamera;
		Size.Width=10.0f;
		Size.Height=10.0f;

		Vertices[0] = video::S3DVertex(-10,-10,0, 0,0,-1, video::SColor(255,255,255,255), 0, 0.25f);
		Vertices[1] = video::S3DVertex(10,-10,0, 0,0,-1, video::SColor(255,255,255,255), 0.25f, 0.25f);
		Vertices[2] = video::S3DVertex(10,10,0, 0,0,-1, video::SColor(255,255,255,255), 0.25f, 0);
		Vertices[3] = video::S3DVertex(-10,10,0, 0,0,-1, video::SColor(255,255,255,255), 0, 0);

		Box.reset(Vertices[0].Pos);
		for (s32 i=1; i<4; ++i)
			Box.addInternalPoint(Vertices[i].Pos);
	}

	virtual void OnPreRender()
	{
		if (IsVisible)
			SceneManager->registerNodeForRendering(this);

		ISceneNode::OnPreRender();
	}

	void resetAnim()
	{
		tick=0;
	}

	virtual void render()
	{
		tick++;
		int localTick=tick/10;
		if( localTick < 16 )
		{
			int tcoordx=localTick%4;
			int tcoordy=localTick>>2;
			f32 tx=(f32)tcoordx/4.0f;
			f32 ty=(f32)tcoordy/4.0f;
			Vertices[0].TCoords = vector2d<f32> (tx, ty+0.25f);
			Vertices[1].TCoords = vector2d<f32> (tx+0.25f, ty+0.25f);
			Vertices[2].TCoords = vector2d<f32> (tx+0.25f, ty);
			Vertices[3].TCoords = vector2d<f32> (tx, ty);
		}
		else
			return;

		// orienter le billboard vers la camera
		core::vector3df pos = getAbsolutePosition();
		core::vector3df campos = m_pCamera->getAbsolutePosition();
		core::vector3df target = m_pCamera->getTarget();
		core::vector3df up = m_pCamera->getUpVector();
		core::vector3df view = target - campos;
		view.normalize();

		core::vector3df horizontal = up.crossProduct(view);
		horizontal.normalize();

		core::vector3df vertical = horizontal.crossProduct(view);
		vertical.normalize();

		horizontal *= 0.5f * Size.Width;
		vertical *= 0.5f * Size.Height;	

		Vertices[0].Pos =  - horizontal - vertical;
		Vertices[1].Pos =  horizontal - vertical;
		Vertices[2].Pos =  horizontal + vertical;
		Vertices[3].Pos =  - horizontal + vertical;

		view *= -1.0f;

		for (s32 i=0; i<4; ++i)
			Vertices[i].Normal = view;

		Box.reset(Vertices[0].Pos);
		for (s32 i=1; i<4; ++i)
			Box.addInternalPoint(Vertices[i].Pos);


		// draw
		if (DebugDataVisible)
		{
			g_data.driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
			video::SMaterial m;
			m.Lighting = false;
			g_data.driver->setMaterial(m);
			g_data.driver->draw3DBox(Box, video::SColor(0,208,195,152));
		}

		core::matrix4 mat;
		g_data.driver->setTransform(video::ETS_WORLD, mat);
		g_data.driver->setMaterial(Material);

		// rendre le scenenode
		u16 indices[] = {	0,1,2, 0,2,3 };
		g_data.driver->setMaterial(Material);
		g_data.driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		g_data.driver->drawIndexedTriangleList(&Vertices[0], 4, &indices[0], 2);
	}

	virtual const core::aabbox3d<f32>& getBoundingBox() const
	{
		return Box;
	}

	virtual s32 getMaterialCount()
	{
		return 1;
	}

	virtual video::SMaterial& getMaterial(s32 i)
	{
		return Material;
	}	
	
	void setSize(const core::dimension2d<f32>& size)
	{
		Size = size;

		if (Size.Width == 0.0f)
			Size.Width = 1.0f;

		if (Size.Height == 0.0f )
			Size.Height = 1.0f;

		f32 avg = (size.Width + size.Height)/6;
		Box.MinEdge.set(-avg,-avg,-avg);
		Box.MaxEdge.set(avg,avg,avg);
	}
};

