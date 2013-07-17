#include "MenuState.h"

#include "Application.h"
#include "PlayState.h"

namespace LC
{
	MenuState MenuState::m_MenuState;

	MenuState::MenuState() { }

	void MenuState::Init(void)
	{
		m_ShutDown = false;

		m_IsHost = TheApplication.IsHost();

		m_OgreRoot = Ogre::Root::getSingletonPtr();
		m_SceneMgr = m_OgreRoot->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);
		m_Camera = m_SceneMgr->createCamera("MenuCamera");
		m_CameraSceneNode = m_SceneMgr->createSceneNode("MenuCameraSceneNode");
		m_CameraSceneNode->attachObject(m_Camera);
		m_RenderWindow = m_OgreRoot->getAutoCreatedWindow();
		m_ViewPort = m_RenderWindow->addViewport(m_Camera);
		m_ViewPort->setBackgroundColour(Ogre::ColourValue::Black);
		m_Input = LC::Input::getSingletonPtr();
		
		m_Camera->setAspectRatio(Ogre::Real(m_ViewPort->getActualWidth()) / Ogre::Real(m_ViewPort->getActualHeight()));

		wmgr = CEGUI::WindowManager::getSingletonPtr();
		
		CEGUI::Window* MenuInterface = wmgr->loadWindowLayout("LightCycleMenu.layout");

		LC::GUI::getSingletonPtr()->getGUIWindow()->addChildWindow(MenuInterface);

