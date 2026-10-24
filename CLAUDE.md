# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

A simple countdown timer for Windows: a native C++ GUI built on WTL (Windows Template Library) on top of ATL. There are no tests and no lint setup.

## Build

- Solution: `Timer.sln` (one project, `Timer/Timer.vcxproj`). Platform is x64 only: `Debug`, `Release`, and `ReleaseSigned`. `ReleaseSigned` is `Release` plus a post-build `signtool` step that signs with the "Scorpio" certificate from the cert store and a DigiCert timestamp. It needs that certificate installed, so use `Release` when you only want to check that the code builds.
- Toolset is `v145` (Visual Studio 2026), C++20 (`stdcpp20`), Unicode, Windows subsystem.
- WTL comes from the NuGet package `wtl` 10.0.10320 (`Timer/packages.config`), restored into `packages/`. Restore it before building from the command line:

```
msbuild Timer.sln /t:Restore /p:RestorePackagesConfig=true
msbuild Timer.sln /p:Configuration=Release /p:Platform=x64
msbuild Timer.sln /p:Configuration=ReleaseSigned /p:Platform=x64
```

Output goes to `x64\<Configuration>\Timer.exe`.

## Architecture

- `pch.h` is the precompiled header. Every `.cpp` includes it first. It pulls in the ATL/WTL headers and declares the global `CAppModule _Module`.
- `Timer.cpp` (`_tWinMain`) initializes COM and common controls, loads settings from `HKCU\SOFTWARE\ScorpioSoftware\Timer`, and runs the WTL message loop with `CMainFrame`.
- `CMainFrame` (`MainFrm.*`) is the only window. It has no visible menu, so the toolbar is the whole UI.
  - It is a `CFrameWindowImpl` with `CAutoUpdateUI` for toolbar check and enable state (`UISetCheck`, `UIEnable`) and `CDoubleBufferImpl`, so painting happens in `DoPaint`.
  - A 200 ms `WM_TIMER` (`OnTimer`) calls `CountdownTimer::Update()`. When the countdown ends, it plays the chime (if `ChimeEnabled`) and flashes the taskbar button. It redraws only when the text changes, then calls `UpdateTitleAndTaskbar()`, which puts the time in the window title and sets taskbar progress through `ITaskbarList3`. That interface is created on the registered `TaskbarButtonCreated` message.
  - The Configure toolbar button is a split button: `TBN_DROPDOWN` shows the preset intervals.
  - The frame saves settings in `OnDestroy`.
- `CountdownTimer` is plain logic with no window.
  - It tracks elapsed time with `GetTickCount64` and handles pause by shifting the start tick.
  - In stopwatch mode (`SetStopwatch`) it counts up from zero and never expires.
  - `Update()` returns true only on the tick where the countdown expires. `GetTimeAsString()` has no side effects.
- `Chime` builds a short WAV in memory when it is constructed and plays it with `PlaySound(SND_MEMORY | SND_ASYNC)`.
- Dialogs:
  - `CConfigDlg` sets minutes and seconds and validates them.
  - `CColorsDlg` edits the six `TimerColor` values with owner-drawn swatch buttons.
  - `CAboutDlg` is the About box.
- Resources: dialog templates, icons (`res/*.ico`), toolbar tooltip strings (the text after `\n` in the string table) and version info are in `Timer.rc`, with IDs in `resource.h`. `Timer.rc` is Windows-1252, not UTF-8, because of the © character, so edit it in a way that keeps that encoding.
- To add a toolbar button:
  1. Add the command and icon IDs to `resource.h`.
  2. Add the `ICON` entry and tooltip string to `Timer.rc`.
  3. Add an entry to the `buttons[]` table in `CMainFrame::InitToolBar`.
  4. Add a `COMMAND_ID_HANDLER` to the message map.
- Settings framework (`Settings.h/.cpp`, `AppSettings.h`): `Settings` is a generic map from names to registry values that supports load and save. `AppSettings` is a singleton (`AppSettings::Get()`) that declares its settings with macros:
  - `SETTING(name, default, SettingType)` inside `BEGIN_SETTINGS`/`END_SETTINGS` registers the setting and its default.
  - `DEF_SETTING(name, type)` generates the typed getter and setter.

  To add a persisted setting, add both macros. The stored size must equal `sizeof(type)`, because this is asserted. Colors don't use the macros. They are registered in a loop and accessed through `Color(TimerColor)`.

## Conventions

- Tabs for indentation. The opening brace goes on the same line.
- Members are prefixed with `m_` in WTL window classes (`m_Timer`) and with `_` in plain classes (`_running`).
- Message handlers use the WTL `BEGIN_MSG_MAP` / `MESSAGE_HANDLER` / `COMMAND_ID_HANDLER` macros and the standard handler signatures listed as comments in the headers.
