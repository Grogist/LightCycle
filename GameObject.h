#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btTransform.h>

namespace LC
{
	class GameObject
	{
	public:
		~GameObject() {};

		std::string m_Name;

		virtual void update(Ogre::Real time) = 0;

	protected:
		GameObject(std::string name) : m_Name(name) {};
	};

	class ServerGameObject : public GameObject
	{
	public:
		~ServerGameObject() {};

		btRigidBody *m_RigidBody;
		// DON'T NEED TO KEEP TRACK OF THIS.
		btDefaultMotionState *m_MotionState;

		btVector3 m_Direction;

		virtual void update(Ogre::Real time) = 0;

	protected:
		ServerGameObject(std::string name, btRigidBody *body, btDefaultMotionState *motionstate) : 
			GameObject(name), m_RigidBody(body), m_MotionState(motionstate),
				m_Direction(0.f,0.f,0.f) { };
	};

	class ServerWall : public ServerGameObject
	{
	public:
		ServerWall(std::string name, btRigidBody *body, btDefaultMotionState *motionstate);
		~ServerWall();

		virtual void update(Ogre::Real time);
	};

	class ServerTempWall : public ServerGameObject
	{
	public:
		ServerTempWall(std::string name, btTransform &transform, btVector3 &direction);
		~ServerTempWall();

		btTransform m_Transform;
		btVector3 m_Direction; // Size = Magnitude of m_Direction.

		virtual void update(Ogre::Real time);
		virtual void update(btVector3 &newEndPostion);
	};

	// Should be Player.
	// ServerPlayer exists only on the server and is authoritative with the corresponding
	//   ClientPlayer which exists on the client.
	class ServerPlayer : public ServerGameObject
	{
	public:
		ServerPlayer(std::string name, btRigidBody *body, btDefaultMotionState *motionstate,
			btDiscreteDynamicsWorld *m_DynamicsWorld);
		~ServerPlayer();

		bool isKeyPressed(std::string key);

		void KeyPressed(std::string key);
		void KeyReleased(std::string key);

		void SetMoveDirection(std::vector<float> direction);

		virtual void update(Ogre::Real time);

		btRaycastVehicle::btVehicleTuning m_tuning;
		btVehicleRaycaster*	m_vehicleRayCaster;
		btRaycastVehicle* m_vehicle;

		float	m_EngineForce;
		float	m_BreakingForce;
		float	m_VehicleSteering;

		static const float	wheelRadius;
		static const float	wheelWidth;

		ServerTempWall *m_TempWall;

		bool m_IsDead;
		bool temp_once;

		std::map<std::string, bool> m_KeyStates;
	};

	class ClientGameObject : public GameObject
	{
	public:
		~ClientGameObject() {};

		Ogre::Vector3 m_Velocity;
		Ogre::Vector3 m_Direction;

		Ogre::SceneNode *m_SceneNode;
		Ogre::Entity *m_Entity;

		virtual void update(Ogre::Real time) = 0;
		
	protected:
		ClientGameObject(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity, Ogre::Vector3 direction) : 
			GameObject(name), m_SceneNode(node), m_Entity(entity), m_Direction(direction)
		{
			m_Velocity = Ogre::Vector3::ZERO;
		};
	};
	
	// Should be GameObject. This applies to all moveable objects in the game.
	//  Other classes can inherit.
	class ClientPlayer : public ClientGameObject
	{
	public:
		ClientPlayer(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity, Ogre::Vector3 direction);
		~ClientPlayer();

		// Interpolative Update.
		virtual void update(Ogre::Real time);
		// Authoritative Update.
		void update(Ogre::Vector3 position, Ogre::Quaternion orientation, Ogre::Vector3 velocity);

	private:
	};

	class ClientWall : public ClientGameObject
	{
	public:
		ClientWall(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity, Ogre::Vector3 direction);
		~ClientWall(){};

		// Should be called. Walls don't update.
		virtual void update(Ogre::Real time){};

	private:
	};

	class ClientTempWall : public ClientGameObject
	{
	public:
		ClientTempWall(std::string name, Ogre::SceneNode *node, Ogre::Entity *entity, Ogre::Vector3 direction);
		~ClientTempWall(){};

		// Interpolative Update.
		virtual void update(Ogre::Real time){};
		// Authoritative Update.
		void update(Ogre::Vector3 startPosition, Ogre::Quaternion orientation, Ogre::Vector3 direction);

	private:
	};
}

#endif