
// MFCApplication1Dlg.h: 头文件
//

#pragma once
#include <array>
#include <vector>


// CMFCApplication1Dlg 对话框
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication1Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg LRESULT OnLoadSystemInformation(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedApplySettings();
	afx_msg void OnBnClickedRebootSystem();
	afx_msg void OnBnClickedToggleSelect();
	afx_msg void OnSettingsOptionChanged(UINT nID);
	DECLARE_MESSAGE_MAP()
public:
	void BuildMainLayout();
	void AdjustLayout(int cx, int cy);
	void LoadSystemInformation();
	void AddSystemInfoRow(const CString& item, const CString& value);
	void EnsureUiFonts();
	void DrawRoundedCard(CDC& dc, const CRect& rect, COLORREF fillColor, int radius);
	void DrawSystemInformation(CDC& dc, const CRect& clientRect);
	void DrawSystemSettings(CDC& dc, const CRect& clientRect);
	void UpdateVerticalScrollBar(int contentHeight, int viewHeight);
	void RecalcLayoutRects(const CRect& clientRect);
	void CreateSettingsControls();
	void UpdateSettingsControlLayout();
	void UpdatePageVisibility();
	void SetStatusText(const CString& text, COLORREF color);
	void UpdateToggleSelectButton();
	bool AreAllOptionsSelected() const;
	void SetAllOptions(bool isChecked);
	std::vector<CButton*> GetOptionCheckBoxes();
	bool IsRunningAsAdmin() const;

	bool ApplyUAC(bool disable);
	bool ApplyFirewall(bool disable);
	bool ApplySecurityCenter(bool disable);
	bool ApplyAutoReboot(bool disable);
	bool ApplyCrashDump(bool enable);
	bool ApplyScreenSaver(bool disable);
	bool ApplyPowerSettings(bool setNever);
	bool ApplyWindowsUpdate(bool disable);
	bool RunProcessAndWait(const CString& fileName, const CString& args, const CString* workingDirectory = nullptr);
	CString ExtractResourceToTempFile(UINT resourceId, const CString& fileName);
	bool ExtractResourceToPath(UINT resourceId, const CString& outputPath);
	void DeleteTempFile(const CString& filePath);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
	struct InfoRow
	{
		CString item;
		CString value;
	};

	std::vector<InfoRow> m_systemRows;
	CFont m_titleFont;
	CFont m_subtitleFont;
	CFont m_labelFont;
	CFont m_valueFont;
	CFont m_menuFont;
	CFont m_settingsFont;
	CBrush m_uiBackgroundBrush;
	bool m_loading = true;
	int m_scrollPos = 0;
	int m_contentHeight = 0;
	int m_activePage = 0;
	CRect m_sideRect;
	CRect m_contentRect;
	CRect m_infoMenuRect;
	CRect m_settingsMenuRect;

	CButton m_chkUAC;
	CButton m_chkFirewall;
	CButton m_chkSecCenter;
	CButton m_chkAutoReboot;
	CButton m_chkCrashDump;
	CButton m_chkScreenSaver;
	CButton m_chkPower;
	CButton m_chkWindowsUpdate;
	CButton m_btnApply;
	CButton m_btnReboot;
	CButton m_btnToggleSelect;
	CStatic m_statusText;
	CStatic m_adminHintText;
};
