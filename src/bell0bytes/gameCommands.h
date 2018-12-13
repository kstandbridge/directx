#pragma once

// bell0bytes util
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

	// define joystick buttons
	ENUM_WITH_STRING(JoystickButtons,	(JoyPOV_Left)\
										(JoyPOV_Right)\
										(JoyPOV_Up)\
										(JoyPOV_Down)\
										(EndPOV)\
		(JoyBtn_1)(JoyBtn_2)(JoyBtn_3)\
		(JoyBtn_4)(JoyBtn_5)(JoyBtn_6)\
		(JoyBtn_7)(JoyBtn_8)(JoyBtn_9)\
		(JoyBtn_10)(JoyBtn_11)(JoyBtn_12)\
		(JoyBtn_13)(JoyBtn_14)(JoyBtn_15)\
		(JoyBtn_16)(JoyBtn_17)(JoyBtn_18)\
		(JoyBtn_19)(JoyBtn_20)(JoyBtn_21)\
		(JoyBtn_22)(JoyBtn_23)(JoyBtn_24)\
		(JoyBtn_25)(JoyBtn_26)(JoyBtn_27)\
		(JoyBtn_28)(JoyBtn_29)(JoyBtn_30)\
		(JoyBtn_31)(JoyBtn_32)(EndButtons))
}