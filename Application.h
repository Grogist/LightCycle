#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <OgreWindowEventUtilities.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "Input.h"
#include "GUI.h"
#include "Client.h"
#include "Server.h"

namespace LC
{
	class GameState;

	class Application : public Ogre::FrameListener, public Ogre::WindowEventListener
	{
	public:
		Application(void);
		~Application(void);

		bool Init(void);
		void Cleanup(void);

		void ChangeState(LC::GameState *gameState);

		boost::shared_ptr<boost::asio::io_service> getIOService() { return m_IO_Service; }
		int getPortNumber() { return m_PortNumber; }

		void UpdateParticipants(std::vector<std::string> participants);
		bool IsHost();
		bool setHost(bool set);

		void ResetIOService();

		LC::GameState *getGameState(void) { return m_CurrentState; }
		
	protected:
		virtual void windowResized(Ogre::RenderWindow *rw);
		virtual void windowClosed(Ogre::RenderWindow *rw);
		virtual bool frameRenderingQueued(const Ogre::FrameEvent &evt);

	private:
		Ogre::Root *m_OgreRoot;
		Ogre::String PluginsCfg;
		Ogre::String ResourcesCfg;
		Ogre::RenderWindow *m_RenderWindow;
		LC::Input *m_Input;
		LC::GUI *m_GUI;

		bool m_IsHost;

		const int m_PortNumber;

		boost::shared_ptr<boost::asio::io_service> m_IO_Service;

		LC::GameState *m_CurrentState;
	};
}

extern LC::Application TheApplication;

#endif