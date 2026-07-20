
// MFCApplication1Dlg.h: 头文件
//

#pragma once
#include <array>
#include <vector>


// CMFCApplication1Dlg 对话框：负责系统信息展示与系统设置操作页面。
class CMFCApplication1Dlg : public CDialogEx
{
	struct InfoRow;

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
	// 处理异步加载 SSD 信息消息。
	afx_msg LRESULT OnLoadSsdInformation(WPARAM wParam, LPARAM lParam);
	// 处理异步加载屏幕详情消息。
	afx_msg LRESULT OnLoadScreenInformation(WPARAM wParam, LPARAM lParam);
	// 接收后台线程完成后的系统信息结果。
	afx_msg LRESULT OnApplyLoadedSystemInformation(WPARAM wParam, LPARAM lParam);
	// 接收后台线程完成后的 SSD 信息结果。
	afx_msg LRESULT OnApplyLoadedSsdInformation(WPARAM wParam, LPARAM lParam);
	// 接收后台线程完成后的屏幕详情结果。
	afx_msg LRESULT OnApplyLoadedScreenInformation(WPARAM wParam, LPARAM lParam);
	// 处理控件颜色与背景刷设置。
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// 处理“应用设置”按钮点击。
	afx_msg void OnBnClickedApplySettings();
	// 处理“重启系统”按钮点击。
	afx_msg void OnBnClickedRebootSystem();
	// 处理“全选/取消全选”按钮点击。
	afx_msg void OnBnClickedToggleSelect();
	// 处理启动项启用按钮点击。
	afx_msg void OnBnClickedStartupEnable();
	// 处理启动项禁用按钮点击。
	afx_msg void OnBnClickedStartupDisable();
	// 处理启动项删除按钮点击。
	afx_msg void OnBnClickedStartupDelete();
	// 处理启动项刷新按钮点击。
	afx_msg void OnBnClickedStartupRefresh();
	// 处理启动项浏览按钮点击。
	afx_msg void OnBnClickedStartupBrowse();
	// 处理启动项添加按钮点击。
	afx_msg void OnBnClickedStartupAdd();
	// 处理电池日志详情按钮点击。
	afx_msg void OnBnClickedBatteryLogDetails();
	// 处理电池日志刷新按钮点击。
	afx_msg void OnBnClickedBatteryLogRefresh();
	// 处理电源日志详情按钮点击。
	afx_msg void OnBnClickedPowerLogDetails();
	// 处理电源日志刷新按钮点击。
	afx_msg void OnBnClickedPowerLogRefresh();
	// 处理设置项勾选变化。
	afx_msg void OnSettingsOptionChanged(UINT nID);
	// 处理 SSD Tab 切换事件。
	afx_msg void OnTcnSelchangeSsdTab(NMHDR* pNMHDR, LRESULT* pResult);
	// 处理启动项列表选择变化。
	afx_msg void OnLvnItemchangedStartupList(NMHDR* pNMHDR, LRESULT* pResult);
	// 处理启动项列表右键菜单。
	afx_msg void OnNMRClickStartupList(NMHDR* pNMHDR, LRESULT* pResult);
	// 处理启动项列表自绘，用颜色区分启用/禁用状态。
	afx_msg void OnNMCustomdrawStartupList(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
public:
	// 初始化页面基础布局参数。
	void BuildMainLayout();
	// 按当前窗口大小重新布局。
	void AdjustLayout(int cx, int cy);
	// 读取并整理系统信息数据。
	void LoadSystemInformation(bool loadSsdDetails = true);
	// 向系统信息列表追加一行键值数据。
	void AddSystemInfoRow(const CString& item, const CString& value);
	// 确保界面绘制所需字体和画刷已创建。
	void EnsureUiFonts();
	// 绘制圆角卡片背景。
	void DrawRoundedCard(CDC& dc, const CRect& rect, COLORREF fillColor, int radius, COLORREF borderColor = CLR_NONE);
	// 绘制主界面（根据当前页分发绘制逻辑）。
	void DrawSystemInformation(CDC& dc, const CRect& clientRect);
	// 绘制“SSD信息”页面。
	void DrawSsdInformation(CDC& dc, const CRect& clientRect);
	// 绘制“屏幕详情”页面。
	void DrawScreenInformation(CDC& dc, const CRect& clientRect);
	// 绘制“系统状态”页面。
	void DrawSystemStatus(CDC& dc, const CRect& clientRect);
	// 绘制“启动项”页面。
	void DrawStartupItems(CDC& dc, const CRect& clientRect);
	// 绘制“硬件详情”页面。
	void DrawAcpiInformation(CDC& dc, const CRect& clientRect);
	// 绘制“系统设置”页面。
	void DrawSystemSettings(CDC& dc, const CRect& clientRect);
	// 绘制 powercfg 导出的日志报告页面。
	void DrawPowerCfgReportPage(CDC& dc, const CRect& clientRect, const CString& title, const CString& subtitle, const std::vector<InfoRow>& rows);
	// 读取并整理屏幕 EDID 详情。
	void LoadScreenInformation();
	// 将指定类别的信息导出为报告文件（命令行模式使用）。
	bool ExportReportToFile(const CString& reportType, const CString& filePath, CString& errorMessage);
	// 根据内容高度更新垂直滚动条状态。
	void UpdateVerticalScrollBar(int contentHeight, int viewHeight);
	// 重新计算侧栏与内容区矩形区域。
	void RecalcLayoutRects(const CRect& clientRect);
	// 动态创建设置页控件。
	void CreateSettingsControls();
	// 更新设置页控件位置与尺寸。
	void UpdateSettingsControlLayout();
	// 动态创建 SSD 页签控件。
	void CreateSsdControls();
	// 更新 SSD 页签控件位置与尺寸。
	void UpdateSsdControlLayout();
	// 动态创建启动项页控件。
	void CreateStartupControls();
	// 更新启动项页控件位置与尺寸。
	void UpdateStartupControlLayout();
	// 动态创建 powercfg 日志页详情按钮。
	void CreatePowerLogControls();
	// 更新 powercfg 日志页详情按钮位置与尺寸。
	void UpdatePowerLogControlLayout();
	// 导出并刷新指定 powercfg 日志页面摘要。
	void EnsurePowerCfgReport(bool batteryReport);
	// 强制重新导出指定 powercfg 日志页面摘要。
	void RefreshPowerCfgReport(bool batteryReport);
	// 使用默认浏览器打开指定 powercfg 日志。
	void OpenPowerCfgReport(bool batteryReport);
	// 读取启动项并刷新列表。
	void LoadStartupItems();
	// 将启动项数据同步到列表控件。
	void RefreshStartupList();
	// 根据当前选择刷新启动项按钮可用状态。
	void UpdateStartupButtons();
	// 设置启动项页状态文字。
	void SetStartupStatusText(const CString& text);
	// 获取当前选中的启动项下标。
	int GetSelectedStartupIndex() const;
	// 启用或禁用当前选中的启动项。
	bool SetSelectedStartupItemEnabled(bool enable);
	// 删除当前选中的启动项。
	bool DeleteSelectedStartupItem();
	// 添加当前用户开机启动项。
	bool AddStartupItem(const CString& name, const CString& command, CString& errorMessage);
	// 按当前 SSD 数据刷新页签。
	void RefreshSsdTabs();
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

	struct AsyncLoadRequest
	{
		HWND targetHwnd = nullptr;
	};

	struct AsyncLoadResult
	{
		std::vector<InfoRow> systemRows;
		std::vector<InfoRow> systemStatusRows;
		std::vector<InfoRow> acpiRows;
		std::vector<InfoRow> ssdRows;
		std::vector<InfoRow> screenRows;
		std::vector<std::vector<InfoRow>> ssdDiskRows;
		std::vector<CString> ssdTabTitles;
		int activeSsdIndex = 0;
	};

	enum class StartupSource
	{
		HkcuRun,
		HklmRun,
		HkcuRun32,
		HklmRun32,
		HkcuRunDisabled,
		HklmRunDisabled,
		ApprovedOnly,
		UserStartupFolder,
		CommonStartupFolder,
		UserStartupFolderDisabled,
		CommonStartupFolderDisabled
	};

	struct StartupItem
	{
		CString name;
		CString command;
		CString sourceLabel;
		CString location;
		CString disabledLocation;
		CString approvedSubKey;
		HKEY approvedRoot = nullptr;
		StartupSource source = StartupSource::HkcuRun;
		bool approvedOnly = false;
		bool enabled = true;
	};

	static UINT LoadSystemInformationThread(LPVOID parameter);
	static UINT LoadSsdInformationThread(LPVOID parameter);
	static UINT LoadScreenInformationThread(LPVOID parameter);
	static void PostAsyncLoadResult(HWND targetHwnd, UINT message, AsyncLoadResult* result);

	// 系统信息数据与绘制资源。
	std::vector<InfoRow> m_systemRows;
	std::vector<InfoRow> m_systemStatusRows;
	std::vector<InfoRow> m_acpiRows;
	std::vector<InfoRow> m_ssdRows;
	std::vector<InfoRow> m_screenRows;
	std::vector<std::vector<InfoRow>> m_ssdDiskRows;
	std::vector<CString> m_ssdTabTitles;
	CFont m_titleFont;
	CFont m_subtitleFont;
	CFont m_labelFont;
	CFont m_valueFont;
	CFont m_menuFont;
	CFont m_settingsFont;
	CFont m_startupListFont;
	CBrush m_uiBackgroundBrush;
	bool m_loading = true;
	int m_scrollPos = 0;
	int m_contentHeight = 0;
	int m_activePage = 0;
	CRect m_sideRect;
	CRect m_contentRect;
	CRect m_infoMenuRect;
	CRect m_statusMenuRect;
	CRect m_startupMenuRect;
	CRect m_acpiMenuRect;
	CRect m_settingsMenuRect;
	CRect m_ssdMenuRect;
	CRect m_screenMenuRect;
	CRect m_batteryLogMenuRect;
	CRect m_powerLogMenuRect;
	CRect m_ssdTabRect;
	int m_activeSsdIndex = 0;
	bool m_ssdLoaded = false;
	bool m_ssdLoading = false;
	bool m_screenLoaded = false;
	bool m_screenLoading = false;
	bool m_batteryLogLoaded = false;
	bool m_powerLogLoaded = false;
	CString m_batteryLogPath;
	CString m_powerLogPath;

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
	CTabCtrl m_ssdTab;
	CListCtrl m_startupList;
	CImageList m_startupRowImageList;
	CButton m_btnStartupEnable;
	CButton m_btnStartupDisable;
	CButton m_btnStartupDelete;
	CButton m_btnStartupRefresh;
	CStatic m_labelStartupName;
	CStatic m_labelStartupPath;
	CEdit m_editStartupName;
	CEdit m_editStartupPath;
	CButton m_btnStartupBrowse;
	CButton m_btnStartupAdd;
	CStatic m_startupStatusText;
	CButton m_btnBatteryLogDetails;
	CButton m_btnBatteryLogRefresh;
	CButton m_btnPowerLogDetails;
	CButton m_btnPowerLogRefresh;
	std::vector<InfoRow> m_batteryLogRows;
	std::vector<InfoRow> m_powerLogRows;
	std::vector<StartupItem> m_startupItems;
};
