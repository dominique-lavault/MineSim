// SimMine.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include <irrlicht.h>
#include <SIrrCreationParameters.h> 

#include "irrland files/LandManager.h"
#include "irrland files/irrLandCam.h"
#include "SimMine.h"
#include "MineWall.h"
#include "Galerie.h"
#include "GlobalData.h"
#include "BillboardSceneNode.h"
#include "IShakeItAnimator.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

#pragma comment(lib, "Irrlicht.lib")
#define RESOLUTIONX 800
#define RESOLUTIONY 600
IrrlichtDevice *device;
globalData g_data;	// container pour toutes les données globales au world
	#define XZSCALE 35.0f

// ===================================================================================
class MyEventReceiver : public IEventReceiver
{
public:
	virtual bool OnEvent(SEvent event)   
	{
		int m_xClicked, m_yClicked;
		position2d<s32> p;

		switch(event.EventType)
		{
		case EET_MOUSE_INPUT_EVENT:
			m_xClicked = p.X = event.MouseInput.X;
			m_yClicked = p.Y = event.MouseInput.Y;
/*
			if( event.MouseInput.Event == EMIE_MOUSE_MOVED && g_data.bMouseCursorCaptured==true)
			{
				ICursorControl * CursorControl = device->getCursorControl();
				core::position2d<f32> cursorpos = CursorControl->getRelativePosition();
				CursorControl->setPosition(0.5f, 0.5f);
				if( cursorpos.X !=0.5f || cursorpos.Y != 0.5f)
				{
					g_data.m_camera->m_camAngleH += (cursorpos.X-0.5f)*10.0f;
					g_data.m_camera->m_camAngleV += (f32)irr::core::PI/2*(0.5-cursorpos.Y);
				}
			}
			else*/ if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
			{
				if( g_data.m_camera->getMode()!=CAMERA_MODE_OUTDOOR )
				{
					// recherche l'intersection entre une line3D et un triangle3D.
					position2d<s32> pos(device->getCursorControl()->getPosition());
					line3d<f32> line = g_data.smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(pos,g_data.m_camera->m_camera);

					vector3df dummyPoint;
  					MineWall * selectedMineWall = g_data.m_galerie.getSelectedMineWall(&line,&dummyPoint, false);
					if( selectedMineWall )
					{
						if( selectedMineWall->isActive())	// on a cliqué un mur actif, on le descend (si on peut)
						{
							int oldNoBloc=selectedMineWall->getMineBloc()->getNoBloc();
							vector3df camToObj(g_data.m_camera->m_camera->getTarget()-g_data.m_camera->m_camera->getAbsolutePosition());
							camToObj.normalize();
							float abattageLen;
							if( selectedMineWall->getWallId() == WALL_ID_RADIER||selectedMineWall->getWallId() == WALL_ID_CIEL)
								abattageLen=50.0f;
							else
								abattageLen=40.0f;
							vector3df oldObjToNewObj(camToObj*abattageLen);

							//créer un nouveau bloc a greffer sur le mur cliqué
							MineBloc * theNewBloc = g_data.m_galerie.AddBloc(oldNoBloc, selectedMineWall->getWallId(), oldObjToNewObj);
							if(theNewBloc!=NULL)
							{
								selectedMineWall->remove();
								g_data.anim->shake(300,4);
								g_data.myBillboardNode->setPosition(theNewBloc->m_BBox.getCenter());
								g_data.myBillboardNode->setSize(dimension2d<f32>(20.0f,20.0f));
								g_data.myBillboardNode->resetAnim();

								// *** particles *** 
								if( g_data.ps != NULL)
									g_data.ps->remove();
								g_data.ps = g_data.smgr->addParticleSystemSceneNode(false);
								g_data.ps->setPosition(theNewBloc->m_BBox.getCenter());
								g_data.ps->setParticleSize(core::dimension2d<f32>(2.0f, 2.0f));

								vector3df centerToCam(theNewBloc->m_BBox.getCenter() - g_data.m_camera->m_camPos);
								scene::IParticleAffector* paf;
								scene::IParticleEmitter* em;
								em = g_data.ps->createBoxEmitter(//theNewBloc->m_BBox,
									core::aabbox3d<f32>(-15,-15,-15,15,15,15),
									-centerToCam/2000.0f, 100,500,
									video::SColor(0,255,255,255), video::SColor(0,255,255,255), 200,800);
								g_data.ps->setEmitter(em);
								em->drop();

								paf = g_data.ps->createGravityAffector(core::vector3df(-0.05f,-0.1f, 0.0f), 2000);
								g_data.ps->addAffector(paf);
								paf->drop();

								paf = g_data.ps->createFadeOutParticleAffector();
								g_data.ps->addAffector(paf);
								paf->drop();

								g_data.ps->setMaterialFlag(video::EMF_LIGHTING, false);
								g_data.ps->setMaterialFlag(video::EMF_ZBUFFER, false);
								g_data.ps->setMaterialTexture(0, g_data.driver->getTexture("media/dust.bmp"));
								g_data.ps->setMaterialType(video::EMT_TRANSPARENT_VERTEX_ALPHA); 
								g_data.particleFXFPS=device->getTimer()->getTime();

								// todo : projeter des blocs
								for(int i=0; i<10; i++)
								{
									IAnimatedMeshSceneNode* rock = g_data.smgr->addAnimatedMeshSceneNode(g_data.m_pOreRock, NULL, -1, theNewBloc->m_BBox.getCenter()+vector3df(rand()%30-15,rand()%30-15,rand()%30-15 ),vector3df(0,0,0),vector3df(rand()%4,rand()%4,rand()%4));
									g_data.m_oreRockSceneNodeArray.push_back(rock);
									scene::ISceneNodeAnimator* anim =  g_data.smgr->createRotationAnimator( vector3df(rand()%15,rand()%15,rand()%15 ));
									rock->addAnimator(anim);
									anim->drop();
								}
							}
						}
						else	// ce n'est pas un mur actif, on se téléporte dans le bloc cliqué
						{
							// on récupère le bloc enfant du bloc derrière ce wall
							MineBloc* myMB = g_data.m_galerie.getBloc(g_data.m_galerie.getBlocContainingCamera());
							if( myMB != (MineBloc*)NO_PARENT_BLOCK )
							{
								MineBloc * childMB = myMB->getChildMineBloc(selectedMineWall->getWallId());				
								if( childMB == (MineBloc*)NO_PARENT_BLOCK )
									childMB = myMB->getParentBloc();	// il n'y a pas de bloc enfant par là. donc si le wall a été descendu, c'est forcément notre parent
								// on récupère le centre
								if( childMB != (MineBloc*)NO_PARENT_BLOCK )
								{
									vector3df dest = childMB->m_BBox.getCenter();
									// on update la position de la camera
									g_data.m_camera->setPosition(dest);
								}
							}
						}

					}
				} // endif mode out door
			}
			else if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
			{
				// free / catch cursor
				ICursorControl * cc = device->getCursorControl();
				if( g_data.bMouseCursorCaptured == false)
				{
					g_data.bMouseCursorCaptured=true;
					cc->setVisible(false);
				}
				else
				{
					g_data.bMouseCursorCaptured=false;
					cc->setVisible(true);
				}
			}
			break;

		case EET_KEY_INPUT_EVENT:
			int i, n;
			wchar_t tmp[255] = L"Liste des entrées : ";
			wchar_t tmp2[10];
			if( ! event.KeyInput.PressedDown)
			{
				switch(event.KeyInput.Key)
				{
				case (KEY_ESCAPE) :
					device->closeDevice();
					break;		

				case (KEY_F2) :
				case (KEY_F3) :
					n = g_data.m_galerie.GetBlocCount();
					for(i=0; i<n; i++)
						g_data.m_galerie.setBlocMaterial(i,EMF_WIREFRAME, (event.KeyInput.Key==KEY_F2));
					break;		
				case (KEY_F4) :
					// afficher la liste des entrées
					for(i=0; i<(int)g_data.m_galerie.m_listeBlocEntree.size(); i++)
					{
						_snwprintf_s(tmp2, 10, L"%d/", g_data.m_galerie.m_listeBlocEntree[i]);
						wcscat_s(tmp, 255, tmp2);
					}
					g_data.m_messagelistbox->addItem(tmp);
					break;		

				case KEY_UP :
				case KEY_DOWN :
					g_data.m_camera->m_acceleration=0;
					break;

				case KEY_LEFT:
				case KEY_RIGHT:
					g_data.m_camera->m_latAcceleration=0;
					break;

				case KEY_SUBTRACT :
				case	KEY_ADD:
					// on fait monter / descendre la caméra dans le bloc au dessus / au dessous
					// si la caméra est dans un bloc
					if( g_data.m_camera->getMode()!=CAMERA_MODE_OUTDOOR )
					{
						int noBloc;
						if( (noBloc = g_data.m_galerie.getBlocContainingCamera()) != -1 )
						{	// tester si il y un bloc dessous / dessus
							MineBloc * mb = g_data.m_galerie.getBloc(noBloc);
							if( mb != (MineBloc*)NO_PARENT_BLOCK)
							{
								WALL_ID targetWallId;
								targetWallId = (event.KeyInput.Key == KEY_ADD ? WALL_ID_CIEL : WALL_ID_RADIER);
								MineWall * mw = mb->getWallSceneNode(targetWallId);

								if( mw != NO_MINEWALL )
									if( ! mw->isActive())	// le mur a été détruit, on peut passer
									{
										// si oui, l'expédier au dessus du radier du bloc en question
										MineBloc * cmb = mb->getChildMineBloc(targetWallId);
										if( cmb == (MineBloc*)NO_PARENT_BLOCK )
											cmb = mb->getParentBloc();	// il n'y a pas de bloc enfant par là. donc si le wall a été descendu, c'est forcément notre parent

										MineWall * cmw = cmb->getWallSceneNode(WALL_ID_RADIER);
										vector3df pos = g_data.m_camera->m_camera->getAbsolutePosition();
										pos.Y=cmw->Box.MaxEdge.Y+1.0f;
										g_data.m_camera->m_camera->setPosition(pos);
										g_data.m_camera->m_camera->updateAbsolutePosition();
									}
							}
						}
					}
					else	// out door : terrasser le terrain
					{
						vector3df justeDevantLaCamera = g_data.m_camera->m_camera->getAbsolutePosition()
						+ (g_data.m_camera->m_camera->getTarget()-g_data.m_camera->m_camera->getAbsolutePosition())/10;
						float value= event.KeyInput.Key == KEY_ADD ? 10.0f : -10.0f;
						justeDevantLaCamera+=vector3df(XZSCALE/2.0f, 0, XZSCALE/2.0f);

						if( g_data.m_landManager->setDeltaAltitude(&justeDevantLaCamera, value) == false)
						{
							s32 sel = g_data.m_messagelistbox->addItem(L"Terrasser si près d'une structure la fragiliserait.");
							g_data.m_messagelistbox->setItemColor(sel, SColor(0,255,0,0));
							g_data.m_messagelistbox->setListTopItem(max(0,sel-3));
						}
					}

					break;
				}
			}
			else // touche enfoncée
			{
				if ( event.KeyInput.Key == KEY_UP )
					g_data.m_camera->m_acceleration=CAMERA_ACCELERATION;
				else if ( event.KeyInput.Key == KEY_DOWN )
					g_data.m_camera->m_acceleration=-CAMERA_ACCELERATION;
				else if ( event.KeyInput.Key == KEY_LEFT )
					g_data.m_camera->m_latAcceleration=-CAMERA_ACCELERATION;
				else if ( event.KeyInput.Key == KEY_RIGHT )
					g_data.m_camera->m_latAcceleration=CAMERA_ACCELERATION;
			}

		}
		return false;
	}
};    


