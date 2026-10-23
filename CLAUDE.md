# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

A simple countdown timer for Windows: a native C++ GUI built on WTL (Windows Template Library) on top of ATL. There are no tests and no lint setup.

## Build

- Solution: `Timer.sln` (one project, `Timer/Timer.vcxproj`). Platform is x64 only (`Debug|x64`, `Release|x64`).
- Toolset is `v145` (Visual Studio 2026), C++20 (`stdcpp20`), Unicode, Windows subsystem.
- WTL comes from the NuGet package `wtl` 10.0.10320 (`Timer/packages.config`), restored into `packages/`. Restore it before building from the command line:

```
msbuild Timer.sln /t:Restore /p:RestorePackagesConfig=true
msbuild Timer.sln /p:Configuration=Release /p:Platform=x64
```

Output goes to `x64\<Configuration>\Timer.exe`.

## Architecture

- `pch.h` is the precompiled header. Every `.cpp` includes it first. It pulls in the ATL/WTL headers and declares the global `CAppModule _Module`.
- `Timer.cpp` (`_tWinMain`) initializes COM and common controls, loads settings from `HKCU\SOFTWARE\ScorpioSoftware\Timer`, and runs the WTL message loop with `CMainFrame`.
- `CMainFrame` (`MainFrm.*`) is the only window. It is a `CFrameWindowImpl` with `CAutoUpdateUI` for toolbar and menu check state (`UISetCheck`) and `CDoubleBufferImpl`, so painting happens in `DoPaint`. A 200 ms `WM_TIMER` polls `CountdownTimer` and redraws the remaining time when the text changes. The frame saves settings in `OnDestroy`.
- `CountdownTimer` is plain logic with no window. It tracks elapsed time with `GetTickCount64`, handles pause by shifting the start tick, and calls back through a `std::function` when the countdown expires.
- `CConfigDlg` sets minutes and seconds. `CAboutDlg` is the About box. Dialog templates, menus, accelerators, icons (`res/*.ico`) and version info are in `Timer.rc`, with IDs in `resource.h`.
- Settings framework (`Settings.h/.cpp`, `AppSettings.h`): `Settings` is a generic map from names to registry values that supports load and save. `AppSettings` is a singleton (`AppSettings::Get()`) that declares its settings with macros:
  - `SETTING(name, default, SettingType)` inside `BEGIN_SETTINGS`/`END_SETTINGS` registers the setting and its default.
  - `DEF_SETTING(name, type)` generates the typed getter and setter.

  To add a persisted setting, add both macros. The stored size must equal `sizeof(type)`, because this is asserted.

## Conventions

- Tabs for indentation. The opening brace goes on the same line.
- Members are prefixed with `m_` in WTL window classes (`m_Timer`) and with `_` in plain classes (`_running`).
- Message handlers use the WTL `BEGIN_MSG_MAP` / `MESSAGE_HANDLER` / `COMMAND_ID_HANDLER` macros and the standard handler signatures listed as comments in the headers.
