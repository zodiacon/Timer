// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "aboutdlg.h"
#include "MainFrm.h"
#include "ConfigDlg.h"
#include "ColorsDlg.h"
#include "AppSettings.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle() {
	UIUpdateToolBar();
	return FALSE;
}

void CMainFrame::DoPaint(CDCHandle dc) {
	CRect rc;
	GetClientRect(&rc);
	auto& settings = AppSettings::Get();
	dc.FillSolidRect(&rc, settings.Color(m_Timer.IsExpired() ? TimerColor::ExpiredBackground : TimerColor::Background));

	rc.top += 40;
	dc.SelectFont(m_Font);
	if (m_Timer.IsExpired())
		dc.SetTextColor(settings.Color(TimerColor::ExpiredText));
	else if (m_Timer.IsPaused())
		dc.SetTextColor(settings.Color(TimerColor::PausedText));
	else if (m_Timer.IsRunning())
		dc.SetTextColor(settings.Color(TimerColor::RunningText));
	else
		dc.SetTextColor(settings.Color(TimerColor::StoppedText));
	CString text(m_LastText);

	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(text, -1, &rc, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	SetMenu(nullptr);

	CToolBarCtrl tb;
	tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, ATL_IDW_TOOLBAR);
	InitToolBar(tb, 32);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(tb);

	UIAddToolBar(tb);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	auto pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	auto& settings = AppSettings::Get();

	m_Timer.Init(settings.Seconds(), settings.Minutes());
	m_LastText = m_Timer.GetRemainingTimeAsString();
	SetAlwaysOnTop(settings.AlwaysOnTop());
	UISetCheck(ID_OPTIONS_CHIME, settings.ChimeEnabled());

	// let Explorer's message through if running elevated
	::ChangeWindowMessageFilterEx(m_hWnd, s_TaskbarButtonCreated, MSGFLT_ALLOW, nullptr);

	SetTimer(1, 200);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	m_Timer.Stop();
	WINDOWPLACEMENT wp = { sizeof(wp) };
	GetWindowPlacement(&wp);
	auto& settings = AppSettings::Get();
	settings.MainWindowPlacement(wp);
	settings.Seconds(m_Timer.GetSeconds());
	settings.Minutes(m_Timer.GetMinutes());
	settings.Save();

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = FALSE;
	return 1;
}

LRESULT CMainFrame::OnShowWindow(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	static bool show = false;
	if (show)
		return 0;
	show = true;

	auto wp = AppSettings::Get().MainWindowPlacement();
	if (wp.showCmd != SW_HIDE)
		SetWindowPlacement(&wp);

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnSize(UINT, WPARAM, LPARAM lp, BOOL& bHandled) {
	CRect rc;
	GetClientRect(&rc);
	rc.top += 40;
	if (m_Font)
		m_Font.DeleteObject();
	m_Font.CreateFont(rc.Height() * 130 / 100, rc.Width() * 100 / 550, 0, 0, 500, 0, 0, 0, 0, 0, 0, 0, 0, L"Consolas");
	bHandled = FALSE;
	Invalidate();
	return 0;
}

LRESULT CMainFrame::OnTimer(UINT, WPARAM id, LPARAM, BOOL& bHandled) {
	// nothing changes once expired, until the user acts
	if (id == 1 && !m_Timer.IsExpired()) {
		if (m_Timer.Update()) {
			UISetCheck(ID_TIMER_RUN, FALSE);
			UISetCheck(ID_TIMER_PAUSE, FALSE);
			if (AppSettings::Get().ChimeEnabled())
				m_Chime.Play();
			// flash the taskbar button until the user switches to the window
			FLASHWINFO fi{ sizeof(fi), m_hWnd, FLASHW_ALL | FLASHW_TIMERNOFG };
			::FlashWindowEx(&fi);
			Invalidate();
		}
		auto text = m_Timer.GetRemainingTimeAsString();
		if (text != m_LastText) {
			m_LastText = text;
			Invalidate();
		}
	}
	if (id == 1)
		UpdateTitleAndTaskbar();
	return 0;
}

LRESULT CMainFrame::OnRun(WORD, WORD, HWND, BOOL&) {
	if (m_Timer.Start()) {
		UISetCheck(ID_TIMER_RUN, true);
		UISetCheck(ID_TIMER_PAUSE, false);
	}
	return 0;
}

LRESULT CMainFrame::OnPause(WORD, WORD, HWND, BOOL&) {
	if (m_Timer.Pause()) {
		UISetCheck(ID_TIMER_PAUSE, true);
		UISetCheck(ID_TIMER_RUN, false);
		Invalidate();
	}
	return 0;
}

LRESULT CMainFrame::OnStop(WORD, WORD, HWND, BOOL&) {
	m_Chime.Stop();
	m_Timer.Stop();
	UISetCheck(ID_TIMER_RUN, false);
	UISetCheck(ID_TIMER_PAUSE, false);
	return 0;
}

LRESULT CMainFrame::OnConfigure(WORD, WORD, HWND, BOOL&) {
	CConfigDlg dlg;
	dlg.SetInterval(m_Timer.GetMinutes(), m_Timer.GetSeconds());
	if (dlg.DoModal() == IDOK)
		SetInterval(dlg.GetMinutes(), dlg.GetSeconds());
	return 0;
}

LRESULT CMainFrame::OnToolbarDropDown(int, LPNMHDR pnmh, BOOL& bHandled) {
	auto tb = (NMTOOLBAR*)pnmh;
	if (tb->iItem != ID_TIMER_CONFIGURE) {
		bHandled = FALSE;
		return TBDDRET_NODEFAULT;
	}

	static const unsigned presets[] = { 1, 2, 3, 5, 10, 15, 20, 25, 30, 45, 60 };
	CMenu menu;
	menu.CreatePopupMenu();
	for (int i = 0; i < _countof(presets); i++) {
		CString text;
		text.Format(L"%u minute%s", presets[i], presets[i] == 1 ? L"" : L"s");
		bool current = m_Timer.GetMinutes() == presets[i] && m_Timer.GetSeconds() == 0;
		menu.AppendMenu(MF_STRING | (current ? MF_CHECKED : 0), i + 1, text);
	}

	CRect rc(tb->rcButton);
	::MapWindowPoints(tb->hdr.hwndFrom, nullptr, (POINT*)&rc, 2);
	int cmd = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom, m_hWnd);
	if (cmd > 0)
		SetInterval(presets[cmd - 1], 0);
	return TBDDRET_DEFAULT;
}

