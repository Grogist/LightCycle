#ifndef _GUI_H_
#define _GUI_H_

#include <CEGUI.h>
#include <CEGUIOgreRenderer.h>

#include <OgreMath.h>

namespace LC
{
	class GUI
	{
	public:
		~GUI(void);

		bool Init(void);

		static GUI *getSingletonPtr(void) { return &m_GUI; }

		void InjectTimestamps(Ogre::Real time);

		void hideGUI(void);

		CEGUI::Window *getGUIWindow(void);
		CEGUI::OgreRenderer *getRenderer(void);

	private:
		GUI(void);

		CEGUI::OgreRenderer *m_Renderer;
		CEGUI::Window *m_GUIWindow;

		static GUI m_GUI;
	};
}

#endif