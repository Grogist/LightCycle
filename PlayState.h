#ifndef _PLAYSTATE_H_
#define _PLAYSTATE_H_

#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreMovablePlane.h>
#include <OgrePlane.h>

#include <boost/lexical_cast.hpp>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision\CollisionDispatch\btGhostObject.h>

#include "GameState.h"
#include "Client.h"
#include "Server.h"
#include "GameObject.h"
#include "GameConsole.h"

#include "ContactCallback.h"

using boost::asio::ip::tcp;

namespace LC
{
	class PlayState : public GameState
	{
	public:
		~PlayState() { };
		void Init();
		void Cleanup();

		void Capture(Ogre::Real time);
		bool KeyPressed(const OIS::KeyEvent &e);
		bool KeyReleased(const OIS::KeyEvent &e);

		bool MousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool MouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool MouseMoved(const OIS::MouseEvent &e);

		bool FrameRenderingQueued(const Ogre::FrameEvent &evt);

		void HandleChatMessage(std::string text);
		void UpdateParticipants(std::vector<std::string> participants);
		// NOT THREAD SAFE. Server Funtion.
		std::vector<SnapShotObject> GetSnapShot();
		// Client Function.
		void HandleSnapShot(std::vector<SnapShotObject> instances);
		//Client Function.
		void HandleCreateAvatar(SnapShotObject avatar);
		void HandlePlayerIsDead(std::string name);
		void HandleVictory(std::string name);
		void SetScore(std::vector<std::string> scores);

		// direction is the long axis of the wall.
		void CreateWall(btTransform &transform, btVector3 &direction);
		void CreateWall(ServerTempWall *tempWall);
		ServerTempWall* StartCreateWall(btTransform &transform, btVector3 &direction);
		void EndCreateWall(ServerTempWall **tempWall);
		void ClientCreateWall(SnapShotObject newWall);
		// Should be HandleRemoveTempWall(std::string name);
		void RemoveServerObject(std::string name);
		void RemoveClientObject(std::string name);

		void ResetGame();

		// Returns a pointer to the singleton MenuState;
		static PlayState* getSingletonPtr() { return &m_PlayState; }

	private:
		PlayState();

		bool m_IsHost;

		bool m_IsGameOver;
		// LAZY!
		bool m_IsScoreIncremented;

		GameConsole *m_Console;

		void ShowMenu();
		bool IsMenuVisible();

		void SetScore();

		// Quits the game.
		bool QuitButton(const CEGUI::EventArgs &e);
		// Returns the game to the lobby after someone have won.
		bool ResetButton(const CEGUI::EventArgs &e);

		void Quit();

		// Authoritative. Objects not in Scene Graph
		std::deque<LC::ServerPlayer> m_ServerPlayerList;
		std::deque<LC::ServerWall> m_ServerWallList;
		std::deque<LC::ServerTempWall> m_ServerTempWallList;
		std::deque<LC::ServerWall> m_ServerSurroundingWalls;

		int m_ServerNumberAlive;

		// Interpolative. Objects are in Scene Graph.
		std::deque<LC::ClientPlayer> m_ClientPlayerList;
		std::deque<LC::ClientWall> m_ClientWallList;
		std::deque<LC::ClientTempWall> m_ClientTempWallList;
		std::deque<LC::ClientWall> m_ClientSurroundingWalls;

		LC::ClientPlayer *m_Avatar;

		btDefaultCollisionConfiguration *m_CollisionConfiguration;
		btCollisionDispatcher *m_Dispatcher;
		btBroadphaseInterface *m_OverlappingPairCache;
		btSequentialImpulseConstraintSolver *m_Solver;
		btDiscreteDynamicsWorld *m_DynamicsWorld;

		btCollisionShape *m_GroundShape;
		btDefaultMotionState *m_GroundMotionState;
		btRigidBody *m_GroundRigidBody;

		btCollisionShape *m_PlayerChassisShape;
		btCollisionShape *m_PlayerWheelShape;
		btCollisionShape *m_WallShape;
		std::vector<btCollisionShape*> m_WallCollisionShapes;
		btCompoundShape *m_Compound;

		static PlayState m_PlayState;

	};
}

#endif