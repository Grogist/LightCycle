#ifndef _MENUSTATE_H_
#define _MENUSTATE_H_

#include <cstdlib>
#include <deque>
#include <exception>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include <OGRE/OgreTimer.h>

#include "GameState.h"
#include "Server.h"
#include "Client.h"

using boost::asio::ip::tcp;

namespace LC
{
	class MenuState : public GameState
	{
	public:
		~MenuState() { };
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
		void ServerClosing();

		void ShowLobby(void);
		void ShowNAMEIPMenu(void);

		// DO NOT LIKE!
		CEGUI::WindowManager *getWindowManager();

		// Returns a pointer to the singleton MenuState;
		static MenuState* getSingletonPtr() { return &m_MenuState; }

	private:
		MenuState();

		bool HostButton(const CEGUI::EventArgs &e);
		bool JoinButton(const CEGUI::EventArgs &e);
		bool CreditsButton(const CEGUI::EventArgs &e);
		bool QuitButton(const CEGUI::EventArgs &e);
		bool ReturnButton(const CEGUI::EventArgs &e);
		bool ContinueButton(const CEGUI::EventArgs &e);
		bool StartButton(const CEGUI::EventArgs &e);

		void ValidateNAMEIP(void);

		bool m_IsHost;

		static MenuState m_MenuState;
	};

}

#endif