#include "GameConsole.h"

namespace LC
{
	GameConsole::GameConsole(CEGUI::Window* consolewindow, float maxposition) : 
		m_ConsoleWindow(consolewindow), m_IsVisible(false), m_IsMoving(0), 
		m_MoveSpeed(1.f), m_MaximumPosition(maxposition)
	{}

	GameConsole::~GameConsole() {}

	void GameConsole::SetVisible(bool set)
	{
		if(set)
		{
			m_IsMoving = -1;
			m_ConsoleWindow->enable();
			m_ConsoleWindow->getChild("LIGHTCYCLEGAME/Console/EditBox")->activate();
			CEGUI::MouseCursor::getSingletonPtr()->show();
		}
		else
		{
			m_IsMoving = 1;
			m_ConsoleWindow->disable();
			m_ConsoleWindow->deactivate();
			CEGUI::MouseCursor::getSingletonPtr()->hide();
		}
		m_IsVisible = !m_IsVisible;
		m_ConsoleWindow->show();
	}

	bool GameConsole::IsVisible()
	{
		return m_IsVisible;
	}

	bool GameConsole::IsActive()
	{
		return m_ConsoleWindow->getChild("LIGHTCYCLEGAME/Console/EditBox")->isActive();
	}

	void GameConsole::update(float time)
	{
		if(m_IsMoving == 0)
			return;

		CEGUI::UVector2 position = m_ConsoleWindow->getPosition();

		position.d_y.d_scale += time * m_MoveSpeed * m_IsMoving;
		float current_d_scale = position.d_y.d_scale;

		if(position.d_y.d_scale <= m_MaximumPosition)
		{
			// Not Moving.
			position.d_y.d_scale = m_MaximumPosition;
			m_IsMoving = 0;
		}
		else if(position.d_y.d_scale >= 1.f)
		{
			position.d_y.d_scale = 1.f;
			m_IsMoving = 0;
		}

		float off = position.d_y.d_offset;
		float scale = position.d_y.d_scale;

		m_ConsoleWindow->setPosition(position);
	}

	void GameConsole::HandleChatMessage(std::string text)
	{
		CEGUI::Listbox *chatBox = static_cast<CEGUI::Listbox*>
			(m_ConsoleWindow->getChild("LIGHTCYCLEGAME/Console/ListBox"));
		CEGUI::ListboxTextItem *newItem = 0;
		newItem = new CEGUI::ListboxTextItem(text, CEGUI::HTF_WORDWRAP_LEFT_ALIGNED);
		chatBox->addItem(newItem);
		chatBox->ensureItemIsVisible(newItem);
	}

	std::string GameConsole::GetText()
	{
		std::string text = m_ConsoleWindow->
			getChild("LIGHTCYCLEGAME/Console/EditBox")->getText().c_str();
		m_ConsoleWindow->getChild("LIGHTCYCLEGAME/Console/EditBox")->setText("");
		return text;
	}

}