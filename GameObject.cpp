#include "GameObject.h"

#include "PlayState.h"

namespace LC
{
	const float	gEngineForce = 0.f;
	const float	gBreakingForce = 0.f;

	const float	maxEngineForce = 1000.f;//this should be engine/velocity dependent
	const float	maxBreakingForce = 100.f;

	const float	steeringClamp = 0.3f;
	const float steeringIncrement = 0.04f;
	
	const float	wheelFriction = 1000;//BT_LARGE_FLOAT;
	const float	suspensionStiffness = 20.f;
	const float	suspensionDamping = 2.3f;
	const float	suspensionCompression = 4.4f;
	const float	rollInfluence = 0.1f;//1.0f;*/
	const float connectionHeight = 1.2f;

	const int CUBE_HALF_EXTENTS = 1;
	const int rightIndex = 0;
	const int upIndex = 1;
	const int forwardIndex = 2;

	const btVector3 wheelDirectionCS0(0,-1,0);
	const btVector3 wheelAxleCS(-1,0,0);
	const btScalar suspensionRestLength(0.6f);

	const float ServerPlayer::wheelRadius = 0.5f;
	const float ServerPlayer::wheelWidth = 0.4f;

	ServerPlayer::ServerPlayer(std::string name, btRigidBody *body, btDefaultMotionState *motionstate, 
		btDiscreteDynamicsWorld *m_DynamicsWorld) : ServerGameObject(name, body, motionstate),
		m_vehicleRayCaster(nullptr), m_vehicle(nullptr), m_TempWall(nullptr), m_IsDead(false), temp_once(false)
	{
		m_KeyStates.insert(std::pair<std::string, bool>("w", false));
		m_KeyStates.insert(std::pair<std::string, bool>("s", false));
		m_KeyStates.insert(std::pair<std::string, bool>("a", false));
		m_KeyStates.insert(std::pair<std::string, bool>("d", false));
		m_KeyStates.insert(std::pair<std::string, bool>("q", false));
		m_KeyStates.insert(std::pair<std::string, bool>("e", false));
		m_KeyStates.insert(std::pair<std::string, bool>("r", false));
		m_Direction = btVector3(0.f,0.f,1.f);

		body->setActivationState(DISABLE_DEACTIVATION);

		m_EngineForce = 0.f;
		m_BreakingForce = 0.f;
		m_VehicleSteering = 0.f;

		m_vehicleRayCaster = new btDefaultVehicleRaycaster(m_DynamicsWorld);
		m_vehicle = new btRaycastVehicle(m_tuning, m_RigidBody, m_vehicleRayCaster);

		m_vehicle->setCoordinateSystem(rightIndex,upIndex,forwardIndex);

		bool isFrontWheel = true;
		btVector3 ConnectionPointCS0(CUBE_HALF_EXTENTS-(0.3f*wheelWidth),
			connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
		m_vehicle->addWheel(ConnectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius,
			m_tuning, isFrontWheel);

		ConnectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3f*wheelWidth),
			connectionHeight,2*CUBE_HALF_EXTENTS-wheelRadius);
		m_vehicle->addWheel(ConnectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius,
			m_tuning, isFrontWheel);

		isFrontWheel = false;
		ConnectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS+(0.3f*wheelWidth),
			connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
		m_vehicle->addWheel(ConnectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, 
			m_tuning, isFrontWheel);

		ConnectionPointCS0 = btVector3(CUBE_HALF_EXTENTS-(0.3f*wheelWidth),
			connectionHeight,-2*CUBE_HALF_EXTENTS+wheelRadius);
		m_vehicle->addWheel(ConnectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength, wheelRadius, 
			m_tuning, isFrontWheel);

		for(int i = 0; i<m_vehicle->getNumWheels(); i++)
		{
			btWheelInfo &wheel = m_vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;
		}