		wmgr->getWindow("LIGHTCYCLEMENU/MainMenu")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Credits")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ReturnButton")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/Text")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/Text")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ContinueButton")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby/Start")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/1")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/2")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/3")->hide();
		wmgr->getWindow("LIGHTCYCLEGAME/Score/4")->hide();

		wmgr->getWindow("LIGHTCYCLEMENU/HostButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::HostButton, this));
		wmgr->getWindow("LIGHTCYCLEMENU/JoinButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::JoinButton, this));
		wmgr->getWindow("LIGHTCYCLEMENU/CreditsButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::CreditsButton, this));
		wmgr->getWindow("LIGHTCYCLEMENU/QuitButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::QuitButton, this));
		wmgr->getWindow("LIGHTCYCLEMENU/ReturnButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::ReturnButton, this));
		wmgr->getWindow("LIGHTCYCLEMENU/ContinueButton")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::ContinueButton, this));
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby/Start")->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MenuState::StartButton, this));
	}

	void MenuState::Cleanup()
	{
		m_SceneMgr->clearScene();
		m_SceneMgr->destroyAllCameras();
		m_OgreRoot->destroySceneManager(m_SceneMgr);
		m_RenderWindow->removeAllViewports();

		m_OgreRoot = nullptr;
		m_SceneMgr = nullptr;
		m_Camera = nullptr;
		m_CameraSceneNode = nullptr;
		m_RenderWindow = nullptr;
		m_ViewPort = nullptr;

		wmgr->destroyWindow("LIGHTCYCLEMENU");
		wmgr = nullptr;
	}

	void MenuState::Capture(Ogre::Real time) { }

	bool MenuState::KeyPressed(const OIS::KeyEvent &e)
	{
		switch(e.key)
		{
		case OIS::KC_ESCAPE:
			m_ShutDown = true;
			break;
		case OIS::KC_RETURN:
			// Continue to next case.
		case OIS::KC_NUMPADENTER:
			{
				if(wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->isActive() ||
					wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->isActive())
				{
					ValidateNAMEIP();					
				}
				else if(wmgr->getWindow("LIGHTCYCLEMENU/Lobby/EditBox")->isActive())
				{
					std::string text = wmgr->getWindow("LIGHTCYCLEMENU/Lobby/EditBox")->getText().c_str();
					wmgr->getWindow("LIGHTCYCLEMENU/Lobby/EditBox")->setText("");
					text.insert(0, Client::getSingletonPtr()->get_client_name() + ": ");
					Client::getSingletonPtr()->write_chat_message(text);
				}
			}
			break;
		}

		return true;
	}

	bool MenuState::KeyReleased(const OIS::KeyEvent &e)
	{
		return true;
	}

	bool MenuState::MousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		return true;
	}

	bool MenuState::MouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		return true;
	}

	bool MenuState::MouseMoved(const OIS::MouseEvent &e)
	{
		CEGUI::System::getSingleton().injectMouseMove(float(e.state.X.rel), float(e.state.Y.rel));
		return true;
	}

	bool MenuState::FrameRenderingQueued(const Ogre::FrameEvent &evt)
	{
		if(m_ShutDown)
		{
			return false;
		}
		return true;
	}

	void MenuState::HandleChatMessage(std::string text)
	{
		CEGUI::Listbox *chatBox = static_cast<CEGUI::Listbox*>(wmgr->getWindow("LIGHTCYCLEMENU/Lobby/ListBox"));
		CEGUI::ListboxTextItem *newItem = 0;
		newItem = new CEGUI::ListboxTextItem(text, CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
		chatBox->addItem(newItem);
		chatBox->ensureItemIsVisible(newItem);
	}

	void MenuState::UpdateParticipants(std::vector<std::string> participants)
	{
		CEGUI::Listbox *participantsBox =
			static_cast<CEGUI::Listbox*>(wmgr->getWindow("LIGHTCYCLEMENU/Lobby/Participants"));
		participantsBox->resetList();
		std::string scores = ("LIGHTCYCLEGAME/Score/");
		std::vector<std::string>::iterator itr = participants.begin();
		int i = 1;
		for(itr, i; itr != participants.end(); ++itr, i++)
		{
			CEGUI::ListboxTextItem *newItem = 0;
			newItem = new CEGUI::ListboxTextItem(*itr, CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
			participantsBox->addItem(newItem);
		}
	}

	void MenuState::ServerClosing()
	{

	}

	bool MenuState::HostButton(const CEGUI::EventArgs &e)
	{
		m_IsHost = TheApplication.setHost(true);
		ShowNAMEIPMenu();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->setText("Server");
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->setText("");
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->disable();
		return true;
	}

	bool MenuState::JoinButton(const CEGUI::EventArgs &e)
	{
		m_IsHost = TheApplication.setHost(false);
		ShowNAMEIPMenu();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->setText("Client");
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->setText("127.0.0.1");
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->enable();		
		return true;
	}

	bool MenuState::CreditsButton(const CEGUI::EventArgs &e)
	{
		wmgr->getWindow("LIGHTCYCLEMENU/MainMenu")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Credits")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/ReturnButton")->show();
		return true;
	}

	bool MenuState::QuitButton(const CEGUI::EventArgs &e)
	{
		m_ShutDown = true;
		return true;
	}

	bool MenuState::ReturnButton(const CEGUI::EventArgs &e)
	{
		wmgr->getWindow("LIGHTCYCLEMENU/MainMenu")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Credits")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ReturnButton")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/Text")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/Text")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ContinueButton")->hide();

		CEGUI::Listbox *chatBox = static_cast<CEGUI::Listbox*>(wmgr->getWindow("LIGHTCYCLEMENU/Lobby/ListBox"));
		chatBox->resetList();
		CEGUI::Listbox *nameBox = static_cast<CEGUI::Listbox*>(wmgr->getWindow("LIGHTCYCLEMENU/Lobby/Participants"));
		nameBox->resetList();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby/EditBox")->setText("");

		if(LC::Client::getSingletonPtr()->is_connected())
		{
			boost::shared_ptr<boost::asio::io_service::strand> strand(
				new boost::asio::io_service::strand(*TheApplication.getIOService()));

			if(m_IsHost)
			{
				strand->post(boost::bind(&Server::write_server_close_message, LC::Server::getSingletonPtr()));
			}

			strand->post(boost::bind(&Client::write_disconnect_message, LC::Client::getSingletonPtr()));

			// Need to make sure these ^ messages are sent before any call to TheApplication.ResetIOService()
			//  is made.
		}

		m_IsHost = TheApplication.setHost(false);

		return true;
	}

	bool MenuState::ContinueButton(const CEGUI::EventArgs &e)
	{
		ValidateNAMEIP();
		return true;
	}

	bool MenuState::StartButton(const CEGUI::EventArgs &e)
	{
		if(m_IsHost)
		{
			Server::getSingletonPtr()->write_game_start_message();
		}
		return true;
	}

	void MenuState::ShowLobby()
	{
		wmgr->getWindow("LIGHTCYCLEMENU/MainMenu")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/Credits")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ReturnButton")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/Text")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/Text")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ContinueButton")->hide();
		if(m_IsHost)
		{
			wmgr->getWindow("LIGHTCYCLEMENU/Lobby/Start")->show();
		}
	}

	void MenuState::ShowNAMEIPMenu()
	{
		wmgr->getWindow("LIGHTCYCLEMENU/MainMenu")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Lobby")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/Credits")->hide();
		wmgr->getWindow("LIGHTCYCLEMENU/ReturnButton")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/Text")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/Text")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->show();
		wmgr->getWindow("LIGHTCYCLEMENU/ContinueButton")->show();
	}

	void MenuState::ValidateNAMEIP()
	{
		// This shouldn't be here!!!
		// Socket needs to be closed/reset before IO_Service is reset.
		if(LC::Client::getSingletonPtr()->socket_)
		{
			boost::shared_ptr<tcp::socket> sockets;
			sockets = LC::Client::getSingletonPtr()->socket_;
			boost::system::error_code error;
			sockets->shutdown(boost::asio::socket_base::shutdown_both, error);
			sockets->close(error);
		}
		LC::Client::getSingletonPtr()->socket_.reset();
		
		// May be overly drastic.
		TheApplication.ResetIOService();

		if(m_IsHost)
		{
			// Create Server.
			std::string userName = wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->getText().c_str();
			Server::getSingletonPtr()->reset(TheApplication.getIOService(), userName);

			// Create Client.
			// Need better way of doing this.
			tcp::resolver resolver(*TheApplication.getIOService());
			std::ostringstream ss;
			int portnumber = TheApplication.getPortNumber();
			ss << portnumber;
			tcp::resolver::query query("127.0.0.1", ss.str().c_str());
			tcp::resolver::iterator iterator = resolver.resolve(query);
			Client::getSingletonPtr()->reset(TheApplication.getIOService(), iterator, userName);
			
			wmgr->getWindow("LIGHTCYCLEMENU/Lobby/ConnectedIP")->setText("IP: SERVER");
			wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->setText("");
			wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->setText("");
		}
		else
		{
			std::string text = wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->getText().c_str();

			// Check for valid IP address.
			boost::system::error_code ec;
			boost::asio::ip::address address = boost::asio::ip::address::from_string(text, ec);

			std::string userName = wmgr->getWindow("LIGHTCYCLEMENU/Name/EditBox")->getText().c_str();

			if(!ec && userName.length()>=1)
			{
				CEGUI::String IPAddress = wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->getText();

				// Need better way of doing this.
				tcp::resolver resolver(*TheApplication.getIOService());
				std::ostringstream ss;
				int portnumber = TheApplication.getPortNumber();
				ss << portnumber;
				tcp::resolver::query query(IPAddress.c_str(), ss.str().c_str());
				tcp::resolver::iterator iterator = resolver.resolve(query);
				Client::getSingletonPtr()->reset(TheApplication.getIOService(), iterator, userName);

				wmgr->getWindow("LIGHTCYCLEMENU/Lobby/ConnectedIP")->setText("IP: " + address.to_string());
			}
			else if(ec)
			{
				wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox")->setText("Invalid Address");
				CEGUI::Editbox *editBox = static_cast<CEGUI::Editbox*>(wmgr->getWindow("LIGHTCYCLEMENU/IP/EditBox"));
				editBox->setCaratIndex(editBox->getText().length());
			}
		}
	}

	CEGUI::WindowManager *MenuState::getWindowManager()
	{
		return wmgr;
	}
}