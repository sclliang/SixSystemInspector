
// MFCApplication1.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include <shellapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	bool TryParseReportCommand(CString& reportType, CString& outputPath, CString& errorMessage, bool& hasReportCommand)
	{
		reportType.Empty();
		outputPath.Empty();
		errorMessage.Empty();
		hasReportCommand = false;

		int argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (argv == nullptr || argc <= 1)
		{
			if (argv != nullptr)
			{
				LocalFree(argv);
			}
			return true;
		}

		const bool reportSwitch = (_wcsicmp(argv[1], L"-Report") == 0);
		if (!reportSwitch)
		{
			LocalFree(argv);
			return true;
		}

		hasReportCommand = true;
		if (argc != 4)
		{
			errorMessage = _T("参数格式错误。请使用：-Report SSD|SYSTEM|EDID <FilePath>");
			LocalFree(argv);
			return false;
		}

		reportType = argv[2];
		outputPath = argv[3];
		LocalFree(argv);
		return true;
	}
}


// CMFCApplication1App

BEGIN_MESSAGE_MAP(CMFCApplication1App, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMFCApplication1App 构造

CMFCApplication1App::CMFCApplication1App()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CMFCApplication1App 对象

CMFCApplication1App theApp;


// CMFCApplication1App 初始化

BOOL CMFCApplication1App::InitInstance()
{
	// 如果应用程序存在以下情况，Windows XP 上需要 InitCommonControlsEx()
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("Sixunited\\SystemInspector"));

	CString reportType;
	CString reportPath;
	CString parseError;
	bool hasReportCommand = false;
	if (!TryParseReportCommand(reportType, reportPath, parseError, hasReportCommand))
	{
		AfxMessageBox(parseError, MB_ICONERROR | MB_OK);
		return FALSE;
	}
	if (hasReportCommand)
	{
		CMFCApplication1Dlg reportDlg;
		CString exportError;
		if (!reportDlg.ExportReportToFile(reportType, reportPath, exportError))
		{
			AfxMessageBox(exportError, MB_ICONERROR | MB_OK);
			return FALSE;
		}
		return FALSE;
	}

	CMFCApplication1Dlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	// 本程序是单对话框应用：关闭主对话框即结束进程。
	return FALSE;
}
