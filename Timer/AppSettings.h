#pragma once

#include "Settings.h"

struct AppSettings : Settings {
	BEGIN_SETTINGS(AppSettings)
		SETTING(MainWindowPlacement, WINDOWPLACEMENT{}, SettingType::Binary);
		SETTING(Minutes, 1, SettingType::Int32);
		SETTING(Seconds, 0, SettingType::Int32);
		SETTING(AlwaysOnTop, 0, SettingType::Bool);
	END_SETTINGS

	DEF_SETTING(Seconds, int)
	DEF_SETTING(Minutes, int)
	DEF_SETTING(AlwaysOnTop, int)
	DEF_SETTING(MainWindowPlacement, WINDOWPLACEMENT)
};
