// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "aboutdlg.h"
#include "MainFrm.h"
#include "ConfigDlg.h"
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
	dc.FillSolidRect(&rc, m_Timer.IsExpired() ? RGB(0, 128, 0) : RGB(240, 240, 240));

	rc.top += 40;
	dc.SelectFont(m_Font);
	if (m_Timer.IsExpired())
		dc.SetTextColor(RGB(0, 255, 0));
	else if (m_Timer.IsPaused())
		dc.SetTextColor(RGB(0, 0, 255));
	else if (m_Timer.IsRunning())
		dc.SetTextColor(RGB(128, 0, 0));
	CString text(m_LastText);

	//if (m_Timer.IsRunning() && !m_Timer.IsPaused() && (::GetTickCount64() / 1000) & 1)
	//	text.SetAt(2, L' ');
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
	m_LastText = m_Timer.GetRemaingTimeAsString();
	SetAlwaysOnTop(settings.AlwaysOnTop());

	SetTimer(1, 200, nullptr);

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

	auto wp = AppSettings::Get().MainWindowPlacement();
	if (wp.showCmd != SW_HIDE)
		SetWindowPlacement(&wp);

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST);	// toolbar is first 1st band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
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
	if (id == 1) {
		if (m_Timer.IsExpired()) {
			Invalidate();
			UISetCheck(ID_TIMER_RUN, FALSE);
		}
		else {
			auto text = m_Timer.GetRemaingTimeAsString();
			if (text != m_LastText) {
				m_LastText = text;
				Invalidate();
				if (m_Timer.IsRunning() && !m_Timer.IsPaused()) {
					//::Beep(800, 50);
				}
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnRun(WORD, WORD, HWND, BOOL&) {
	if (m_Timer.Start(nullptr)) {
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
	m_Timer.Stop();
	UISetCheck(ID_TIMER_RUN, false);
	UISetCheck(ID_TIMER_PAUSE, false);
	return 0;
}

LRESULT CMainFrame::OnConfigure(WORD, WORD, HWND, BOOL&) {
	CConfigDlg dlg;
	dlg.SetInterval(m_Timer.GetMinutes(), m_Timer.GetSeconds());
	if (dlg.DoModal() == IDOK) {
		m_Timer.Init(dlg.GetSeconds(), dlg.GetMinutes());
		UISetCheck(ID_TIMER_RUN, false);
		UISetCheck(ID_TIMER_PAUSE, false);
	}
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

void CMainFrame::InitToolBar(CToolBarCtrl& tb, int size) {
	CImageList tbImages;
	tbImages.Create(size, size, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);

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
		{ ID_TIMER_CONFIGURE , IDI_CONFIGURE },
		{ ID_OPTIONS_ALWAYSONTOP, IDI_PIN },
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
