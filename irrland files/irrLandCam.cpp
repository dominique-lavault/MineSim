//#include <windows.h>
#include <stdio.h>

#include <irrlicht.h>
//#include "f:\code\lib\irrlicht-0.6\include\icamerascenenode.h"

#include "landSceneNode.h"
#include "landManager.h"
#include ".\irrlandcam.h"
#include "../globalData.h"

extern globalData g_data;

irrLandCam::irrLandCam(ICameraSceneNode * camera)
{
	m_camAngleH = m_camAngleV = 0.0f;
	//m_camDeltaAngleH = 0.0f;
	m_cameraSpeed=0.001f;
	m_acceleration=0.0f;
	m_latCameraSpeed=0.0f;
	m_latAcceleration=0.0f;
	m_autopilotState=AS_OFF;
	m_diagXZ = XZSCALE*(f32)sqrt(2.0f);
	m_camera=camera;
	m_camera->setFOV((f32)(60.0 * PI /180));
	m_camAlt=CAMERA_GROUND_ALTITUDE;
	m_camMode=CAMERA_MODE_OUTDOOR;
}

irrLandCam::~irrLandCam(void)
{
}

void irrLandCam::setPosition(vector3df &pos)
{
	m_camPos=pos;
	m_cameraSpeed=0;
	m_latCameraSpeed=0;
	m_acceleration=0;	
	m_camera->setPosition(m_camPos);
	m_camera->updateAbsolutePosition();
}

void irrLandCam::updatePosition(CLandManager * p_landManager)
{
	vector3df deplacement;
	
	m_camPos = m_camera->getAbsolutePosition();
	deplacement.X = m_cameraSpeed * sin(m_camAngleH) + m_latCameraSpeed * sin(m_camAngleH+PI/2);
	deplacement.Y = 0;
	deplacement.Z = m_cameraSpeed * cos(m_camAngleH) + m_latCameraSpeed * cos(m_camAngleH+PI/2);

	m_camPos+=deplacement;

	if( m_camMode==CAMERA_MODE_OUTDOOR)
	{
	// for real terrain 
		f32 Y00,Y10,Y01,Y11,x,z;
		x = floor(m_camPos.X/XZSCALE)*XZSCALE;
		z = floor(m_camPos.Z/XZSCALE)*XZSCALE;
		Y00=p_landManager->getAltitudeWithDelta(x, z);
		Y10=p_landManager->getAltitudeWithDelta(x+XZSCALE, z);
		Y01=p_landManager->getAltitudeWithDelta(x, z+XZSCALE);
		Y11=p_landManager->getAltitudeWithDelta(x+XZSCALE, z+XZSCALE);

		float Hc = (m_camPos.X-x)/XZSCALE*(Y10-Y00)+Y00;
		float hc = (m_camPos.X-x)/XZSCALE*(Y11-Y01)+Y01;
		m_camPos.Y = (m_camPos.Z-z)/XZSCALE*(hc-Hc)+Hc+20.0f;
	}
	// for perlin terrain
	else if (m_camMode!=CAMERA_MODE_OUTDOOR)
	{
		if( g_data.m_galerie.getAltitudeAt(&m_camPos, &m_camPos.Y))
			m_camPos.Y+=20.0f;
	}

/*
	if( m_autopilotState == AS_ON)
	{
		if( m_camAlt<CAMERA_FLY_ALTITUDE)
			m_camAlt+=10.0f;
	}
	else
	{
		if( m_camAlt>CAMERA_GROUND_ALTITUDE)
		{
			m_camAlt-=10.0f;
			m_camAngleV=0.0f;
		}
	}

	if( m_autopilotState == AS_OFF)
	{
	*/
		m_targetPos.X=m_camPos.X + 500.0f * sin(m_camAngleH) * cos(m_camAngleV);
		m_targetPos.Y=m_camPos.Y + 500.0f * sin(m_camAngleV);
		m_targetPos.Z=m_camPos.Z + 500.0f * cos(m_camAngleH) * cos(m_camAngleV);
		m_camera->setTarget(m_targetPos);
	/*
	}
	else	if( m_autopilotState == AS_ON)
	{
		m_latCameraSpeed=m_latAcceleration=0.0f;
		if( m_targetPos.getDistanceFrom(m_camPos)<=2.0f*m_cameraSpeed+m_camAlt)
			m_autopilotState=AS_STOPPING;
	}
	else	if( m_autopilotState == AS_STOPPING)
	{
		m_autopilotState=AS_OFF;
	}
	*/
	m_camera->setPosition(m_camPos);

	if( (m_acceleration>0 && m_cameraSpeed<0 && m_cameraSpeed+m_acceleration>0) 
		|| (m_acceleration<0 && m_cameraSpeed>0 && m_cameraSpeed+m_acceleration<0) )
	{
		m_acceleration=0;
		m_cameraSpeed=0;
	}
	else
		m_cameraSpeed = MAX(MIN(m_cameraSpeed, CAMERA_MAX_SPEED), -CAMERA_MAX_SPEED);

	if( (m_latAcceleration>0 && m_latCameraSpeed<0 && m_latCameraSpeed+m_latAcceleration>0) 
		|| (m_latAcceleration<0 && m_latCameraSpeed>0 && m_latCameraSpeed+m_latAcceleration<0) )
	{
		m_latAcceleration=0;
		m_latCameraSpeed=0;
	}
	else
		m_latCameraSpeed = MAX(MIN(m_latCameraSpeed, CAMERA_MAX_SPEED), -CAMERA_MAX_SPEED);

	m_cameraSpeed+=m_acceleration;
	m_latCameraSpeed+=m_latAcceleration;

	if( m_autopilotState !=AS_ON)		// frottements = autoralentissement
	{
		m_cameraSpeed*=0.95f;
		m_latCameraSpeed*=0.95f;
	}
}

void irrLandCam::activateAutopilot(vector3df targetPos)
{
	m_targetPos=targetPos;
	m_autopilotState=AS_ON;
	m_cameraSpeed = 10.0f;
	m_acceleration = 0.0f;
	//m_camDeltaAngleH=0.0f;
	f32 d = (f32)m_camPos.getDistanceFrom(m_targetPos);
	f32 dx = m_targetPos.X - m_camPos.X;
	f32 dz = m_targetPos.Z - m_camPos.Z;
	m_camAngleH = (f32) acos (dz/d);
	if( asin(dx/d)<0)
		m_camAngleH *= -1;
	m_camera->setTarget(m_targetPos);
}

CAMERA_MODE irrLandCam::getMode()
{
	return m_camMode;
}
void irrLandCam::setMode(CAMERA_MODE mode)
{
	m_camMode=mode;
}
