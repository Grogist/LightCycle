#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include <OgreMath.h>
#include <OIS.h>

#include "Application.h"
#include "Input.h"

namespace LC
{
	class GameState
	{
	public:
		virtual ~GameState() {};

		// Is called when entering a GameState;
		virtual void Init(void) = 0;
		// Is called when leaving a GameState
		virtual void Cleanup(void) = 0;

		// Captures and responds to keyboard input.
		// CaptureInput is called per frame.
		virtual void Capture(Ogre::Real time) = 0;
		virtual bool KeyPressed(const OIS::KeyEvent &e) = 0;
		virtual bool KeyReleased(const OIS::KeyEvent &e)= 0;

		// Responds to mouse input.
		// Mousepressed and mouseReleased are called on a mouse button event.
		virtual bool MousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;
		virtual bool MouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id) = 0;
		// MouseMoved is called whenever the mouse is moved.
		virtual bool MouseMoved(const OIS::MouseEvent &e) = 0;

		// Is called everyframe to update a GameState.
		virtual bool FrameRenderingQueued(const Ogre::FrameEvent &evt) = 0;

		virtual void HandleChatMessage(std::string text) = 0;
		virtual void UpdateParticipants(std::vector<std::string> participants) = 0;

		void SetShutDown(bool shutdown)
		{
			m_ShutDown = shutdown;
		}

		// Changes the current GameState.
		void ChangeState(GameState* state)
		{
			TheApplication.ChangeState(state);
		}

	protected:
		GameState() {};

		CEGUI::WindowManager *wmgr;
		
		Ogre::Root *m_OgreRoot;
		Ogre::SceneManager *m_SceneMgr;
		Ogre::Camera *m_Camera;
		Ogre::SceneNode *m_CameraSceneNode;
		Ogre::Viewport *m_ViewPort;
		Ogre::RenderWindow *m_RenderWindow;
		LC::Input *m_Input;

		bool m_ShutDown;
	};
}

#endif