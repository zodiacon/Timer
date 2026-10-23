#include "pch.h"
#include "resource.h"
#include "ConfigDlg.h"

void CConfigDlg::SetInterval(unsigned min, unsigned sec) {
	m_Min = min;
	m_Sec = sec;
}

unsigned CConfigDlg::GetMinutes() const {
	return m_Min;
}

unsigned CConfigDlg::GetSeconds() const {
	return m_Sec;
}

LRESULT CConfigDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CenterWindow(GetParent());
	m_MinUD.Attach(GetDlgItem(IDC_MINSPIN));
	m_MinUD.SetRange32(0, 99);
	SetDlgItemInt(IDC_MIN, m_Min, false);

	m_SecUD.Attach(GetDlgItem(IDC_SECSPIN));
	m_SecUD.SetRange32(0, 59);
	SetDlgItemInt(IDC_SEC, m_Sec, false);

	return TRUE;
}

LRESULT CConfigDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (wID == IDOK) {
		BOOL minOk, secOk;
		auto min = GetDlgItemInt(IDC_MIN, &minOk, false);
		auto sec = GetDlgItemInt(IDC_SEC, &secOk, false);
		if (!minOk || !secOk || min > 99 || sec > 59 || min + sec == 0) {
			AtlMessageBox(m_hWnd, L"Enter 0-99 minutes and 0-59 seconds, with a total of at least one second.",
				IDR_MAINFRAME, MB_ICONWARNING);
			return 0;
		}
		m_Min = min;
		m_Sec = sec;
	}
	EndDialog(wID);
	return 0;
}
