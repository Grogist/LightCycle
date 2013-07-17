#include "PlayState.h"

#include "Application.h"
#include "MenuState.h"

namespace LC
{
	PlayState PlayState::m_PlayState;

	PlayState::PlayState() : m_Avatar(nullptr), m_Console(nullptr) {}

	void PlayState::Init()
	{
		m_ShutDown = false;

		m_IsGameOver = false;
		m_IsScoreIncremented = false;

		m_IsHost = TheApplication.IsHost();

		m_Avatar = nullptr;

		m_OgreRoot = Ogre::Root::getSingletonPtr();
		m_SceneMgr = m_OgreRoot->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);
		m_Camera = m_SceneMgr->createCamera("GameCamera");
		m_CameraSceneNode = m_SceneMgr->createSceneNode("GameCameraSceneNode");
		m_CameraSceneNode->attachObject(m_Camera);
		m_RenderWindow = m_OgreRoot->getAutoCreatedWindow();
		m_ViewPort = m_RenderWindow->addViewport(m_Camera);
		m_ViewPort->setBackgroundColour(Ogre::ColourValue::Black);
		m_Input = LC::Input::getSingletonPtr();
		
		m_Camera->setAspectRatio(Ogre::Real(m_ViewPort->getActualWidth()) / Ogre::Real(m_ViewPort->getActualHeight()));
		m_Camera->setNearClipDistance(10.f);

		m_SceneMgr->setAmbientLight(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
		m_SceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 5000.f, true);
 
		// SETUP BULLET PHYSICS
		m_CollisionConfiguration = new btDefaultCollisionConfiguration();
		m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
		m_OverlappingPairCache = new btDbvtBroadphase();
		m_Solver = new btSequentialImpulseConstraintSolver;
		m_DynamicsWorld = new btDiscreteDynamicsWorld(m_Dispatcher, 
			m_OverlappingPairCache, m_Solver, m_CollisionConfiguration);
		m_DynamicsWorld->setGravity(btVector3(0.f, -10.f, 0.f));
		// END SETUP BULLET PHYSICS

