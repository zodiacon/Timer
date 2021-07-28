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
	m_MinUD.SetRange32(0, 24);
	SetDlgItemInt(IDC_MIN, m_Min, false);

	m_SecUD.Attach(GetDlgItem(IDC_SECSPIN));
	m_SecUD.SetRange32(0, 59);
	SetDlgItemInt(IDC_SEC, m_Sec, false);

	return TRUE;
}

LRESULT CConfigDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if (wID == IDOK) {
		m_Min = GetDlgItemInt(IDC_MIN, nullptr, false);
		m_Sec = GetDlgItemInt(IDC_SEC, nullptr, false);

	}
	EndDialog(wID);
	return 0;
}
