#pragma once

#include "Settings.h"

enum class TimerColor {
	Background,
	ExpiredBackground,
	StoppedText,
	RunningText,
	PausedText,
	ExpiredText,
	Count
};

struct AppSettings : Settings {
	BEGIN_SETTINGS(AppSettings)
		SETTING(MainWindowPlacement, WINDOWPLACEMENT{}, SettingType::Binary);
		SETTING(Minutes, 1, SettingType::Int32);
		SETTING(Seconds, 0, SettingType::Int32);
		SETTING(AlwaysOnTop, 0, SettingType::Bool);
		SETTING(ChimeEnabled, 1, SettingType::Bool);
		SETTING(Stopwatch, 0, SettingType::Bool);
		for (int i = 0; i < (int)TimerColor::Count; i++)
			Set(ColorName((TimerColor)i), DefaultColor((TimerColor)i), SettingType::Int32);
	END_SETTINGS

	DEF_SETTING(Seconds, int)
	DEF_SETTING(Minutes, int)
	DEF_SETTING(AlwaysOnTop, int)
	DEF_SETTING(ChimeEnabled, int)
	DEF_SETTING(Stopwatch, int)
	DEF_SETTING(MainWindowPlacement, WINDOWPLACEMENT)

	COLORREF Color(TimerColor color) const {
		return GetValueOrDefault<COLORREF>(ColorName(color), DefaultColor(color));
	}
	void Color(TimerColor color, COLORREF value) {
		Set(ColorName(color), value, SettingType::Int32);
	}

	static PCWSTR ColorName(TimerColor color) {
		static const PCWSTR names[] = {
			L"BackgroundColor", L"ExpiredBackgroundColor",
			L"StoppedTextColor", L"RunningTextColor", L"PausedTextColor", L"ExpiredTextColor",
		};
		return names[(int)color];
	}
	static COLORREF DefaultColor(TimerColor color) {
		static const COLORREF colors[] = {
			RGB(240, 240, 240), RGB(0, 128, 0),
			RGB(0, 0, 0), RGB(128, 0, 0), RGB(0, 0, 255), RGB(0, 255, 0),
		};
		return colors[(int)color];
	}
};
