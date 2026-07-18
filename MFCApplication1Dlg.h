
// MFCApplication1Dlg.h: 头文件
//

#pragma once
#include <array>
#include <vector>


// CMFCApplication1Dlg 对话框：负责系统信息展示与系统设置操作页面。
class CMFCApplication1Dlg : public CDialogEx
{
// 构造
public:
	// 构造主对话框对象并初始化基础状态。
	CMFCApplication1Dlg(CWnd* pParent = nullptr);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION1_DIALOG };
#endif

protected:
	// 绑定控件与成员变量的 DDX/DDV 入口。
	virtual void DoDataExchange(CDataExchange* pDX);


// 实现
protected:
	// 主窗口图标句柄。
	HICON m_hIcon;

	// 消息映射函数：处理窗口生命周期、输入与控件事件。
	// 初始化对话框，创建布局并触发异步数据加载。
	virtual BOOL OnInitDialog();
	// 处理系统菜单命令（如“关于”）。
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	// 处理窗口重绘。
	afx_msg void OnPaint();
	// 返回拖拽最小化图标时使用的光标。
	afx_msg HCURSOR OnQueryDragIcon();
	// 处理窗口尺寸变化并调整布局。
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// 处理鼠标左键，用于侧边栏页面切换。
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	// 处理垂直滚动条滚动事件。
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	// 处理鼠标滚轮滚动事件。
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	// 处理异步加载系统信息消息。
	afx_msg LRESULT OnLoadSystemInformation(WPARAM wParam, LPARAM lParam);
	// 处理控件颜色与背景刷设置。
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// 处理“应用设置”按钮点击。
	afx_msg void OnBnClickedApplySettings();
	// 处理“重启系统”按钮点击。
	afx_msg void OnBnClickedRebootSystem();
	// 处理“全选/取消全选”按钮点击。
	afx_msg void OnBnClickedToggleSelect();
	// 处理设置项勾选变化。
	afx_msg void OnSettingsOptionChanged(UINT nID);
	DECLARE_MESSAGE_MAP()
public:
	// 初始化页面基础布局参数。
	void BuildMainLayout();
	// 按当前窗口大小重新布局。
	void AdjustLayout(int cx, int cy);
	// 读取并整理系统信息数据。
	void LoadSystemInformation();
	// 向系统信息列表追加一行键值数据。
	void AddSystemInfoRow(const CString& item, const CString& value);
	// 确保界面绘制所需字体和画刷已创建。
	void EnsureUiFonts();
	// 绘制圆角卡片背景。
	void DrawRoundedCard(CDC& dc, const CRect& rect, COLORREF fillColor, int radius);
	// 绘制主界面（根据当前页分发绘制逻辑）。
	void DrawSystemInformation(CDC& dc, const CRect& clientRect);
	// 绘制“系统设置”页面。
	void DrawSystemSettings(CDC& dc, const CRect& clientRect);
	// 根据内容高度更新垂直滚动条状态。
	void UpdateVerticalScrollBar(int contentHeight, int viewHeight);
	// 重新计算侧栏与内容区矩形区域。
	void RecalcLayoutRects(const CRect& clientRect);
	// 动态创建设置页控件。
	void CreateSettingsControls();
	// 更新设置页控件位置与尺寸。
	void UpdateSettingsControlLayout();
	// 根据当前页切换控件可见性。
	void UpdatePageVisibility();
	// 更新状态栏文本内容。
	void SetStatusText(const CString& text, COLORREF color);
	// 根据勾选状态刷新“全选/取消全选”按钮文案。
	void UpdateToggleSelectButton();
	// 判断是否所有设置项都被选中。
	bool AreAllOptionsSelected() const;
	// 批量设置所有选项的勾选状态。
	void SetAllOptions(bool isChecked);
	// 获取全部设置项复选框集合。
	std::vector<CButton*> GetOptionCheckBoxes();
	// 判断当前进程是否以管理员权限运行。
	bool IsRunningAsAdmin() const;

	// 应用 UAC 设置（启用/禁用）。
	bool ApplyUAC(bool disable);
	// 应用 Windows 防火墙状态设置。
	bool ApplyFirewall(bool disable);
	// 应用安全中心通知设置。
	bool ApplySecurityCenter(bool disable);
	// 应用系统崩溃后自动重启设置。
	bool ApplyAutoReboot(bool disable);
	// 应用崩溃转储开关设置。
	bool ApplyCrashDump(bool enable);
	// 应用屏保开关设置。
	bool ApplyScreenSaver(bool disable);
	// 应用电源超时策略（是否设为永不）。
	bool ApplyPowerSettings(bool setNever);
	// 应用 Windows 自动更新设置。
	bool ApplyWindowsUpdate(bool disable);
	// 启动外部进程并等待其结束，返回是否成功。
	bool RunProcessAndWait(const CString& fileName, const CString& args, const CString* workingDirectory = nullptr);
	// 将资源释放为临时文件并返回路径。
	CString ExtractResourceToTempFile(UINT resourceId, const CString& fileName);
	// 将资源写入指定路径文件。
	bool ExtractResourceToPath(UINT resourceId, const CString& outputPath);
	// 删除临时文件（若路径有效）。
	void DeleteTempFile(const CString& filePath);
	// 处理背景擦除消息，减少界面闪烁。
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
	// 系统信息行：左侧项目名 + 右侧值。
	struct InfoRow
	{
		CString item;
		CString value;
	};

	// 系统信息数据与绘制资源。
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

	// 设置页控件集合。
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