// =================================================================================
int WINAPI WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	g_data.bMouseCursorCaptured=false;
	// create engine and camera
	MyEventReceiver receiver; 
	SIrrlichtCreationParameters param;
	param.AntiAlias=true;
	param.Bits=32;
	param.DriverType=video::EDT_DIRECT3D9;
	param.EventReceiver=&receiver;
	param.Stencilbuffer=false;
	param.Vsync=true;
	param.WindowId=NULL;
	param.WindowSize = dimension2d< s32 >(RESOLUTIONX,RESOLUTIONY);

	// start up the engine
	if( (device = createDeviceEx(param))==NULL)
	{
		MessageBox(NULL, "Erreur","Impossible d'initialiser le moteur 3D.",MB_OK);
		return 1;
	}

	device->setWindowCaption(L"Sim Mine");
	g_data.driver = device->getVideoDriver();
	g_data.smgr = device->getSceneManager();
	g_data.driver->setTextureCreationFlag(ETCF_ALWAYS_32_BIT , TRUE);
	g_data.driver->setFog(SColor(0,173,186,203),true,200,2000);

	// --- add terrain
	// create triangle selector for the terrain	
	/*
	scene::ITriangleSelector* terrainSelector = g_data.smgr->createTerrainTriangleSelector(terrain, 0);
	terrain->setTriangleSelector(terrainSelector);
	terrainSelector->drop();
	triangle3df t;
	s32 cnt;
	// POURQUOI EST CE QUE CA INTERSECTE PAS?????
	line3d<f32> linefind(vector3df(20.0,1000.0,20.0),vector3df(20.0,-1000.0,20.0));
	terrainSelector->getTriangles(&t ,1, cnt, linefind);
	*/

	// irrland terrain
	g_data.m_landManager = new CLandManager(g_data.smgr->getRootSceneNode(),g_data.smgr,g_data.driver);
	vector3df centerOfTheWorld(CENTEROFTHEWORLD_X, 0.0f, CENTEROFTHEWORLD_Z);

	// creating light and camera
	g_data.m_camera = new irrLandCam(g_data.smgr->addCameraSceneNode(0, centerOfTheWorld, vector3df(100,30,100),1));
	ILightSceneNode * light = g_data.smgr->addLightSceneNode(g_data.m_camera->m_camera, vector3df(0,0,0),SColorf(155,145,100),1.0f);

	// -------------- créée le carreau de la mine et l'oriente par rapport au relief ------------------
	g_data.m_metaSelector = g_data.smgr->createMetaTriangleSelector();
	/*
	scene::ISceneNodeAnimator* anim =  g_data.smgr->createCollisionResponseAnimator(g_data.m_metaSelector, g_data.m_camera , core::vector3df(30,50,30),core::vector3df(0,-3,0),core::vector3df(0,50,0));
	g_data.m_camera ->addAnimator(anim);
	anim->drop();
	*/
	g_data.driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS,false);
	ISceneNode* SkyBox = g_data.smgr->addSkyBoxSceneNode(
		g_data.driver->getTexture("media/up.jpg"),
		g_data.driver->getTexture("media/dn.jpg"),
		g_data.driver->getTexture("media/lt.jpg"),
		g_data.driver->getTexture("media/rt.jpg"),
		g_data.driver->getTexture("media/ft.jpg"),
		g_data.driver->getTexture("media/bk.jpg"));
	g_data.driver->setTextureCreationFlag(ETCF_CREATE_MIP_MAPS,true);

	g_data.pCarreau = g_data.smgr->getMesh ("media/carreau_mine2.3ds");
	g_data.pPorche = g_data.smgr->getMesh ("media/porche.3ds");
	g_data.m_pOreRock = g_data.smgr->getMesh ("media/ore-rock.3ds");
	g_data.pEtagePuits = g_data.smgr->getMesh ("media/etage_puits.3ds");
	IMeshManipulator* mm = g_data.smgr->getMeshManipulator();
	IMesh * etagePuitsSM = g_data.pEtagePuits->getMesh(0);
	mm->makePlanarTextureMapping(etagePuitsSM, 3.0f);
	mm->recalculateNormals(etagePuitsSM);

	// ------------ init triangle selector ---------------
	core::line3d<f32> line;
	core::vector3df intersection;
	core::triangle3df tri;

	// ------------ building terrain loop ---------------
	for(int build=0; build<10; build++)
	{
		g_data.m_camera->updatePosition(g_data.m_landManager);
		g_data.m_landManager->updateLand();
	}

	// ----------------------------------- création des meshes ---------------------------------------------
	vector3df axeMine;
	vector3df Origin(0.0f,g_data.m_landManager->getAltitude(0.0f,0.0f), 0.0f);
	vector3df Xaxis(Origin.X+50.0f,g_data.m_landManager->getAltitude(Origin.X+50.0f,Origin.Z), Origin.Z);
	vector3df Zaxis(Origin.X,g_data.m_landManager->getAltitude(Origin.X,Origin.Z+50.0f), Origin.Z+50.0f);
	vector3df NBXaxis=Xaxis-Origin;
	vector3df NBZaxis=Zaxis-Origin;
	vector3df normale=NBZaxis.crossProduct(NBXaxis);
	normale.Y=0;
	normale.normalize();
	axeMine=normale;
	if( abs(axeMine.X) > abs(axeMine.Z))
		axeMine.Z=0;
	else
		axeMine.X=0;
	axeMine.normalize();

	Origin.Y=0;	// l'altitude est déjà comprise dans les coordonnées du profil d'entrée
	g_data.m_origin=Origin;	
	MineProfil profilJonction = g_data.m_landManager->createGalerieJonction(0,0,&axeMine);
	profilJonction.reverse();
	axeMine*=-40.0f;
	MineBloc * newBloc = g_data.m_galerie.AddBloc(NO_PARENT_BLOCK,WALL_ID_FDTFRONT,axeMine,WALL_ID_FDTBACK,&profilJonction);

	g_data.anim=new IShakeItAnimator(10, 3);
	g_data.m_camera->m_camera->addAnimator(g_data.anim);
	g_data.anim->drop();
		
	// ---- création d'un billboard pour divers effets spéciaux (explosion, ...) -----
	g_data.myBillboardNode = new BillBoardSceneNode(g_data.smgr->getRootSceneNode(), g_data.smgr, 666, g_data.m_camera->m_camera);
	g_data.myBillboardNode->setDebugDataVisible(false);
	g_data.myBillboardNode->setScale(vector3df(4.0f,4.0f,4.0f));
	g_data.myBillboardNode->setMaterialTexture(0, g_data.driver->getTexture("media/boum.tga"));
	g_data.myBillboardNode->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
	g_data.myBillboardNode->setMaterialFlag(EMF_LIGHTING, false);
	g_data.myBillboardNode->drop();

	// add GUI
	rect<s32> r(0,0,500,100);
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	//g_data.m_messagelistbox = (CGUIExtListBox*) guienv->addListBox(r);
	//g_data.m_messagelistbox->setRelativePosition(r);
	g_data.m_messagelistbox=new CGUIExtListBox(guienv, guienv->getRootGUIElement(), 112, r);
	IGUISkin* skin = guienv->getSkin();
	IGUIFont* font = guienv->getFont("media/fonthaettenschweiler.bmp");
	if (font)
		skin->setFont(font);
	g_data.m_messagelistbox->addItem(L"Welcome MineSim :)");
	g_data.m_messagelistbox->addItem(L"Move with cursor keys...");
	CAMERA_MODE camMode, oldCamMode=CAMERA_MODE_OUTDOOR;

	//--------------------------------------------------------------------------------------------------------------
	ISceneNode * t1 = g_data.smgr->addTestSceneNode(1.0f, light, -1, vector3df(0,0,0));
	float distanceToEntrance=0.0f;

	wchar_t tmp[255];
	int oldNoBloc=-1;
	while(device->run())
	{
		if( g_data.bMouseCursorCaptured==true)
		{
			ICursorControl * CursorControl = device->getCursorControl();
			core::position2d<f32> cursorpos = CursorControl->getRelativePosition();
			CursorControl->setPosition(0.5f, 0.5f);
			if( cursorpos.X !=0.5f || cursorpos.Y != 0.5f)
			{
				g_data.m_camera->m_camAngleH += (cursorpos.X-0.5f)*PI/2;
				g_data.m_camera->m_camAngleV += (f32)irr::core::PI/2*(0.5f-cursorpos.Y);
			}
		}

		// -- update camera position
		vector3df oldpos=g_data.m_camera->m_camPos;
		g_data.m_camera->updatePosition(g_data.m_landManager);
		vector3df newpos=g_data.m_camera->m_camPos;
		if( oldpos!=newpos)
		{
			int noBloc = g_data.m_galerie.getBlocContainingCamera();
			if( noBloc!=-1)
			{
				MineBloc * bloc = g_data.m_galerie.getBloc(noBloc);
				if( bloc!=NULL )
				{
					f32 distance;
					f32 minDistance=0.0f;	// distance au ciel
					// on teste tous les walls actifs, sauf le radier
					for(int i=(int)WALL_ID_CIEL; i<(int)NB_WALL_ID; i++)
					{
						MineWall * mw=bloc->getWallSceneNode((WALL_ID)i);
						if( mw->isActive() )
						{
							MineProfil * profil = mw->getProfil();
							plane3d<f32> plane(profil->m_s1, profil->m_s3, profil->m_s2);
							plane.Normal.normalize();
							distance = plane.getDistanceTo(newpos);

							if( distance < minDistance)
							{
								float penetration=minDistance-distance;
								g_data.m_camera->setPosition(newpos+plane.Normal*penetration);
							}
						}
						minDistance=6.0f;	// pour les walls != ciel
					}
				}
			}
		}

		// ----------------- camera position stuff ---------------------
		int noBloc=g_data.m_galerie.getBlocContainingCamera();
		if( noBloc != oldNoBloc )
		{
			oldNoBloc=noBloc;

			if( noBloc==-1)
				camMode=CAMERA_MODE_OUTDOOR;
			else
			{
				#define SKYBOX_ONOFF_DISTANCE_TO_ENTRANCE	200.0f
				bool closeToEntrance=false;
				for(int i=0; i<(int)g_data.m_galerie.m_listeBlocEntree.size(); i++)
				{
					MineBloc * pmb = g_data.m_galerie.getBloc(g_data.m_galerie.m_listeBlocEntree[i]);
					distanceToEntrance=(float)pmb->m_BBox.getCenter().getDistanceFrom(g_data.m_camera->m_camPos);
					if( distanceToEntrance < SKYBOX_ONOFF_DISTANCE_TO_ENTRANCE )
					{						
						closeToEntrance=true;
					}
				}
				if( closeToEntrance )
					camMode=CAMERA_MODE_INDOOR_NEAR_ENTRANCE;
				else
					camMode=CAMERA_MODE_INDOOR_FAR_FROM_ENTRANCE;

			}
			g_data.m_camera->setMode(camMode);
			g_data.m_galerie.setLOD(g_data.m_camera->m_camPos);
		}
		g_data.m_landManager->updateLand();

		// ------- que doit-on afficher? --------
		camMode = g_data.m_camera->getMode();
		if( camMode != oldCamMode)
		{
			if( camMode ==CAMERA_MODE_INDOOR_FAR_FROM_ENTRANCE )
			{
				g_data.m_camera->m_camera->setFarValue(200.0f);
				SkyBox->setVisible(false);
			}
			else if( camMode ==CAMERA_MODE_INDOOR_NEAR_ENTRANCE )
			{
				SkyBox->setVisible(true);
				//g_data.m_galerie.setUndergroundPartVisible(true);
				g_data.m_camera->m_camera->setFarValue(8000.0f);
			}
			else if( camMode ==CAMERA_MODE_OUTDOOR )
			{
				SkyBox->setVisible(true);
				g_data.m_camera->m_camera->setFarValue(8000.0f);
				//g_data.m_galerie.setUndergroundPartVisible(false);
			}
			oldCamMode = camMode;
		}

		if( camMode ==CAMERA_MODE_INDOOR_NEAR_ENTRANCE )
		{
			f32 farValue = 2000.0f+6000.0f*(1-distanceToEntrance/SKYBOX_ONOFF_DISTANCE_TO_ENTRANCE);
			farValue=MAX(2000.0f, farValue);
			farValue=MIN(8000.0f, farValue);
			g_data.m_camera->m_camera->setFarValue(farValue);
		}

		// dummy sample  code pour balader un mesh devant la caméra (pelle, pioche, rail ...)
//		Origin =g_data.m_camera->m_camera->getAbsolutePosition()
	//		+(g_data.m_camera->m_camera->getTarget()-g_data.m_camera->m_camera->getAbsolutePosition())/2;
		//vector3df tmpv=g_data.m_camera->m_camera->getTarget()-g_data.m_camera->m_camera->getAbsolutePosition();

// ----------------------- draw everything --------------------------
		g_data.driver->beginScene(true, true, video::SColor(0,0,0,0));
		g_data.smgr->drawAll();
		guienv->drawAll();

		// draw triangle selector
		line.start = g_data.m_camera->m_camera->getPosition();
		line.end = line.start +	(g_data.m_camera->m_camera->getTarget() - line.start).normalize() * 1000.0f;
		if (g_data.smgr->getSceneCollisionManager()->getCollisionPoint(	line, g_data.m_metaSelector, intersection, tri))
		{
			g_data.driver->setTransform(video::ETS_WORLD, core::matrix4());
			g_data.driver->draw3DTriangle(tri, video::SColor(0,255,0,0));
		}

		// display débug info : galerie profil
		/*
		g_data.driver->setTransform(video::ETS_WORLD, core::matrix4());
		int nb = g_data.m_galerie.GetBlocCount();
		MineBloc * bloc = g_data.m_galerie.getBloc(0);
		vector3df start=bloc->getBlocVector();
		vector3df end;
		for(int i=1; i<nb; i++)
		{
			bloc = g_data.m_galerie.getBloc(i);
			end=bloc->getBlocVector();
			g_data.driver->draw3DLine(start,start+end,SColor(255,255,255,255));
			start+=end;
		}
		*/

		/* trace le repère du world irrlicht
		g_data.driver->draw3DLine(vector3df(0,0,0), vector3df(1000,0,0),SColor(255,255,0,255));
		g_data.driver->draw3DLine(vector3df(0,0,0), vector3df(0,1000,0),SColor(255,255,255,0));
		g_data.driver->draw3DLine(vector3df(0,0,0), vector3df(0,0,1000),SColor(255,255,255,255));
		*/
		g_data.driver->endScene();

		_snwprintf_s(tmp, 255, L"MineSim %s fps:%d polys : %d / mode=%s / noBloc=%d / DtE=%0.1f / h=%0.2f / V=%0.2f", 
			g_data.driver->getName(),	
			g_data.driver->getFPS(), 
			g_data.driver->getPrimitiveCountDrawn(),
			g_data.m_camera->getMode()!=CAMERA_MODE_OUTDOOR?L"indoor":L"outdoor",
			noBloc,distanceToEntrance, g_data.m_camera->m_camAngleH, g_data.m_camera->m_camAngleV);
		device->setWindowCaption(tmp);
		
		// animate particles
		if( g_data.ps != NULL )
		{
			int deltaTime = device->getTimer()->getTime() - g_data.particleFXFPS;

			if( deltaTime < 2000 )
			{
				g_data.ps->setPosition(g_data.ps->getAbsolutePosition()+vector3df(0,deltaTime/1000.0f,0));
				int n=g_data.m_oreRockSceneNodeArray.size();
				for( int rock=0; rock<n; rock++)
				{
					vector3df v(g_data.m_oreRockSceneNodeArray[rock]->getAbsolutePosition());
					v+=vector3df (0, -0.1f-(deltaTime/2000.0f)*(deltaTime/2000.0f),0 );
					g_data.m_oreRockSceneNodeArray[rock]->setPosition(v);
				}
			}
			else 
			{
				g_data.ps->removeAllAffectors();
				g_data.ps->removeAnimators();
				g_data.ps->remove();
				g_data.ps=NULL;
				int n=g_data.m_oreRockSceneNodeArray.size();
				for( int rock=0; rock<n; rock++)
					g_data.m_oreRockSceneNodeArray[rock]->remove();
				g_data.m_oreRockSceneNodeArray.clear();
			}
		}
	} // end l00p

	delete g_data.m_camera;
	device->drop();
	delete g_data.m_landManager;
	
	return 0;
}