		// TERRIBLE. FIX!
		if(m_IsHost)
		{
			m_PlayerChassisShape = new btBoxShape(btVector3(1.f,0.5f,2.f));
			m_PlayerWheelShape = new btCylinderShapeX(btVector3(ServerPlayer::wheelWidth,
				ServerPlayer::wheelRadius, ServerPlayer::wheelRadius));
			m_WallShape = new btBoxShape(btVector3(1.f,1.f,149.f));
			m_Compound = new btCompoundShape();
			btTransform localTrans;
			localTrans.setIdentity();
			localTrans.setOrigin(btVector3(0,1,0));
			m_Compound->addChildShape(localTrans, m_PlayerChassisShape);

			btScalar mass(100.f);
			bool isDynamic = (mass != 0.f);
			btVector3 localInertia(0,0,0);
			if(isDynamic)
				m_PlayerChassisShape->calculateLocalInertia(mass, localInertia);

			// DO THIS BETTER AS WELL!
			m_ServerNumberAlive = 0;

			std::set<participant_ptr> *participants = Server::getSingletonPtr()->get_all_participants();
			// TEMP
			int numberofplayers = 0;
			std::set<participant_ptr>::iterator itr = participants->begin();
			for(itr; itr != participants->end(); ++itr)
			{
				if(numberofplayers == 0)
				{
					btTransform startTransform;
					startTransform.setIdentity();
					startTransform.setOrigin(btVector3(125.f, 5.f, 125.f));
					startTransform.setRotation(btQuaternion(btVector3(0.f,1.f,0.f),btScalar(3.1415)));
					btDefaultMotionState *motionState = new btDefaultMotionState(startTransform);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,m_Compound,localInertia);
					btRigidBody* body = new btRigidBody(rbInfo);
					body->setWorldTransform(startTransform);
					body->setDamping(0.2f,0.2f);
					m_DynamicsWorld->addRigidBody(body);
					std::string player = "Player" + boost::lexical_cast<std::string>(numberofplayers);
					LC::ServerPlayer p(player, body, motionState, m_DynamicsWorld);
					m_ServerPlayerList.push_back(p);
					itr->get()->player_ptr_ = &m_ServerPlayerList.back();
					body->setUserPointer(&m_ServerPlayerList.back());
					numberofplayers++;
					m_ServerNumberAlive++;
				}
				else if(numberofplayers == 1)
				{
					btTransform startTransform;
					startTransform.setIdentity();
					startTransform.setOrigin(btVector3(-125.f, 5.f, -125.f));
					btDefaultMotionState *motionState = new btDefaultMotionState(startTransform);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,m_Compound,localInertia);
					btRigidBody* body = new btRigidBody(rbInfo);
					body->setWorldTransform(startTransform);
					body->setDamping(0.2f,0.2f);
					m_DynamicsWorld->addRigidBody(body);
					std::string player = "Player" + boost::lexical_cast<std::string>(numberofplayers);
					LC::ServerPlayer p(player, body, motionState, m_DynamicsWorld);
					m_ServerPlayerList.push_back(p);
					itr->get()->player_ptr_ = &m_ServerPlayerList.back();
					body->setUserPointer(&m_ServerPlayerList.back());
					numberofplayers++;
					m_ServerNumberAlive++;
				}
				else if(numberofplayers == 2)
				{
					btTransform startTransform;
					startTransform.setIdentity();
					startTransform.setOrigin(btVector3(125.f, 10.f, -125.f));
					btDefaultMotionState *motionState = new btDefaultMotionState(startTransform);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,m_Compound,localInertia);
					btRigidBody* body = new btRigidBody(rbInfo);
					body->setWorldTransform(startTransform);
					body->setDamping(0.2f,0.2f);
					m_DynamicsWorld->addRigidBody(body);
					std::string player = "Player" + boost::lexical_cast<std::string>(numberofplayers);
					LC::ServerPlayer p(player, body, motionState, m_DynamicsWorld);
					m_ServerPlayerList.push_back(p);
					itr->get()->player_ptr_ = &m_ServerPlayerList.back();
					body->setUserPointer(&m_ServerPlayerList.back());
					numberofplayers++;
					m_ServerNumberAlive++;
				}
				else if(numberofplayers == 3)
				{
					btTransform startTransform;
					startTransform.setIdentity();
					startTransform.setOrigin(btVector3(-125.f, 10.f, 125.f));
					startTransform.setRotation(btQuaternion(btVector3(0.f,1.f,0.f),btScalar(3.1415)));
					btDefaultMotionState *motionState = new btDefaultMotionState(startTransform);
					btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,m_Compound,localInertia);
					btRigidBody* body = new btRigidBody(rbInfo);
					body->setWorldTransform(startTransform);
					body->setDamping(0.2f,0.2f);
					m_DynamicsWorld->addRigidBody(body);
					std::string player = "Player" + boost::lexical_cast<std::string>(numberofplayers);
					LC::ServerPlayer p(player, body, motionState, m_DynamicsWorld);
					m_ServerPlayerList.push_back(p);
					itr->get()->player_ptr_ = &m_ServerPlayerList.back();
					body->setUserPointer(&m_ServerPlayerList.back());
					m_ServerNumberAlive++;
				}
			}
		}

		
		if(m_IsHost)
		{
			// SERVER SEND CREATE AVATAR MESSAGES.
			std::vector<SnapShotObject> avatars;

			// NEED TO BLOCK m_ServerPlayerList.
			for(unsigned int i=0; i < m_ServerPlayerList.size(); ++i)
			{
				LC::ServerPlayer *player = &m_ServerPlayerList.at(i);
				if(!player->m_RigidBody)
				{
					m_ShutDown = true;
					return;
				}
				btTransform transform = player->m_RigidBody->getWorldTransform();

				SnapShotObject newInstance(player->m_Name, transform.getOrigin(),
					transform.getRotation(), player->m_RigidBody->getLinearVelocity());

				avatars.push_back(newInstance);
			}
			LC::Server::getSingletonPtr()->write_create_avatar_message(avatars);

			// SERVER SEND SCORE MESSAGE.
			std::vector<std::string> scores;

			std::set<participant_ptr> *participants = 
				LC::Server::getSingletonPtr()->get_all_participants();
			std::set<participant_ptr>::iterator itr = participants->begin();
			// MERGE WITH ABOVE!

			for(itr; itr != participants->end(); ++itr)
			{
				std::string score = itr->get()->get_participant_name() + " " +
					boost::lexical_cast<std::string>(itr->get()->score_);
				scores.push_back(score);
			}
			LC::Server::getSingletonPtr()->write_score_message(scores);
		}

		// MAKE GROUND
		Ogre::Vector2 WorldSize(300.f,300.f);

		Ogre::Vector2 segments;
		segments.x = WorldSize.x / 25;
		segments.y = WorldSize.y / 25;

		Ogre::Plane plane(Ogre::Vector3::UNIT_Y, Ogre::Real(-2.5));
		Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			plane, WorldSize.x, WorldSize.y, 20, 20, true, 1, segments.x, segments.y, Ogre::Vector3::UNIT_Z);

		Ogre::Entity *entGround = m_SceneMgr->createEntity("GroundEntity", "ground");
		m_SceneMgr->getRootSceneNode()->createChildSceneNode("Ground")->attachObject(entGround);
		entGround->setMaterialName("Ground/Red");

		if(m_IsHost)
		{
			m_GroundShape = new btStaticPlaneShape(btVector3(0.f,1.f,0.f),0);
		
			m_GroundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
			btRigidBody::btRigidBodyConstructionInfo
					groundRigidBodyCI(0,m_GroundMotionState,m_GroundShape,btVector3(0,0,0));
			m_GroundRigidBody = new btRigidBody(groundRigidBodyCI);
			m_DynamicsWorld->addRigidBody(m_GroundRigidBody);
		}
		// END MAKE GROUND

		
		// MAKE SURROUNDING WALLS
		Ogre::SceneNode *node = m_SceneMgr->getRootSceneNode()->
			createChildSceneNode("Surr1");
		Ogre::Entity *ent = m_SceneMgr->createEntity("Surr1ent", "Cube.mesh");
		node->attachObject(ent);
		node->setPosition(150.f,-1.25f,0.f);
		node->rotate(Ogre::Vector3(Ogre::Vector3::UNIT_Y), Ogre::Degree(0.f));
		node->setScale(1.f,1.f,151.f);
		m_ClientSurroundingWalls.push_back(
			LC::ClientWall(node->getName(), node, ent, Ogre::Vector3::UNIT_X));

		if(m_IsHost)
		{
			Ogre::Quaternion Oq = node->getOrientation();
			btQuaternion Bq(Oq.x, Oq.y, Oq.z, Oq.w);
			Ogre::Vector3 Ov = node->getPosition();
			btVector3 Bv(Ov.x, Ov.y, Ov.z);
			btTransform t(Bq, Bv);

			CreateWall(t, btVector3(1.f, 0.f, 0.f));
		}

		node = m_SceneMgr->getRootSceneNode()->
			createChildSceneNode("Surr2");
		ent = m_SceneMgr->createEntity("Surr2ent", "Cube.mesh");
		node->attachObject(ent);
		node->setPosition(-150.f,-1.25f,0.f);
		node->rotate(Ogre::Vector3(Ogre::Vector3::UNIT_Y), Ogre::Degree(0.f));
		node->setScale(1.f,1.f,151.f);
		m_ClientSurroundingWalls.push_back(
			LC::ClientWall(node->getName(), node, ent, Ogre::Vector3::UNIT_X));

		if(m_IsHost)
		{
			Ogre::Quaternion Oq = node->getOrientation();
			btQuaternion Bq(Oq.x, Oq.y, Oq.z, Oq.w);
			Ogre::Vector3 Ov = node->getPosition();
			btVector3 Bv(Ov.x, Ov.y, Ov.z);
			btTransform t(Bq, Bv);

			CreateWall(t, btVector3(1.f, 0.f, 0.f));
		}

		node = m_SceneMgr->getRootSceneNode()->
			createChildSceneNode("Surr3");
		ent = m_SceneMgr->createEntity("Surr3ent", "Cube.mesh");
		node->attachObject(ent);
		node->setPosition(0.f,-1.25f,150.f);
		node->rotate(Ogre::Vector3(Ogre::Vector3::UNIT_Y), Ogre::Degree(90.f));
		node->setScale(1.f,1.f,151.f);
		m_ClientSurroundingWalls.push_back(
			LC::ClientWall(node->getName(), node, ent, Ogre::Vector3::UNIT_X));

		if(m_IsHost)
		{
			Ogre::Quaternion Oq = node->getOrientation();
			btQuaternion Bq(Oq.x, Oq.y, Oq.z, Oq.w);
			Ogre::Vector3 Ov = node->getPosition();
			btVector3 Bv(Ov.x, Ov.y, Ov.z);
			btTransform t(Bq, Bv);

			CreateWall(t, btVector3(1.f, 0.f, 0.f));
		}

		node = m_SceneMgr->getRootSceneNode()->
			createChildSceneNode("Surr4");
		ent = m_SceneMgr->createEntity("Surr4ent", "Cube.mesh");
		node->attachObject(ent);
		node->setPosition(0.f,-1.25f,-150.f);
		node->rotate(Ogre::Vector3(Ogre::Vector3::UNIT_Y), Ogre::Degree(90.f));
		node->setScale(1.f,1.f,151.f);
		m_ClientSurroundingWalls.push_back(
			LC::ClientWall(node->getName(), node, ent, Ogre::Vector3::UNIT_X));

		if(m_IsHost)
		{
			Ogre::Quaternion Oq = node->getOrientation();
			btQuaternion Bq(Oq.x, Oq.y, Oq.z, Oq.w);
			Ogre::Vector3 Ov = node->getPosition();
			btVector3 Bv(Ov.x, Ov.y, Ov.z);
			btTransform t(Bq, Bv);

			CreateWall(t, btVector3(1.f, 0.f, 0.f));
		}
		// END MAKE SURROUNDING WALLS


		wmgr = CEGUI::WindowManager::getSingletonPtr();
		
		CEGUI::Window* MenuInterface = wmgr->loadWindowLayout("LightCycleGame.layout");

		wmgr->getWindow("LIGHTCYCLEGAME/MenuWindow/Quit")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::QuitButton, this));
		wmgr->getWindow("LIGHTCYCLEGAME/Victory/Reset")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&PlayState::ResetButton, this));

		wmgr->getWindow("LIGHTCYCLEGAME/MenuWindow")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/MenuWindow")->setAlwaysOnTop(true);
		wmgr->getWindow("LIGHTCYCLEGAME/Victory")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/1")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/2")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/3")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/4")->hide();

		LC::GUI::getSingletonPtr()->getGUIWindow()->addChildWindow(MenuInterface);

		CEGUI::Window* consolewindow = wmgr->getWindow("LIGHTCYCLEGAME/Console");
		consolewindow->hide();
		m_Console = new LC::GameConsole(consolewindow, 0.6f);

		CEGUI::MouseCursor::getSingletonPtr()->hide();

		if(m_IsHost)
			LC::Server::getSingletonPtr()->write_all_chat_messages_to_all();
	}

	void PlayState::Cleanup()
	{
		m_SceneMgr->clearScene();
		m_SceneMgr->destroyAllCameras();
		m_RenderWindow->removeAllViewports();
		m_OgreRoot->destroySceneManager(m_SceneMgr);		

		m_OgreRoot = nullptr;
		m_SceneMgr = nullptr;
		m_Camera = nullptr;
		m_CameraSceneNode = nullptr;
		m_RenderWindow = nullptr;
		m_ViewPort = nullptr;

		m_ClientPlayerList.clear();
		m_ClientWallList.clear();
		m_ClientTempWallList.clear();

		if(m_IsHost)
		{
			// CLEANUP BULLET PHYSICS		
			for(unsigned int i = 0; i<m_ServerPlayerList.size(); i++)
			{
				m_DynamicsWorld->removeRigidBody(m_ServerPlayerList.at(i).m_RigidBody);
				delete m_ServerPlayerList.at(i).m_RigidBody;
				delete m_ServerPlayerList.at(i).m_MotionState;
				delete m_ServerPlayerList.at(i).m_vehicleRayCaster;
				delete m_ServerPlayerList.at(i).m_vehicle;
			}
			m_ServerPlayerList.clear();
		
			for(unsigned int i = 0; i<m_ServerWallList.size(); i++)
			{
				m_DynamicsWorld->removeRigidBody(m_ServerWallList.at(i).m_RigidBody);
				delete m_ServerWallList.at(i).m_RigidBody;
				delete m_ServerWallList.at(i).m_MotionState;
			}
			m_ServerWallList.clear();

			delete m_GroundShape;

			delete m_PlayerChassisShape;
			delete m_PlayerWheelShape;
		
			delete m_WallShape;

			for(unsigned int i = 0; i < m_WallCollisionShapes.size(); i++)
			{
				delete m_WallCollisionShapes.at(i);
			}
			m_WallCollisionShapes.clear();

			delete m_DynamicsWorld;
			delete m_Solver;
			delete m_OverlappingPairCache;
			delete m_Dispatcher;
			delete m_CollisionConfiguration;
			// END CLEANUP BULLET PHYSICS

			// Reset player_ptr_
			std::set<participant_ptr> *participants = Server::getSingletonPtr()->get_all_participants();
			std::set<participant_ptr>::iterator itr = participants->begin();
			for(itr; itr != participants->end(); ++itr)
			{
				itr->get()->player_ptr_ = nullptr;
			}

			m_ServerTempWallList.clear();
		}

		wmgr->destroyWindow("LIGHTCYCLEGAME");
		wmgr = nullptr;

		delete m_Console;
		m_Console = nullptr;
	}

	void PlayState::Capture(Ogre::Real time)
	{
		if(m_Console->IsVisible() || m_IsGameOver)
			return;

		if(m_Input->isKeyDown(OIS::KC_I))
		{
			Ogre::Quaternion quat = m_CameraSceneNode->getOrientation();

			Ogre::Vector3 moveDirection = quat.zAxis();
			moveDirection.normalise();
			m_CameraSceneNode->translate(-80 * moveDirection * time);
		}
		if(m_Input->isKeyDown(OIS::KC_K))
		{
			Ogre::Quaternion quat = m_CameraSceneNode->getOrientation();

			Ogre::Vector3 moveDirection = quat.zAxis();
			moveDirection.normalise();
			m_CameraSceneNode->translate(80 * moveDirection * time);
		}
		if(m_Input->isKeyDown(OIS::KC_J))
		{
			m_CameraSceneNode->translate(Ogre::Vector3(-80.0f,0.0f,0.0f) * time, Ogre::Node::TS_LOCAL);
		}
		if(m_Input->isKeyDown(OIS::KC_L))
		{
			m_CameraSceneNode->translate(Ogre::Vector3(80.0f,0.0f,0.0f) * time, Ogre::Node::TS_LOCAL);
		}
		if(m_Input->isKeyDown(OIS::KC_U))
		{
			Ogre::Quaternion quat = m_CameraSceneNode->getOrientation();

			Ogre::Vector3 moveDirection = quat.yAxis();
			moveDirection.normalise();
			m_CameraSceneNode->translate(-80 * moveDirection * time);
		}
		if(m_Input->isKeyDown(OIS::KC_O))
		{
			Ogre::Quaternion quat = m_CameraSceneNode->getOrientation();

			Ogre::Vector3 moveDirection = quat.yAxis();
			moveDirection.normalise();
			m_CameraSceneNode->translate(80 * moveDirection * time);
		}
	}

	inline Ogre::Vector3 NormaliseDirection(Ogre::Vector3 d)
	{
		d.y = 0;
		d.normalise();
		return d;
	}

	bool PlayState::KeyPressed(const OIS::KeyEvent &e)
	{
		// Console is Visible
		if(m_Console->IsVisible())
		{
			switch(e.key)
			{
			case OIS::KC_ESCAPE:
				ShowMenu();
				//m_ShutDown = true;
				break;
			case OIS::KC_RETURN:
			// Continue to next case.
			case OIS::KC_NUMPADENTER:
				{
					if(m_Console->IsActive())
					{
						std::string text = m_Console->GetText();
						text.insert(0, Client::getSingletonPtr()->get_client_name() + ": ");
						Client::getSingletonPtr()->write_chat_message(text);
					}
				}
				break;
			case OIS::KC_GRAVE:
				{
					m_Console->SetVisible(false);
				}
				break;
			default:
				break;
			}

			return true;
		}

		if(m_IsGameOver)
			return true;

		// Console is not visible
		switch(e.key)
		{
		case OIS::KC_ESCAPE:
			ShowMenu();
			break;
		case OIS::KC_RETURN:
			// Continue to next case.
		case OIS::KC_NUMPADENTER:
			break;
		case OIS::KC_W:
			{
				Client::getSingletonPtr()->write_button_pressed_message("w");
				Ogre::Vector3 d = NormaliseDirection(m_CameraSceneNode->getOrientation() *
					Ogre::Vector3::NEGATIVE_UNIT_Z);
				Client::getSingletonPtr()->write_move_direction_message(d);
			}
			break;
		case OIS::KC_S:
			{
				Client::getSingletonPtr()->write_button_pressed_message("s");
				Ogre::Vector3 d = NormaliseDirection(m_CameraSceneNode->getOrientation() *
					Ogre::Vector3::NEGATIVE_UNIT_Z);
				Client::getSingletonPtr()->write_move_direction_message(d);
			}
				break;
		case OIS::KC_A:
			{
				Client::getSingletonPtr()->write_button_pressed_message("a");
				Ogre::Vector3 d = NormaliseDirection(m_CameraSceneNode->getOrientation() *
					Ogre::Vector3::NEGATIVE_UNIT_Z);
				Client::getSingletonPtr()->write_move_direction_message(d);
			}
			break;
		case OIS::KC_D:
			{
				Client::getSingletonPtr()->write_button_pressed_message("d");
				Ogre::Vector3 d = NormaliseDirection(m_CameraSceneNode->getOrientation() *
					Ogre::Vector3::NEGATIVE_UNIT_Z);
				Client::getSingletonPtr()->write_move_direction_message(d);
			}
			break;
		case OIS::KC_Q:
			{
				//Client::getSingletonPtr()->write_button_pressed_message("q");
			}
			break;
		case OIS::KC_E:
			{
				//Client::getSingletonPtr()->write_button_pressed_message("e");
			}
			break;
		case OIS::KC_R:
			{
				Client::getSingletonPtr()->write_button_pressed_message("r");
			}
			break;
		case OIS::KC_X:
			{
				if(m_IsHost && m_IsGameOver)
				{
					Server::getSingletonPtr()->write_reset_game_message();
				}
			}
		case OIS::KC_GRAVE:
			{
				m_Console->SetVisible(true);
			}
			break;
		default:
			break;
		}

		return true;
	}

	bool PlayState::KeyReleased(const OIS::KeyEvent &e)
	{
		switch(e.key)
		{
		case OIS::KC_W:
				Client::getSingletonPtr()->write_button_released_message("w");
			break;
		case OIS::KC_S:
				Client::getSingletonPtr()->write_button_released_message("s");
			break;
		case OIS::KC_A:
				Client::getSingletonPtr()->write_button_released_message("a");
			break;
		case OIS::KC_D:
				Client::getSingletonPtr()->write_button_released_message("d");
			break;
		case OIS::KC_Q:
				Client::getSingletonPtr()->write_button_released_message("q");
			break;
		case OIS::KC_E:
				Client::getSingletonPtr()->write_button_released_message("e");
			break;
		case OIS::KC_R:
				Client::getSingletonPtr()->write_button_released_message("r");
			break;
		default:
			break;
		}

		return true;
	}

	bool PlayState::MousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		return true;
	}

	bool PlayState::MouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		return true;
	}

	bool PlayState::MouseMoved(const OIS::MouseEvent &e)
	{
		if(m_Console->IsVisible() || IsMenuVisible() || m_IsGameOver)
		{
			CEGUI::System::getSingleton().injectMouseMove(float(e.state.X.rel), float(e.state.Y.rel));
			return true;
		}

		Ogre::Quaternion quat = m_CameraSceneNode->getOrientation();
		{
			Ogre::Real a = quat.getPitch().valueDegrees();
			Ogre::Vector3 moveDirection = quat.xAxis();
			moveDirection.normalise();
			m_CameraSceneNode->translate(.75f * moveDirection * float(e.state.X.rel));
			
			if(m_Avatar->m_Direction.angleBetween(moveDirection) > Ogre::Degree(45.f) &&
				LC::Input::getSingletonPtr()->isKeyDown(OIS::KC_W))
			{
				Ogre::Vector3 d = NormaliseDirection(m_CameraSceneNode->getOrientation() *
					Ogre::Vector3::NEGATIVE_UNIT_Z);
				Client::getSingletonPtr()->write_move_direction_message(d);
			}
		}

		{
			Ogre::Vector3 moveDirection = quat.yAxis();
			moveDirection.normalise();
			m_CameraSceneNode->translate(.75f * moveDirection * float(e.state.Y.rel));

			if(m_CameraSceneNode->getPosition().y < 0.f)
				m_CameraSceneNode->translate(-.75f * moveDirection * float(e.state.Y.rel));
			else if(m_CameraSceneNode->getPosition().y > 45.f)
				m_CameraSceneNode->translate(-.75f * moveDirection * float(e.state.Y.rel));
		}

		return true;
	}

	bool PlayState::FrameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		if(m_ShutDown)
		{
			return false;
		}

		if(m_IsHost)
		{
			for(unsigned int i = 0; i<m_ServerPlayerList.size(); i++)
			{
				if(!m_ServerPlayerList.at(i).temp_once && m_ServerPlayerList.at(i).m_IsDead)
				{
					LC::Server::getSingletonPtr()->
						write_a_player_is_dead_message(m_ServerPlayerList.at(i).m_Name);
					m_ServerPlayerList.at(i).temp_once = true;
					m_ServerNumberAlive--;
				}

				m_ServerPlayerList.at(i).update(evt.timeSinceLastFrame);
			}

			m_DynamicsWorld->stepSimulation(evt.timeSinceLastFrame);

			int numManifolds = m_DynamicsWorld->getDispatcher()->getNumManifolds();
			for (int i = 0; i < numManifolds; i++)
			{
				btPersistentManifold *contactManifolds = 
					m_DynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
				btCollisionObject* obA = 
					static_cast<btCollisionObject*>(contactManifolds->getBody0());
				btCollisionObject* obB = 
					static_cast<btCollisionObject*>(contactManifolds->getBody1());

				if(obA->getUserPointer() && obB->getUserPointer())
				{
					LC::GameObject* obA_GO = ((GameObject*)obA->getUserPointer());
					LC::GameObject* obB_GO = ((GameObject*)obB->getUserPointer());

					std::string obA_name = obA_GO->m_Name;
					std::string obB_name = obB_GO->m_Name;

					if(obA_name.substr(0,4) == "Play" && obB_name.substr(0,4) == "Wall")
					{
						ContactSensorCallback callback(*dynamic_cast<LC::ServerPlayer*>(obA_GO)->m_RigidBody);

						m_DynamicsWorld->contactPairTest(obA, obB, callback);
					}
					else if(obA_name.substr(0,4) == "Wall" && obB_name.substr(0,4) == "Play")
					{
						ContactSensorCallback callback(*dynamic_cast<LC::ServerPlayer*>(obB_GO)->m_RigidBody);

						m_DynamicsWorld->contactPairTest(obA, obB, callback);
					}
				}
			}

			// Do this once per game.
			if(m_ServerNumberAlive <= 1 && !m_IsScoreIncremented)
			{
				// Find player that is not dead.
				for(unsigned int i = 0; i < m_ServerPlayerList.size(); i++)
				{
					if(!m_ServerPlayerList.at(i).m_IsDead)
					{
						std::set<participant_ptr> *participants = 
							LC::Server::getSingletonPtr()->get_all_participants();
						std::set<participant_ptr>::iterator itr = participants->begin();
						for(itr; itr != participants->end(); ++itr)
						{
							if(itr->get()->player_ptr_->m_Name == 
								m_ServerPlayerList.at(i).m_Name)
							{
								if(LC::Server::getSingletonPtr()->get_all_participant_names().size() > 1)
								{
									LC::Server::getSingletonPtr()->
										write_victory_message(itr->get()->get_participant_name());
								}
								m_IsScoreIncremented = true;
								itr->get()->score_++;
								break;
							}
						}
						break;
					}
				}
				m_IsGameOver = true;
			}
		}

		for(unsigned int i = 0; i<m_ClientPlayerList.size(); i++)
		{
			// Interpolative.
			m_ClientPlayerList.at(i).update(evt.timeSinceLastFrame);
		}

		if(m_Avatar)
		{
			Ogre::Vector3 APos = m_Avatar->m_SceneNode->getPosition();
			Ogre::Vector3 CPos = m_CameraSceneNode->getPosition();
			Ogre::Vector3 directionVector = CPos - APos;
			directionVector.normalise();
			float tightness = 50.f;
			Ogre::Vector3 newPosition = APos + directionVector * tightness;
			newPosition.y = std::min(CPos.y, tightness);
			m_CameraSceneNode->setPosition(newPosition);
		}

		if(m_Console)
		{
			m_Console->update(evt.timeSinceLastFrame);
		}

		return true;
	}

	void PlayState::HandleChatMessage(std::string text)
	{
		m_Console->HandleChatMessage(text);
	}

	void PlayState::UpdateParticipants(std::vector<std::string> participants)
	{
		/*CEGUI::Listbox *participantsBox =
			static_cast<CEGUI::Listbox*>(wmgr->getWindow("LIGHTCYCLEMENU/Lobby/Participants"));
		participantsBox->resetList();
		std::vector<std::string>::iterator itr = participants.begin();
		for(itr; itr != participants.end(); ++itr)
		{
			CEGUI::ListboxTextItem *newItem = 0;
			newItem = new CEGUI::ListboxTextItem(*itr, CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
			participantsBox->addItem(newItem);
		}*/
	}

	// NOT THREAD SAFE. Server Function.
	std::vector<SnapShotObject> PlayState::GetSnapShot()
	{
		std::vector<SnapShotObject> instances;

		// NEED TO BLOCK m_ServerPlayerList.
		for(unsigned int i=0; i < m_ServerPlayerList.size(); ++i)
		{
			LC::ServerPlayer *player = &m_ServerPlayerList.at(i);

			btTransform transform = player->m_RigidBody->getWorldTransform();

			SnapShotObject newInstance(player->m_Name, transform.getOrigin(),
				transform.getRotation(), player->m_RigidBody->getLinearVelocity());

			instances.push_back(newInstance);			
		}
		for(unsigned int i=0; i < m_ServerTempWallList.size(); ++i)
		{
			ServerTempWall *tempWall = &m_ServerTempWallList.at(i);

			SnapShotObject newInstance(tempWall->m_Name, tempWall->m_Transform.getOrigin(),
				tempWall->m_Transform.getRotation(), tempWall->m_Direction);

			instances.push_back(newInstance);
		}

		return instances;
	}

	// Client Function.
	void PlayState::HandleSnapShot(std::vector<SnapShotObject> instances)
	{
		for(unsigned int i=0; i < instances.size(); ++i)
		{
			bool found = false;
			for(unsigned int j=0; j < m_ClientPlayerList.size(); ++j)
			{
				if(m_ClientPlayerList.at(j).m_Name == instances.at(i).getName())
				{
					m_ClientPlayerList.at(j).update(instances.at(i).getPosition(),
						instances.at(i).getOrientation(), instances.at(i).getVelocity());
					found = true;
					break;
				}
			}
			for(unsigned int j=0; j < m_ClientTempWallList.size(); ++j)
			{
				if(m_ClientTempWallList.at(j).m_Name == instances.at(i).getName())
				{
					m_ClientTempWallList.at(j).update(instances.at(i).getPosition(),
						// Velocity --> direction.
						instances.at(i).getOrientation(), instances.at(i).getVelocity());
					found = true;
					break;
				}
			}
			// CREATE NEW GAME OBJECT
			if(!found)
			{
				Ogre::SceneNode *node = m_SceneMgr->getRootSceneNode()->
					createChildSceneNode(instances.at(i).getName());

				Ogre::Entity *ent;

				if(node->getName().substr(0,4) == "Play")
				{
					ent = m_SceneMgr->createEntity(instances.at(i).getName() + "ent", "Arrow.mesh");
					node->setPosition(instances.at(i).getPosition());
					node->setOrientation(instances.at(i).getOrientation());
					node->pitch(Ogre::Degree(-90.f));
					node->scale(1.f,2.f,0.5f);
					node->attachObject(ent);

					LC::ClientPlayer newPlayer(node->getName(), node, ent, Ogre::Vector3::UNIT_X);
					m_ClientPlayerList.push_back(newPlayer);
				}
				else if(node->getName().substr(0,4) == "Temp")
				{
					ent = m_SceneMgr->createEntity(instances.at(i).getName() + "ent", "Cube.mesh");
					ent->setMaterialName("Textures/CubeRed");
					node->setPosition(instances.at(i).getPosition());
					node->setOrientation(instances.at(i).getOrientation());
					float size = instances.at(i).getVelocity().length()/2;
					node->scale(1.f,1.f,size);

					//1.f,0.5f,2.f
					Ogre::Real CARSHAPEZ = 2.f;
					Ogre::Vector3 offset = instances.at(i).getVelocity().normalisedCopy() * CARSHAPEZ*10;
					node->translate(instances.at(i).getVelocity()/2 - offset);

					node->attachObject(ent);
					
					// Velocity --> Direction.
					LC::ClientTempWall newPlayer(node->getName(), node, ent, instances.at(i).getVelocity());
					m_ClientTempWallList.push_back(newPlayer);
				}
				else
				{
					ent = m_SceneMgr->createEntity(instances.at(i).getName() + "ent", "Bullet.mesh");
					node->scale(2.f,2.5f,2.f);
				}
			}
		}
	}

	// Client Function
	void PlayState::HandleCreateAvatar(SnapShotObject avatar)
	{
		bool found = false;
		for(unsigned int j=0; j < m_ClientPlayerList.size(); ++j)
		{
			if(m_ClientPlayerList.at(j).m_Name == avatar.getName())
			{
				m_Avatar = &m_ClientPlayerList.at(j);

				found = true;
				break;
			}
		}
		// CREATE NEW GAME OBJECT
		if(!found)
		{
			Ogre::SceneNode *node = m_SceneMgr->getRootSceneNode()->createChildSceneNode(
				avatar.getName(), avatar.getPosition(), avatar.getOrientation());
			
			Ogre::Entity *ent = m_SceneMgr->createEntity(avatar.getName(), "Arrow.mesh");
			ent->setMaterialName("Textures/ArrowGreen");
			node->pitch(Ogre::Degree(-90.f));
			node->scale(1.f,2.f,0.5f);
			node->attachObject(ent);			

			LC::ClientPlayer newPlayer(node->getName(), node, ent, Ogre::Vector3::UNIT_X);
			m_ClientPlayerList.push_back(newPlayer);

			m_Avatar = &m_ClientPlayerList.back();

			Ogre::Vector3 p = m_Avatar->m_SceneNode->getPosition();
			m_CameraSceneNode->setPosition(30.f, 30.f, 30.f);

			// The camera will always look at the camera target
			m_CameraSceneNode->setAutoTracking (true, m_Avatar->m_SceneNode);
			// Needed because of auto tracking
            m_CameraSceneNode->setFixedYawAxis (true);
		}
	}

	// Client function
	void PlayState::HandlePlayerIsDead(std::string name)
	{
		std::deque<LC::ClientPlayer>::iterator itr =  m_ClientPlayerList.begin();
		for(itr; itr != m_ClientPlayerList.end(); ++itr)
		{
			if(itr->m_Name == name)
			{
				itr->m_Entity->setVisible(false);
			}
		}
	}

	void PlayState::HandleVictory(std::string name)
	{
		CEGUI::Window *victory = wmgr->getSingletonPtr()->getWindow("LIGHTCYCLEGAME/Victory");
		victory->show();
		victory->setText(name + " Wins!");

		if(!m_IsHost)
		{
			wmgr->getSingletonPtr()->getWindow("LIGHTCYCLEGAME/Victory/Reset")->hide();
		}

		CEGUI::MouseCursor::getSingletonPtr()->show();

		m_IsGameOver = true;
	}

	void PlayState::SetScore(std::vector<std::string> scores)
	{
		if(scores.size() > 0)
		{
			wmgr->getWindow("LIGHTCYCLEGAME/Score/1")->setText(scores.at(0));
			wmgr->getWindow("LIGHTCYCLEGAME/Score/1")->show();
		}
		if(scores.size() > 1)
		{
			wmgr->getWindow("LIGHTCYCLEGAME/Score/2")->setText(scores.at(1));
			wmgr->getWindow("LIGHTCYCLEGAME/Score/2")->show();
		}
		if(scores.size() > 2)
		{
			wmgr->getWindow("LIGHTCYCLEGAME/Score/3")->setText(scores.at(2));
			wmgr->getWindow("LIGHTCYCLEGAME/Score/3")->show();
		}
		if(scores.size() > 3)
		{
			wmgr->getWindow("LIGHTCYCLEGAME/Score/4")->setText(scores.at(3));
			wmgr->getWindow("LIGHTCYCLEGAME/Score/4")->show();
		}
	}

	int numWall = 0;

	void PlayState::CreateWall(btTransform &transform, btVector3 &direction)
	{
		std::string name = "Wall" + boost::lexical_cast<std::string>(numWall);
		numWall++;

		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if(isDynamic)
			m_WallShape->calculateLocalInertia(mass, localInertia);

		btBoxShape *box = dynamic_cast<btBoxShape*>(m_WallShape);
		btVector3 size = box->getHalfExtentsWithoutMargin()*2;

		btDefaultMotionState *motionState = new btDefaultMotionState(transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,m_WallShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		m_DynamicsWorld->addRigidBody(body);
		LC::ServerWall p(name, body, motionState);
		p.m_Direction = direction;
		m_ServerWallList.push_back(p);
		body->setUserPointer(&m_ServerWallList.back());
	}

	void PlayState::CreateWall(ServerTempWall *tempWall)
	{
		// Remove TempWall from Clients
		LC::Server::getSingletonPtr()->write_remove_client_object_message(tempWall->m_Name);

		std::string name = "Wall" + boost::lexical_cast<std::string>(numWall);
		numWall++;
		
		btScalar length = tempWall->m_Direction.length()/2;
		btCollisionShape *newWallShape = new btBoxShape(btVector3(1.f,2.f,length));
		m_WallCollisionShapes.push_back(newWallShape);
		
		btScalar mass(0.f);
		bool isDynamic = (mass != 0.f);
		btVector3 localInertia(0,0,0);
		if(isDynamic)
			newWallShape->calculateLocalInertia(mass, localInertia);

		// GET PROPER SHAPE/SIZE!!! Seems fine.
		btBoxShape *box = dynamic_cast<btBoxShape*>(newWallShape);
		btVector3 size = box->getHalfExtentsWithoutMargin()*2;
		
		btVector3 dir = tempWall->m_Direction;

		btTransform trans = tempWall->m_Transform;
		btVector3 origin = trans.getOrigin();

		btScalar carshapeZ = dynamic_cast<btBoxShape*>(m_PlayerChassisShape)->getHalfExtentsWithoutMargin().z();

		btVector3 offset = dir.normalized() * (carshapeZ*2);

		trans.setOrigin(trans.getOrigin() + dir/2 - offset);

		btDefaultMotionState *motionState = new btDefaultMotionState(trans);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,newWallShape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);
		m_DynamicsWorld->addRigidBody(body);
		LC::ServerWall p(name, body, motionState);
		p.m_Direction = tempWall->m_Direction;
		m_ServerWallList.push_back(p);
		body->setUserPointer(&m_ServerWallList.back());

		btTransform transform = p.m_RigidBody->getWorldTransform();

		SnapShotObject newWall(p.m_Name, transform.getOrigin(),
				transform.getRotation(), p.m_Direction);
		LC::Server::getSingletonPtr()->write_create_wall_message(newWall);
	}

	ServerTempWall* PlayState::StartCreateWall(btTransform &transform, btVector3 &direction)
	{
		if(!m_Avatar)
			return nullptr;

		std::string name = "TempWall" + boost::lexical_cast<std::string>(numWall);
		
		ServerTempWall newTempWall(name, transform, direction);

		m_ServerTempWallList.push_back(newTempWall);

		return &m_ServerTempWallList.back();
	}

	void PlayState::EndCreateWall(ServerTempWall **tempWall)
	{
		std::deque<LC::ServerTempWall>::iterator itr = m_ServerTempWallList.begin();
		for(itr; itr!=m_ServerTempWallList.end();++itr)
		{
			if(itr->m_Name == (*tempWall)->m_Name)
			{

				CreateWall(*tempWall);
				m_ServerTempWallList.erase(itr);
				(*tempWall) = nullptr; // Not needed.				
				return;
			}
		}
		return;
	}

	void PlayState::RemoveServerObject(std::string name)
	{
		if(name.substr(0,4) == "Play")
		{
			std::deque<LC::ServerPlayer>::iterator itr = m_ServerPlayerList.begin();
			for(itr; itr!=m_ServerPlayerList.end();++itr)
			{
				if(itr->m_Name == name)
				{
					m_DynamicsWorld->removeRigidBody(itr->m_RigidBody);
					//btCollisionObject *obj = itr->m_RigidBody;
					//m_DynamicsWorld->removeCollisionObject(obj);
					m_DynamicsWorld->removeAction(itr->m_vehicle);

					delete itr->m_RigidBody;
					delete itr->m_MotionState;
					delete itr->m_vehicleRayCaster;
					delete itr->m_vehicle;
				
					m_ServerPlayerList.erase(itr);

					m_ServerNumberAlive--;

					return;
				}
			}
		}
	}

	void PlayState::RemoveClientObject(std::string name)
	{
		if(name.substr(0,4) == "Temp")
		{
			std::deque<LC::ClientTempWall>::iterator itr = m_ClientTempWallList.begin();
			for(itr; itr!=m_ClientTempWallList.end();++itr)
			{
				if(itr->m_Name == name)
				{
					m_SceneMgr->destroyEntity(itr->m_Entity);
				
					m_ClientTempWallList.erase(itr);

					return;
				}
			}
		}
		else if(name.substr(0,4) == "Play")
		{
			std::deque<LC::ClientPlayer>::iterator itr = m_ClientPlayerList.begin();
			for(itr; itr!=m_ClientPlayerList.end();++itr)
			{
				if(itr->m_Name == name)
				{
					m_SceneMgr->destroyEntity(itr->m_Entity);
				
					m_ClientPlayerList.erase(itr);

					return;
				}
			}
		}
	}

	void PlayState::ClientCreateWall(SnapShotObject newWall)
	{
		Ogre::SceneNode *node = m_SceneMgr->getRootSceneNode()->
			createChildSceneNode(newWall.getName());

		Ogre::Entity *ent;

		ent = m_SceneMgr->createEntity(newWall.getName() + "ent", "Cube.mesh");
		float size = newWall.getVelocity().length()/2;

		Ogre::Vector3 dir = newWall.getVelocity();

		node->setOrientation(newWall.getOrientation());
		node->setPosition(newWall.getPosition());
		node->setScale(1.f,1.f,size);

		node->attachObject(ent);

		LC::ClientWall wall(node->getName(), node, ent, Ogre::Vector3::UNIT_X);
		m_ClientWallList.push_back(wall);
	}

	void PlayState::ResetGame()
	{
		TheApplication.ChangeState(LC::MenuState::getSingletonPtr());
		LC::MenuState::getSingletonPtr()->ShowLobby();
		if(m_IsHost)
		{
			LC::Server::getSingletonPtr()->write_get_names_message();
			LC::Server::getSingletonPtr()->write_all_chat_messages_to_all();

			LC::MenuState::getSingletonPtr()->getWindowManager()->
				getWindow("LIGHTCYCLEMENU/Lobby/ConnectedIP")->setText("IP: SERVER");
		}
		else
		{
			std::string address = LC::Client::getSingletonPtr()->socket().local_endpoint().address().to_string();
			LC::MenuState::getSingletonPtr()->getWindowManager()->
				getWindow("LIGHTCYCLEMENU/Lobby/ConnectedIP")->setText("IP: " + address);
		}
	}

	void PlayState::ShowMenu()
	{
		CEGUI::Window* menuwindow = wmgr->getWindow("LIGHTCYCLEGAME/MenuWindow");
		menuwindow->setVisible(!menuwindow->isVisible());
		CEGUI::MouseCursor *cursor = CEGUI::MouseCursor::getSingletonPtr();
		if(!m_Console->IsVisible())
			cursor->setVisible(!cursor->isVisible());
	}

	bool PlayState::IsMenuVisible()
	{
		return wmgr->getWindow("LIGHTCYCLEGAME/MenuWindow")->isVisible();
	}

	bool PlayState::QuitButton(const CEGUI::EventArgs &e)
	{
		Quit();

		return true;
	}
	
	bool PlayState::ResetButton(const CEGUI::EventArgs &e)
	{
		if(m_IsHost && m_IsGameOver)
		{
			Server::getSingletonPtr()->write_reset_game_message();
		}

		return true;
	}

	void PlayState::Quit()
	{

		if(LC::Client::getSingletonPtr()->is_connected())
		{
			boost::shared_ptr<boost::asio::io_service::strand> strand(
				new boost::asio::io_service::strand(*TheApplication.getIOService()));

			if(m_IsHost)
			{
				strand->post(boost::bind(&Server::write_server_close_message, LC::Server::getSingletonPtr()));
			}

			strand->post(boost::bind(&Client::write_disconnect_message, LC::Client::getSingletonPtr()));
			TheApplication.ChangeState(LC::MenuState::getSingletonPtr());
			LC::Client::getSingletonPtr()->socket().shutdown(boost::asio::socket_base::shutdown_receive);
		}
	}
}