#include "pch.h"
#include "resource.h"
#include "ColorsDlg.h"

LRESULT CColorsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CenterWindow(GetParent());
	auto& settings = AppSettings::Get();
	for (int i = 0; i < _countof(m_Colors); i++)
		m_Colors[i] = settings.Color((TimerColor)i);

	return TRUE;
}

LRESULT CColorsDlg::OnDrawItem(UINT /*uMsg*/, WPARAM id, LPARAM lParam, BOOL& bHandled) {
	if (id < IDC_COLOR_BACK || id > IDC_COLOR_EXPIRED) {
		bHandled = FALSE;
		return 0;
	}

	auto dis = (DRAWITEMSTRUCT*)lParam;
	CDCHandle dc(dis->hDC);
	CRect rc(dis->rcItem);
	dc.DrawFrameControl(&rc, DFC_BUTTON, DFCS_BUTTONPUSH | ((dis->itemState & ODS_SELECTED) ? DFCS_PUSHED : 0));
	rc.DeflateRect(4, 4);
	dc.FillSolidRect(&rc, m_Colors[id - IDC_COLOR_BACK]);
	dc.FrameRect(&rc, (HBRUSH)::GetStockObject(GRAY_BRUSH));
	if (dis->itemState & ODS_FOCUS) {
		rc.InflateRect(2, 2);
		dc.DrawFocusRect(&rc);
	}
	return TRUE;
}

LRESULT CColorsDlg::OnPickColor(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto& color = m_Colors[wID - IDC_COLOR_BACK];
	CColorDialog dlg(color, CC_FULLOPEN, m_hWnd);
	if (dlg.DoModal() == IDOK) {
		color = dlg.GetColor();
		GetDlgItem(wID).Invalidate();
	}
	return 0;
}

LRESULT CColorsDlg::OnDefaults(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	for (int i = 0; i < _countof(m_Colors); i++) {
		m_Colors[i] = AppSettings::DefaultColor((TimerColor)i);
		GetDlgItem(IDC_COLOR_BACK + i).Invalidate();
	}
	return 0;
}

LRESULT CColorsDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (wID == IDOK) {
		auto& settings = AppSettings::Get();
		for (int i = 0; i < _countof(m_Colors); i++)
			settings.Color((TimerColor)i, m_Colors[i]);
	}
	EndDialog(wID);
	return 0;
}
