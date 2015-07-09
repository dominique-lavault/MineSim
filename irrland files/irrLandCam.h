#pragma once
#include "LandManager.h"
#include <irrlicht.h>
#include "c:\code\lib\irrlicht-0.14.0\include\icamerascenenode.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#define CAMERA_MAX_SPEED 100.0f
#define CAMERA_GROUND_ALTITUDE 10.0f
#define CAMERA_FLY_ALTITUDE 500.0f
#define CAMERA_ACCELERATION 0.06f

enum AUTOPILOT_STATE { AS_OFF, AS_STARTING, AS_ON, AS_STOPPING };
enum CAMERA_MODE { CAMERA_MODE_INDOOR_FAR_FROM_ENTRANCE, CAMERA_MODE_INDOOR_NEAR_ENTRANCE, CAMERA_MODE_OUTDOOR};

class irrLandCam
{
public:
	irrLandCam::irrLandCam(ICameraSceneNode * camera);
	~irrLandCam(void);

	f32 m_cameraSpeed, m_acceleration;
	f32 m_latAcceleration, m_latCameraSpeed;
	f32 m_camAngleH, m_camAngleV;
	//f32 m_camDeltaAngleH;
	AUTOPILOT_STATE m_autopilotState;
	vector3df m_camPos, m_targetPos;
	ICameraSceneNode * m_camera;

	void irrLandCam::updatePosition(CLandManager * p_landManager);
	void irrLandCam::activateAutopilot(vector3df targetPos);
	CAMERA_MODE getMode();
	void setMode(CAMERA_MODE mode);
	void irrLandCam::setPosition(vector3df &pos);
private:
	f32 m_diagXZ;
	float m_camAlt;
	CAMERA_MODE m_camMode;
};
