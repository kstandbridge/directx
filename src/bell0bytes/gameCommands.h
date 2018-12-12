#pragma once

#include "stringifiedEnum.h"

namespace input
{
	// define the game commands
	ENUM_WITH_STRING(GameCommands,	(Select)\
									(Back)\
									(ShowFPS)\
									(MoveLeft)\
									(MoveRight)\
									(MoveUp)\
									(MoveDown)\
									(nGameCommands))

	// define application events
	ENUM_WITH_STRING(Events,		(StartApplication)\
									(PauseApplication)\
									(ResumeApplication)\
									(QuitApplication)\
									(SwitchFullscreen)\
									(WindowChanged)\
									(ChangeResolution)\
									(nApplicationEvents))
}