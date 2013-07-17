#ifndef _GAMECONSOLE_H_
#define _GAMECONSOLE_H_

#include <OGRE/OgreFrameListener.h>
//#include <CEGUIWindow.h>
// Don't want to include, but can't find definition of ListBox in src.
#include <CEGUI.h>


namespace LC
{

	class GameConsole
	{
	public:
		GameConsole(CEGUI::Window* consolewindow, float maxposition);
		~GameConsole();

		void SetVisible(bool set);
		bool IsVisible();
		bool IsActive();

		void HandleChatMessage(std::string text);
		std::string GetText();
		
		void update(float time);

	private:
		CEGUI::Window* m_ConsoleWindow;

		bool m_IsVisible;
		// > 0  moving down, < 0 moving up, = 0 not moving.
		short m_IsMoving;

		// Percent of the screen the console can move per second.
		float m_MoveSpeed;
		// Assuming movement only in y-direction;
		float m_MaximumPosition;
	};

}

#endif