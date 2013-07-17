#include "Application.h"
#include "GameState.h"
#include "MenuState.h"

namespace LC
{
	Application::Application(void) : m_CurrentState(nullptr), m_Input(nullptr),
		m_PortNumber(20568), m_IO_Service(new boost::asio::io_service)
	{
	};

	Application::~Application(void)
	{};

	bool Application::Init(void)
	{
		#ifdef _DEBUG
			PluginsCfg = "plugins_d.cfg";
		#else
			PluginsCfg = "plugins.cfg";
		#endif

		#ifdef _DEBUG
			ResourcesCfg = "resources_d.cfg";
		#else
			ResourcesCfg = "resources.cfg";
		#endif

		m_OgreRoot = new Ogre::Root(PluginsCfg);
		Ogre::ConfigFile cf;
		cf.load(ResourcesCfg);

		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		Ogre::String secName, typeName, archName;
		while (seci.hasMoreElements())
		{
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i=settings->begin(); i!=settings->end(); ++i)
			{
				typeName = i->first;
				archName = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName,typeName,secName);
			}
		}
		if(!(m_OgreRoot->restoreConfig() || m_OgreRoot->showConfigDialog()))
		{
			return false;
		}

		m_RenderWindow = m_OgreRoot->initialise(true, "Light Cycle");
		Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		Ogre::WindowEventUtilities::addWindowEventListener(m_RenderWindow, this);
		m_OgreRoot->addFrameListener(this);

		m_OgreRoot->addFrameListener(LC::Server::getSingletonPtr());
		m_OgreRoot->addFrameListener(LC::Client::getSingletonPtr());

		m_Input = LC::Input::getSingletonPtr();
		m_Input->Init(m_RenderWindow);

		m_GUI = LC::GUI::getSingletonPtr();
		m_GUI->Init();

		m_IsHost = false;

		ChangeState(LC::MenuState::getSingletonPtr()); // FIX
		m_OgreRoot->startRendering();

		return true;
	}

	void Application::Cleanup(void)
	{
		m_CurrentState->Cleanup();
		m_CurrentState = nullptr; // Not needed.
	}

	void Application::ChangeState(LC::GameState *gameState)
	{
		if(m_CurrentState)
		{
			m_CurrentState->Cleanup();
			m_CurrentState = nullptr; // Not needed.
		}

		m_CurrentState = gameState;
		m_CurrentState->Init();
	}

	void Application::UpdateParticipants(std::vector<std::string> participants)
	{
		m_CurrentState->UpdateParticipants(participants);
	}

	bool Application::IsHost()
	{
		return m_IsHost;
	}

	bool Application::setHost(bool set)
	{
		m_IsHost = set;
		return m_IsHost;
	}

	void Application::ResetIOService()
	{
		m_IO_Service.reset(new boost::asio::io_service);
	}

	void Application::windowResized(Ogre::RenderWindow *rw)
	{
		unsigned int width, height, depth;
		int left, top;
		rw->getMetrics(width,height,depth,left,top);
		m_Input->SetWindowExtents(width,height);
	}

	void Application::windowClosed(Ogre::RenderWindow *rw)
	{
		if (rw == m_RenderWindow)
		{
			if (m_Input) // FIX
			{
				m_Input->SetWindowClosed();
			}
		}
	}

	bool Application::frameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		if(m_RenderWindow->isClosed())
			return false;

		boost::system::error_code ec;
		m_IO_Service->poll(ec);
		if(ec)
			std::cout << ec.message();

		m_Input->Capture(evt.timeSinceLastFrame);

		m_GUI->InjectTimestamps(evt.timeSinceLastFrame);

		if(m_CurrentState->FrameRenderingQueued(evt))
		{
			return true;
		}
		else
		{
			Cleanup();
			return false;
		}
	}

}