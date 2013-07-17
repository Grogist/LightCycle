#ifndef _INPUT_H_
#define _INPUT_H_

#include <OIS.h>

#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OgreRenderWindow.h>
#include <CEGUI.h>

namespace LC
{
	class Input : public OIS::KeyListener, public OIS::MouseListener
	{
	public:
		~Input();

		void Init(Ogre::RenderWindow *renderWindow);
		void SetWindowExtents(int width, int height);
		void SetWindowClosed(void);

		void Capture(Ogre::Real time);

		static Input *getSingletonPtr(void){ return &m_Input; }

		// AUTO REPEAT KEY FOR GUI
		//http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Auto+Repeat+Key+Input&structure=Cookbook
		void BeginRepeatKey(const OIS::KeyEvent &evt);
		void EndRepeatKey(const OIS::KeyEvent &evt);
		void UpdateRepeatKey(float elapsed); // In seconds.

		bool isKeyDown(OIS::KeyCode key);

	private:
		Input(void);
		
		bool keyPressed(const OIS::KeyEvent &e);
		bool keyReleased(const OIS::KeyEvent &e);

		bool mousePressed(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool mouseReleased(const OIS::MouseEvent &e, OIS::MouseButtonID id);
		bool mouseMoved(const OIS::MouseEvent &e);

		OIS::InputManager *m_InputManager;
		OIS::Mouse *m_Mouse;
		OIS::Keyboard *m_Keyboard;

		static Input m_Input;

		// AUTO REPEAT KEY FOR GUI
		OIS::KeyCode m_Key;
		unsigned int m_Char;
		float m_Elapsed;
		float m_Delay;
		// REPURPOSE FOR OWN USE
		void RepeatKey(OIS::KeyCode Key, unsigned int Char);

	};
}


#endif