		m_DynamicsWorld->addVehicle(m_vehicle);
	}

	ServerPlayer::~ServerPlayer(){}

	bool ServerPlayer::isKeyPressed(std::string key)
	{
		if(m_KeyStates.find(key) != m_KeyStates.end())
		{
			return m_KeyStates[key];
		}
		return false;
	}

	void ServerPlayer::KeyPressed(std::string key)
	{
		if(m_KeyStates.find(key) != m_KeyStates.end())
			m_KeyStates[key] = true;
		if(key == "r" && !m_IsDead)
		{
			if(!m_TempWall)
			{
				btTransform t;
				m_MotionState->getWorldTransform(t);

				// Need to strip out y component from transform.
				btQuaternion q = t.getRotation();

				float pitch = std::atan2(2*q.y()*q.w()-2*q.x()*q.z(), 1-2*q.y()*q.y()-2*q.z()*q.z());
				//float yaw = std::asin(2*q.x()*q.y() + 2*q.z()*q.w());
				float roll = std::atan2(2*q.x()*q.w() - 2*q.y()*q.z(), 1-2*q.x()*q.x() - 2*q.z()*q.z());

				q.setEuler(pitch,0.f,roll);
				t.setRotation(q);
				
				m_Direction = m_RigidBody->getLinearVelocity();

				m_TempWall = PlayState::getSingletonPtr()->StartCreateWall(t, m_Direction.normalize());
			}
			else if(m_TempWall)
			{
				PlayState::getSingletonPtr()->EndCreateWall(&m_TempWall);
				m_TempWall = nullptr;
			}
		}		
	}

	void ServerPlayer::KeyReleased(std::string key)
	{
		if(m_KeyStates.find(key) != m_KeyStates.end())
			m_KeyStates[key] = false;
	}

	void ServerPlayer::SetMoveDirection(std::vector<float> direction)
	{
		if(direction.size() < 3)
			return;
		m_Direction = btVector3(direction[0],direction[1],direction[2]);
	}

	static const float HALF_PI = 1.5707f;

	void ServerPlayer::update(Ogre::Real time)
	{
		// MOVE TO KEYPRESSED.
		if(m_KeyStates["w"] == true)
		{
			m_EngineForce = maxEngineForce;
			m_BreakingForce = 0.f;
		}
		if(m_KeyStates["s"] == true)
		{
			m_EngineForce = -maxEngineForce;
		}
		if(!m_KeyStates["w"] && !m_KeyStates["s"])
		{
			m_EngineForce = 0.f;//maxEngineForce/2;
		}
		if(m_KeyStates["a"] == true)
		{
			m_VehicleSteering += steeringIncrement;
			if(m_VehicleSteering > steeringClamp)
				m_VehicleSteering = steeringClamp;

			if(m_TempWall)
			{
				PlayState::getSingletonPtr()->EndCreateWall(&m_TempWall);
				m_TempWall = nullptr;
			}
		}
		if(m_KeyStates["d"] == true)
		{
			m_VehicleSteering -= steeringIncrement;
			if(m_VehicleSteering < steeringClamp)
				m_VehicleSteering = -steeringClamp;

			if(m_TempWall)
			{
				PlayState::getSingletonPtr()->EndCreateWall(&m_TempWall);
				m_TempWall = nullptr;
			}
		}
		if(!m_KeyStates["a"] && !m_KeyStates["d"])
		{
			m_VehicleSteering = 0.f;
		}
		if(m_KeyStates["q"] == true)
		{
			m_RigidBody->setLinearVelocity(btVector3(0.f,40.f,0.f));
		}
		if(m_KeyStates["e"] == true)
		{
			m_RigidBody->setLinearVelocity(btVector3(0.f,-40.f,0.f));
		}

		if(m_TempWall)
		{
			btTransform t;
			t.setIdentity();
			m_MotionState->getWorldTransform(t);

			m_TempWall->update(t.getOrigin());
		}

		int wheelIndex = 2;
		m_vehicle->applyEngineForce(m_EngineForce,wheelIndex);
		m_vehicle->setBrake(m_BreakingForce,wheelIndex);
		wheelIndex = 3;
		m_vehicle->applyEngineForce(m_EngineForce,wheelIndex);
		m_vehicle->setBrake(m_BreakingForce,wheelIndex);

		wheelIndex = 0;
		m_vehicle->setSteeringValue(m_VehicleSteering,wheelIndex);
		wheelIndex = 1;
		m_vehicle->setSteeringValue(m_VehicleSteering,wheelIndex);
	}

	ServerWall::ServerWall(std::string name, btRigidBody *body, btDefaultMotionState *motionstate)
		: ServerGameObject(name, body, motionstate) { }

	ServerWall::~ServerWall() { }

	void ServerWall::update(Ogre::Real time) { }

	ServerTempWall::ServerTempWall(std::string name, btTransform &transform, btVector3 &direction)
		: ServerGameObject(name, nullptr, nullptr), m_Transform(transform),
		  m_Direction(direction) { }

	ServerTempWall::~ServerTempWall() { }

	void ServerTempWall::update(Ogre::Real time) { }

	void ServerTempWall::update(btVector3 &newEndPosition)
	{
		m_Direction = newEndPosition - m_Transform.getOrigin();
		m_Direction.setY(0.f);
	}

	ClientPlayer::ClientPlayer(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity, Ogre::Vector3 direction)
		: ClientGameObject(name, node, entity, direction) { };

	ClientPlayer::~ClientPlayer() { };

	// Interpolative Update.
	void ClientPlayer::update(Ogre::Real time)
	{
		Ogre::Vector3 normalizedVelocity = m_Velocity.normalisedCopy();
		m_SceneNode->translate(normalizedVelocity*time);
	}
	
	// Authoritative Update.
	void ClientPlayer::update(Ogre::Vector3 position, Ogre::Quaternion orientation, Ogre::Vector3 velocity)
	{
		//if(position.distance(m_SceneNode->getPosition()) > 5.0f)
		{
			Ogre::Vector3 nodePosition = m_SceneNode->getPosition();
			m_SceneNode->setPosition(position);
		}

		m_SceneNode->setOrientation(orientation);

		if(m_SceneNode->getName().substr(0,4) == "Play")
			m_SceneNode->pitch(Ogre::Degree(-90.f));

		m_Velocity = velocity;

		if(velocity != Ogre::Vector3::ZERO)
		{
			m_Direction = velocity.normalisedCopy();
		}
	}

	ClientWall::ClientWall(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity,
		Ogre::Vector3 direction) : ClientGameObject(name, node, entity, direction) { }

	ClientTempWall::ClientTempWall(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity,
		Ogre::Vector3 direction) : ClientGameObject(name, node, entity, direction) { }

	void ClientTempWall::update(Ogre::Vector3 startPosition, Ogre::Quaternion orientation,
		Ogre::Vector3 direction)
	{
		m_Direction = direction;
		// Hackish.
		m_Direction.y = -2.5f;
		startPosition.y = 0.f;

		float size = direction.length();
		m_SceneNode->setPosition(startPosition);
		m_SceneNode->setScale(1.f,1.f,size/2);

		//1.f,0.5f,2.f
		Ogre::Real CARSHAPEZ = 2.f;
		Ogre::Vector3 offset = direction.normalisedCopy() * (CARSHAPEZ*2);
		m_SceneNode->translate(m_Direction/2-offset);
	}
}