void CMainFrame::SetInterval(unsigned min, unsigned sec) {
	m_Timer.Init(sec, min);
	UISetCheck(ID_TIMER_RUN, false);
	UISetCheck(ID_TIMER_PAUSE, false);
	Invalidate();
}

LRESULT CMainFrame::OnColors(WORD, WORD, HWND, BOOL&) {
	CColorsDlg dlg;
	if (dlg.DoModal() == IDOK)
		Invalidate();
	return 0;
}

LRESULT CMainFrame::OnRestart(WORD, WORD, HWND, BOOL&) {
	SendMessage(WM_COMMAND, ID_TIMER_STOP);
	SendMessage(WM_COMMAND, ID_TIMER_RUN);
	return 0;
}

LRESULT CMainFrame::OnAlwaysOnTop(WORD, WORD id, HWND, BOOL&) {
	auto& settings = AppSettings::Get();
	bool ontop;
	settings.AlwaysOnTop(ontop = !settings.AlwaysOnTop());
	UISetCheck(id, ontop);
	SetAlwaysOnTop(ontop);

	return 0;
}

LRESULT CMainFrame::OnChime(WORD, WORD id, HWND, BOOL&) {
	auto& settings = AppSettings::Get();
	bool enabled;
	settings.ChimeEnabled(enabled = !settings.ChimeEnabled());
	UISetCheck(id, enabled);
	if (!enabled)
		m_Chime.Stop();

	return 0;
}

LRESULT CMainFrame::OnTaskbarButtonCreated(UINT, WPARAM, LPARAM, BOOL&) {
	// sent again if Explorer restarts, with a fresh button
	m_Taskbar.Release();
	if (SUCCEEDED(m_Taskbar.CoCreateInstance(CLSID_TaskbarList)) && FAILED(m_Taskbar->HrInit()))
		m_Taskbar.Release();
	m_TaskbarState = TBPF_NOPROGRESS;
	UpdateTitleAndTaskbar();
	return 0;
}

void CMainFrame::UpdateTitleAndTaskbar() {
	CString title;
	if (m_Timer.IsExpired())
		title = L"Time's up - ";
	else if (m_Timer.IsPaused())
		title = m_LastText + L" (Paused) - ";
	else if (m_Timer.IsRunning())
		title = m_LastText + L" - ";
	CString name;
	name.LoadString(IDR_MAINFRAME);
	title += name;
	if (title != m_LastTitle) {
		m_LastTitle = title;
		SetWindowText(title);
	}

	if (!m_Taskbar)
		return;

	auto state = !m_Timer.IsRunning() || m_Timer.IsExpired() ? TBPF_NOPROGRESS :
		m_Timer.IsPaused() ? TBPF_PAUSED : TBPF_NORMAL;
	if (state != m_TaskbarState) {
		m_TaskbarState = state;
		m_Taskbar->SetProgressState(m_hWnd, state);
	}
	if (state != TBPF_NOPROGRESS)
		m_Taskbar->SetProgressValue(m_hWnd, (ULONGLONG)(m_Timer.GetProgress() * 1000), 1000);
}

void CMainFrame::InitToolBar(CToolBarCtrl& tb, int size) {
	CImageList tbImages;
	tbImages.Create(size, size, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);
	tb.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);

	const struct {
		UINT id;
		int image;
		BYTE style = BTNS_BUTTON;
		PCWSTR text = nullptr;
	} buttons[] = {
		{ ID_TIMER_RUN, IDI_PLAY },
		{ ID_TIMER_STOP, IDI_STOP },
		{ ID_TIMER_PAUSE, IDI_PAUSE },
		{ 0 },
		{ ID_TIMER_RESTART, IDI_RESTART },
		{ 0 },
		{ ID_TIMER_COLORS , IDI_COLORS },
		{ ID_TIMER_CONFIGURE , IDI_CONFIGURE, BTNS_DROPDOWN },
		{ ID_OPTIONS_ALWAYSONTOP, IDI_PIN },
		{ ID_OPTIONS_CHIME, IDI_CHIME },
		{ 0 },
		{ ID_APP_ABOUT, IDI_ABOUT },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			auto hIcon = AtlLoadIconImage(b.image, 0, size, size);
			ATLASSERT(hIcon);
			int image = tbImages.AddIcon(hIcon);
			tb.AddButton(b.id, b.style, TBSTATE_ENABLED, image, b.text, 0);
		}
	}
}

void CMainFrame::SetAlwaysOnTop(bool ontop) {
	SetWindowPos(ontop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	UISetCheck(ID_OPTIONS_ALWAYSONTOP, ontop);
}
