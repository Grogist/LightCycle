#include "GUI.h"

namespace LC
{
	GUI GUI::m_GUI;

	GUI::GUI(void) {};

	GUI::~GUI(void)
	{
		CEGUI::WindowManager::getSingleton().destroyAllWindows();
	}

	bool GUI::Init(void)
	{
		m_Renderer = &CEGUI::OgreRenderer::bootstrapSystem();

		CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
		CEGUI::Font::setDefaultResourceGroup("Fonts");
		CEGUI::Scheme::setDefaultResourceGroup("Schemes");
		CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
		CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

		CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");
		CEGUI::System::getSingleton().setDefaultMouseCursor("TaharezLook", "MouseArrow");

		CEGUI::WindowManager &wmgr = CEGUI::WindowManager::getSingleton();

		m_GUIWindow = wmgr.createWindow("DefaultWindow", "ROOT");
		CEGUI::System::getSingleton().setGUISheet(m_GUIWindow);
		
		return true;
	}

	void GUI::InjectTimestamps(Ogre::Real time)
	{
		CEGUI::System::getSingleton().injectTimePulse(time);
	}

	void GUI::hideGUI(void)
	{
		m_GUIWindow->setVisible(!m_GUIWindow->isVisible());
	}

	CEGUI::Window *GUI::getGUIWindow(void)
	{
		return m_GUIWindow;
	}

	CEGUI::OgreRenderer *GUI::getRenderer(void)
	{
		return m_Renderer;
	}

}