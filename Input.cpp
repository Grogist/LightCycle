#include "Input.h"

#include "Application.h"
#include "GameState.h"

CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
	switch(buttonID)
	{
	case OIS::MB_Left:
		return CEGUI::LeftButton;
	case OIS::MB_Right:
		return CEGUI::RightButton;
	case OIS::MB_Middle:
		return CEGUI::MiddleButton;
	default:
		return CEGUI::LeftButton;
	}
}

namespace LC
{
	const static float Repeat_Delay = 0.05f;
	const static float Initial_Delay = 0.3f;

	Input Input::m_Input;

	Input::Input() : m_Mouse(nullptr), m_Keyboard(nullptr), m_InputManager(nullptr),
		m_Key(OIS::KC_UNASSIGNED)
	{ }

	Input::~Input()
	{
		if(m_InputManager)
		{
			if(m_Mouse)
			{
				m_InputManager->destroyInputObject(m_Mouse);
			}
			if(m_Keyboard)
			{
				m_InputManager->destroyInputObject(m_Keyboard);
			}
		}

		m_InputManager->destroyInputSystem(m_InputManager);
		m_InputManager = 0;
	}

	void Input::Init(Ogre::RenderWindow *renderWindow)
	{
		if(!m_InputManager)
		{
			OIS::ParamList pl;
			size_t windowHnd = 0;
			std::ostringstream windowHndStr;

			renderWindow->getCustomAttribute("WINDOW", &windowHnd);

			windowHndStr << (unsigned int) windowHnd;
			pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

			m_InputManager = OIS::InputManager::createInputSystem(pl);

			m_Keyboard = static_cast<OIS::Keyboard*>(m_InputManager->createInputObject( OIS::OISKeyboard, false ));
			m_Keyboard->setEventCallback(this);
			m_Keyboard->setBuffered(true);
			m_Mouse = static_cast<OIS::Mouse*>(m_InputManager->createInputObject( OIS::OISMouse, false ));
			m_Mouse->setEventCallback(this);
			m_Mouse->setBuffered(true);

			// Get Window Size
			unsigned int width, height, depth;
			int left, top;
			renderWindow->getMetrics(width, height, depth, left, top);

			SetWindowExtents(width,height);

			// INCREDIBLY HACKISH!!!
			OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(m_Mouse->getMouseState());
			mutableMouseState.X.abs = 1;
			mutableMouseState.Y.abs = 1;
		}
	}

	void Input::SetWindowExtents(int width, int height)
	{
		m_Mouse->getMouseState().width = width;
		m_Mouse->getMouseState().height = height;
	}

	void Input::SetWindowClosed(void)
	{
		if (m_InputManager)
		{
			m_InputManager->destroyInputObject(m_Mouse);
			m_InputManager->destroyInputObject(m_Keyboard);

			OIS::InputManager::destroyInputSystem(m_InputManager);
			m_InputManager = 0;
		}
	}

	void Input::Capture(Ogre::Real time)
	{
		if(m_Mouse) m_Mouse->capture();

		if(m_Keyboard) m_Keyboard->capture();

		UpdateRepeatKey(time);

		TheApplication.getGameState()->Capture(time);
	}

	bool Input::keyPressed(const OIS::KeyEvent &e)
	{
		TheApplication.getGameState()->KeyPressed(e);

		if(e.key == OIS::KC_GRAVE)
			return true;

		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.injectKeyDown(e.key);
		sys.injectChar(e.text);
		BeginRepeatKey(e);

		return true;
	}
	
	bool Input::keyReleased(const OIS::KeyEvent &e)
	{
		CEGUI::System::getSingleton().injectKeyUp(e.key);
		EndRepeatKey(e);

		TheApplication.getGameState()->KeyReleased(e);

		return true;
	}

	bool Input::mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));

		TheApplication.getGameState()->MousePressed(e, id);

		return true;
	}

	bool Input::mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id)
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));

		TheApplication.getGameState()->MouseReleased(e, id);

		return true;
	}

	bool Input::mouseMoved(const OIS::MouseEvent &e)
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();

		TheApplication.getGameState()->MouseMoved(e);

		return true;
	}

	// AUTO REPEAT KEY
	void Input::BeginRepeatKey(const OIS::KeyEvent &evt)
	{
		m_Key = evt.key;
		m_Char = evt.text;

		m_Elapsed = 0.f;
		m_Delay = Initial_Delay;
	}

	void Input::EndRepeatKey(const OIS::KeyEvent &evt)
	{
		if(m_Key != evt.key) return;

		m_Key = OIS::KC_UNASSIGNED;
	}

	void Input::UpdateRepeatKey(float elapsed)
	{
		if(m_Key == OIS::KC_UNASSIGNED) return;

		m_Elapsed += elapsed;

		if(m_Elapsed < m_Delay) return;

		m_Elapsed -= m_Delay;
		m_Delay = Repeat_Delay;

		do {
			RepeatKey(m_Key, m_Char);

			m_Elapsed -= Repeat_Delay;
		} while(m_Elapsed >= Repeat_Delay);

		m_Elapsed = 0;
	}

	bool Input::isKeyDown(OIS::KeyCode key)
	{
		return m_Keyboard->isKeyDown(key);
	}

	void Input::RepeatKey(OIS::KeyCode Key, unsigned int Char)
	{
		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.injectKeyUp(Key);
		sys.injectKeyDown(Key);
		sys.injectChar(Char);
	}

}