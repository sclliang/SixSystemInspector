// MFCApplication1Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"

#include <Wbemidl.h>
#include <comdef.h>
#include <atlbase.h>
#include <atlstr.h>
#include <set>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	constexpr UINT WM_APP_LOAD_SYSTEM_INFO = WM_APP + 100;
	constexpr int PAGE_SYSTEM_INFO = 0;
	constexpr int PAGE_SYSTEM_SETTINGS = 1;
	constexpr UINT IDC_CHK_UAC = 3001;
	constexpr UINT IDC_CHK_FIREWALL = 3002;
	constexpr UINT IDC_CHK_SEC_CENTER = 3003;
	constexpr UINT IDC_CHK_AUTO_REBOOT = 3004;
	constexpr UINT IDC_CHK_CRASH_DUMP = 3005;
	constexpr UINT IDC_CHK_SCREEN_SAVER = 3006;
	constexpr UINT IDC_CHK_POWER = 3007;
	constexpr UINT IDC_CHK_WINDOWS_UPDATE = 3008;
	constexpr UINT IDC_BTN_APPLY_SETTINGS = 3010;
	constexpr UINT IDC_BTN_REBOOT = 3011;
	constexpr UINT IDC_BTN_TOGGLE_SELECT = 3012;
	constexpr UINT IDC_STATUS_TEXT = 3013;
	constexpr UINT IDC_ADMIN_HINT_TEXT = 3014;
	constexpr UINT IDC_OPTIONS_START = IDC_CHK_UAC;
	constexpr UINT IDC_OPTIONS_END = IDC_CHK_WINDOWS_UPDATE;

	CString Trimmed(const CString& value)
	{
		CString result(value);
		result.Trim();
		return result;
	}

	bool HasValue(const CString& value)
	{
		const CString trimmed = Trimmed(value);
		return !trimmed.IsEmpty() && trimmed.CompareNoCase(_T("N/A")) != 0;
	}

	CString ToLower(const CString& value)
	{
		CString lowered(value);
		lowered.MakeLower();
		return lowered;
	}

	CString NormalizeMultilineValue(const CString& value)
	{
		CString normalized(value);
		normalized.Replace(_T(" / "), _T("\r\n"));
		normalized.Replace(_T(" | "), _T("\r\n"));
		return normalized;
	}

	bool IsGenericMonitorLabel(const CString& value)
	{
		const CString lowered = ToLower(Trimmed(value));
		return lowered.IsEmpty() ||
			lowered == _T("generic pnp monitor") ||
			lowered == _T("default monitor") ||
			lowered == _T("通用即插即用监视器") ||
			lowered == _T("默认监视器");
	}

	CString FormatBytesToGB(unsigned long long bytes)
	{
		const double sizeGb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
		CString text;
		text.Format(_T("%.1f GB"), sizeGb);
		return text;
	}

	unsigned long long ParseUnsignedLongLong(const CString& text)
	{
		const CString trimmed = Trimmed(text);
		if (trimmed.IsEmpty())
		{
			return 0;
		}
		return _wcstoui64(trimmed, nullptr, 10);
	}

	bool EnsureComInitialized()
	{
		static bool initialized = false;
		static bool attempted = false;
		if (attempted)
		{
			return initialized;
		}

		attempted = true;
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
		{
			return false;
		}

		hr = CoInitializeSecurity(
			nullptr,
			-1,
			nullptr,
			nullptr,
			RPC_C_AUTHN_LEVEL_DEFAULT,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			nullptr,
			EOAC_NONE,
			nullptr);

		if (FAILED(hr) && hr != RPC_E_TOO_LATE)
		{
			return false;
		}

		initialized = true;
		return true;
	}

	CString VariantToCString(VARIANT& var)
	{
		if (var.vt == VT_NULL || var.vt == VT_EMPTY)
		{
			return _T("");
		}

		if (var.vt == VT_BSTR && var.bstrVal != nullptr)
		{
			return CString(var.bstrVal);
		}

		if (var.vt == VT_BOOL)
		{
			return var.boolVal == VARIANT_TRUE ? _T("True") : _T("False");
		}

		if ((var.vt & VT_ARRAY) && ((var.vt & VT_TYPEMASK) == VT_UI2))
		{
			CString text;
			SAFEARRAY* arr = var.parray;
			if (arr == nullptr)
			{
				return _T("");
			}

			LONG lower = 0;
			LONG upper = -1;
			SafeArrayGetLBound(arr, 1, &lower);
			SafeArrayGetUBound(arr, 1, &upper);

			for (LONG i = lower; i <= upper; ++i)
			{
				USHORT ch = 0;
				if (SUCCEEDED(SafeArrayGetElement(arr, &i, &ch)))
				{
					if (ch == 0)
					{
						break;
					}
					text.AppendChar(static_cast<WCHAR>(ch));
				}
			}

			text.Trim();
			return text;
		}

		if ((var.vt & VT_ARRAY) && ((var.vt & VT_TYPEMASK) == VT_UI1))
		{
			CString text;
			SAFEARRAY* arr = var.parray;
			if (arr == nullptr)
			{
				return _T("");
			}

			LONG lower = 0;
			LONG upper = -1;
			SafeArrayGetLBound(arr, 1, &lower);
			SafeArrayGetUBound(arr, 1, &upper);

			for (LONG i = lower; i <= upper; ++i)
			{
				unsigned char ch = 0;
				if (SUCCEEDED(SafeArrayGetElement(arr, &i, &ch)))
				{
					if (ch == 0)
					{
						break;
					}
					text.AppendChar(static_cast<WCHAR>(ch));
				}
			}

			text.Trim();
			return text;
		}

		_variant_t converted;
		const HRESULT hr = VariantChangeType(&converted, &var, 0, VT_BSTR);
		if (SUCCEEDED(hr) && converted.vt == VT_BSTR && converted.bstrVal != nullptr)
		{
			return CString(converted.bstrVal);
		}

		return _T("");
	}

	std::vector<std::vector<CString>> QueryWmiRows(
		const CString& wmiNamespace,
		const CString& wql,
		const std::vector<CString>& properties)
	{
		std::vector<std::vector<CString>> rows;
		if (!EnsureComInitialized())
		{
			return rows;
		}

		IWbemLocator* locator = nullptr;
		HRESULT hr = CoCreateInstance(
			CLSID_WbemLocator,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_IWbemLocator,
			reinterpret_cast<LPVOID*>(&locator));
		if (FAILED(hr) || locator == nullptr)
		{
			return rows;
		}

		IWbemServices* services = nullptr;
		hr = locator->ConnectServer(
			_bstr_t(wmiNamespace),
			nullptr,
			nullptr,
			nullptr,
			0,
			nullptr,
			nullptr,
			&services);
		if (FAILED(hr) || services == nullptr)
		{
			locator->Release();
			return rows;
		}

		hr = CoSetProxyBlanket(
			services,
			RPC_C_AUTHN_WINNT,
			RPC_C_AUTHZ_NONE,
			nullptr,
			RPC_C_AUTHN_LEVEL_CALL,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			nullptr,
			EOAC_NONE);
		if (FAILED(hr))
		{
			services->Release();
			locator->Release();
			return rows;
		}

		IEnumWbemClassObject* enumerator = nullptr;
		hr = services->ExecQuery(
			_bstr_t(L"WQL"),
			_bstr_t(wql),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			nullptr,
			&enumerator);
		if (FAILED(hr) || enumerator == nullptr)
		{
			services->Release();
			locator->Release();
			return rows;
		}

		while (true)
		{
			IWbemClassObject* object = nullptr;
			ULONG returned = 0;
			hr = enumerator->Next(WBEM_INFINITE, 1, &object, &returned);
			if (FAILED(hr) || returned == 0 || object == nullptr)
			{
				break;
			}

			std::vector<CString> row;
			row.reserve(properties.size());
			for (const CString& prop : properties)
			{
				VARIANT value;
				VariantInit(&value);
				const HRESULT getHr = object->Get(_bstr_t(prop), 0, &value, nullptr, nullptr);
				if (SUCCEEDED(getHr))
				{
					row.push_back(Trimmed(VariantToCString(value)));
				}
				else
				{
					row.push_back(_T(""));
				}
				VariantClear(&value);
			}

			rows.push_back(row);
			object->Release();
		}

		enumerator->Release();
		services->Release();
		locator->Release();
		return rows;
	}

	CString FirstValue(const CString& wmiNamespace, const CString& wql, const CString& property)
	{
		const auto rows = QueryWmiRows(wmiNamespace, wql, { property });
		for (const auto& row : rows)
		{
			if (!row.empty() && HasValue(row[0]))
			{
				return row[0];
			}
		}
		return _T("");
	}

	CString JoinValues(const CString& wmiNamespace, const CString& wql, const CString& property)
	{
		const auto rows = QueryWmiRows(wmiNamespace, wql, { property });
		std::set<CString, std::less<>> unique;
		for (const auto& row : rows)
		{
			if (!row.empty() && HasValue(row[0]))
			{
				unique.insert(row[0]);
			}
		}

		CString result;
		for (const CString& value : unique)
		{
			if (!result.IsEmpty())
			{
				result += _T(" / ");
			}
			result += value;
		}
		return result;
	}

	CString ResolveMonitorModel()
	{
		const auto monitorRows = QueryWmiRows(
			_T("ROOT\\WMI"),
			_T("SELECT UserFriendlyName FROM WmiMonitorID"),
			{ _T("UserFriendlyName") });

		std::set<CString, std::less<>> monitorNames;
		for (const auto& row : monitorRows)
		{
			if (!row.empty() && HasValue(row[0]) && !IsGenericMonitorLabel(row[0]))
			{
				monitorNames.insert(row[0]);
			}
		}

		CString monitor;
		for (const CString& value : monitorNames)
		{
			if (!monitor.IsEmpty())
			{
				monitor += _T("\r\n");
			}
			monitor += value;
		}

		if (HasValue(monitor))
		{
			return monitor;
		}

		const auto pnpRows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name FROM Win32_PnPEntity WHERE PNPClass='Monitor'"),
			{ _T("Name") });
		for (const auto& row : pnpRows)
		{
			if (!row.empty() && HasValue(row[0]) && !IsGenericMonitorLabel(row[0]))
			{
				if (!monitor.IsEmpty())
				{
					monitor += _T("\r\n");
				}
				monitor += row[0];
			}
		}

		if (HasValue(monitor))
		{
			return monitor;
		}

		monitor = JoinValues(
				_T("ROOT\\CIMV2"),
				_T("SELECT Name FROM Win32_DesktopMonitor"),
				_T("Name"));
		return monitor;
	}

	CString ResolveMemoryInfo()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Capacity, Speed, Manufacturer, PartNumber FROM Win32_PhysicalMemory"),
			{ _T("Capacity"), _T("Speed"), _T("Manufacturer"), _T("PartNumber") });

		if (rows.empty())
		{
			return _T("");
		}

		unsigned long long totalBytes = 0;
		std::vector<CString> modules;
		modules.reserve(rows.size());

		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			const unsigned long long bytes = ParseUnsignedLongLong(row[0]);
			totalBytes += bytes;

			CString moduleText = HasValue(row[0]) ? FormatBytesToGB(bytes) : _T("容量未知");
			if (HasValue(row[1]))
			{
				moduleText += _T(" ");
				moduleText += row[1];
				moduleText += _T("MHz");
			}
			if (HasValue(row[2]))
			{
				moduleText += _T(" ");
				moduleText += row[2];
			}
			if (HasValue(row[3]))
			{
				moduleText += _T(" ");
				moduleText += row[3];
			}

			modules.push_back(moduleText);
		}

		CString result;
		if (totalBytes > 0)
		{
			result += _T("总计 ");
			result += FormatBytesToGB(totalBytes);
			result += _T("\r\n");
		}

		result.AppendFormat(_T("内存条: %u\r\n"), static_cast<unsigned>(modules.size()));
		for (size_t i = 0; i < modules.size(); ++i)
		{
			if (i != 0)
			{
				result += _T("\r\n");
			}
			CString prefix;
			prefix.Format(_T("内存%u: "), static_cast<unsigned>(i + 1));
			result += prefix;
			result += modules[i];
		}
		return result;
	}

	CString ResolveDiskInfo()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Model, Size, MediaType, InterfaceType FROM Win32_DiskDrive"),
			{ _T("Model"), _T("Size"), _T("MediaType"), _T("InterfaceType") });

		if (rows.empty())
		{
			return _T("");
		}

		std::vector<CString> allDisks;

		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			const CString& model = row[0];
			const CString& size = row[1];
			const CString& interfaceType = row[3];

			CString diskText = model;
			if (HasValue(size))
			{
				const unsigned long long bytes = ParseUnsignedLongLong(size);
				if (bytes > 0)
				{
					diskText += _T(" ");
					diskText += FormatBytesToGB(bytes);
				}
			}
			if (HasValue(interfaceType))
			{
				diskText += _T(" ");
				diskText += interfaceType;
			}

			allDisks.push_back(diskText);
		}

		CString result;
		for (size_t i = 0; i < allDisks.size(); ++i)
		{
			if (i != 0)
			{
				result += _T("\r\n");
			}
			result += allDisks[i];
		}
		return result;
	}

	CString ResolveEcVersion()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT EmbeddedControllerMajorVersion, EmbeddedControllerMinorVersion FROM Win32_BIOS"),
			{ _T("EmbeddedControllerMajorVersion"), _T("EmbeddedControllerMinorVersion") });

		if (rows.empty() || rows[0].size() < 2)
		{
			return _T("");
		}

		const CString& majorText = rows[0][0];
		const CString& minorText = rows[0][1];
		if (!HasValue(majorText) || !HasValue(minorText))
		{
			return _T("");
		}

		const int major = _wtoi(majorText);
		const int minor = _wtoi(minorText);
		CString version;
		version.Format(_T("%d.%d"), major, minor);
		return version;
	}

	CString ResolveMacAddress(bool wireless)
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, MACAddress, PhysicalAdapter, PNPDeviceID FROM Win32_NetworkAdapter WHERE MACAddress IS NOT NULL"),
			{ _T("Name"), _T("MACAddress"), _T("PhysicalAdapter"), _T("PNPDeviceID") });

		std::set<CString, std::less<>> macs;
		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			const CString& name = row[0];
			const CString& mac = row[1];
			const CString& physical = row[2];
			const CString& pnp = row[3];

			if (!HasValue(mac))
			{
				continue;
			}
			if (physical.CompareNoCase(_T("True")) != 0)
			{
				continue;
			}

			const CString matcher = ToLower(name + _T(" ") + pnp);
			const bool isWireless =
				matcher.Find(_T("wireless")) >= 0 ||
				matcher.Find(_T("wi-fi")) >= 0 ||
				matcher.Find(_T("wifi")) >= 0 ||
				matcher.Find(_T("802.11")) >= 0 ||
				matcher.Find(_T("wlan")) >= 0;
			const bool isBluetooth = matcher.Find(_T("bluetooth")) >= 0;

			if (wireless)
			{
				if (isWireless)
				{
					macs.insert(mac);
				}
			}
			else
			{
				if (!isWireless && !isBluetooth)
				{
					macs.insert(mac);
				}
			}
		}

		CString result;
		for (const CString& mac : macs)
		{
			if (!result.IsEmpty())
			{
				result += _T(" / ");
			}
			result += mac;
		}
		return result;
	}

	CString ResolveBluetoothAddress()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, MACAddress, PhysicalAdapter, PNPDeviceID FROM Win32_NetworkAdapter WHERE MACAddress IS NOT NULL"),
			{ _T("Name"), _T("MACAddress"), _T("PhysicalAdapter"), _T("PNPDeviceID") });

		std::set<CString, std::less<>> macs;
		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			const CString& name = row[0];
			const CString& mac = row[1];
			const CString& physical = row[2];
			const CString& pnp = row[3];
			if (!HasValue(mac))
			{
				continue;
			}

			const CString matcher = ToLower(name + _T(" ") + pnp);
			if (matcher.Find(_T("bluetooth")) >= 0 &&
				(physical.CompareNoCase(_T("True")) == 0 || physical.IsEmpty()))
			{
				macs.insert(mac);
			}
		}

		CString result;
		for (const CString& mac : macs)
		{
			if (!result.IsEmpty())
			{
				result += _T("\r\n");
			}
			result += mac;
		}
		return result;
	}

	CString ResolveTpmModel()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2\\Security\\MicrosoftTpm"),
			_T("SELECT ManufacturerIdTxt, ManufacturerVersionInfo, SpecVersion FROM Win32_Tpm"),
			{ _T("ManufacturerIdTxt"), _T("ManufacturerVersionInfo"), _T("SpecVersion") });

		if (rows.empty() || rows[0].size() < 3)
		{
			return _T("");
		}

		CString model;
		if (HasValue(rows[0][0]))
		{
			model += rows[0][0];
		}
		if (HasValue(rows[0][1]))
		{
			if (!model.IsEmpty())
			{
				model += _T(" ");
			}
			model += rows[0][1];
		}
		if (HasValue(rows[0][2]))
		{
			if (!model.IsEmpty())
			{
				model += _T(" ");
			}
			model += _T("(Spec ");
			model += rows[0][2];
			model += _T(")");
		}
		return model;
	}
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg()
	: CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CMFCApplication1Dlg 对话框
CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_APP_LOAD_SYSTEM_INFO, &CMFCApplication1Dlg::OnLoadSystemInformation)
	ON_BN_CLICKED(IDC_BTN_APPLY_SETTINGS, &CMFCApplication1Dlg::OnBnClickedApplySettings)
	ON_BN_CLICKED(IDC_BTN_REBOOT, &CMFCApplication1Dlg::OnBnClickedRebootSystem)
	ON_BN_CLICKED(IDC_BTN_TOGGLE_SELECT, &CMFCApplication1Dlg::OnBnClickedToggleSelect)
	ON_COMMAND_RANGE(IDC_OPTIONS_START, IDC_OPTIONS_END, &CMFCApplication1Dlg::OnSettingsOptionChanged)
END_MESSAGE_MAP()

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		CString strAboutMenu;
		if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	BuildMainLayout();
	CreateSettingsControls();
	UpdatePageVisibility();
	m_loading = true;
	PostMessage(WM_APP_LOAD_SYSTEM_INFO, 0, 0);

	return TRUE;
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		const int cxIcon = GetSystemMetrics(SM_CXICON);
		const int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		const int x = (rect.Width() - cxIcon + 1) / 2;
		const int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect clientRect;
		GetClientRect(&clientRect);
		DrawSystemInformation(dc, clientRect);
	}
}

HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCApplication1Dlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	AdjustLayout(cx, cy);
	if (cx > 0 && cy > 0)
	{
		Invalidate();
	}
}

void CMFCApplication1Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_infoMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_SYSTEM_INFO)
		{
			m_activePage = PAGE_SYSTEM_INFO;
			UpdatePageVisibility();
			Invalidate();
		}
	}
	else if (m_settingsMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_SYSTEM_SETTINGS)
		{
			m_activePage = PAGE_SYSTEM_SETTINGS;
			UpdatePageVisibility();
			Invalidate();
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCApplication1Dlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_activePage != PAGE_SYSTEM_INFO && m_activePage != PAGE_SYSTEM_SETTINGS)
	{
		CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	UNREFERENCED_PARAMETER(pScrollBar);

	SCROLLINFO si = {};
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &si);

	int nextPos = m_scrollPos;
	switch (nSBCode)
	{
	case SB_LINEUP:
		nextPos -= 24;
		break;
	case SB_LINEDOWN:
		nextPos += 24;
		break;
	case SB_PAGEUP:
		nextPos -= static_cast<int>(si.nPage);
		break;
	case SB_PAGEDOWN:
		nextPos += static_cast<int>(si.nPage);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nextPos = static_cast<int>(si.nTrackPos);
		break;
	case SB_TOP:
		nextPos = 0;
		break;
	case SB_BOTTOM:
		nextPos = si.nMax;
		break;
	default:
		break;
	}

	const int maxPos = max(0, si.nMax - static_cast<int>(si.nPage) + 1);
	nextPos = max(0, min(nextPos, maxPos));
	if (nextPos != m_scrollPos)
	{
		m_scrollPos = nextPos;
		SetScrollPos(SB_VERT, m_scrollPos, TRUE);
		if (m_activePage == PAGE_SYSTEM_SETTINGS)
		{
			UpdateSettingsControlLayout();
		}
		Invalidate();
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CMFCApplication1Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_activePage != PAGE_SYSTEM_INFO && m_activePage != PAGE_SYSTEM_SETTINGS)
	{
		return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
	}

	UNREFERENCED_PARAMETER(nFlags);
	UNREFERENCED_PARAMETER(pt);

	if (m_contentHeight <= 0)
	{
		return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
	}

	const int step = 36;
	int nextPos = m_scrollPos - (zDelta / WHEEL_DELTA) * step;

	SCROLLINFO si = {};
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	GetScrollInfo(SB_VERT, &si);
	const int maxPos = max(0, si.nMax - static_cast<int>(si.nPage) + 1);
	nextPos = max(0, min(nextPos, maxPos));

	if (nextPos != m_scrollPos)
	{
		m_scrollPos = nextPos;
		SetScrollPos(SB_VERT, m_scrollPos, TRUE);
		if (m_activePage == PAGE_SYSTEM_SETTINGS)
		{
			UpdateSettingsControlLayout();
		}
		Invalidate();
		return TRUE;
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CMFCApplication1Dlg::BuildMainLayout()
{
	EnsureUiFonts();
	SetWindowText(_T("SixSystemInspector"));

	CRect clientRect;
	GetClientRect(&clientRect);
	RecalcLayoutRects(clientRect);
}

void CMFCApplication1Dlg::AdjustLayout(int cx, int cy)
{
	CRect clientRect(0, 0, cx, cy);
	RecalcLayoutRects(clientRect);
	UpdateSettingsControlLayout();
}

void CMFCApplication1Dlg::AddSystemInfoRow(const CString& item, const CString& value)
{
	InfoRow row;
	row.item = item;
	row.value = HasValue(value) ? NormalizeMultilineValue(value) : _T("N/A");
	m_systemRows.push_back(row);
}

LRESULT CMFCApplication1Dlg::OnLoadSystemInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	LoadSystemInformation();
	return 0;
}

void CMFCApplication1Dlg::LoadSystemInformation()
{
	m_systemRows.clear();
	m_scrollPos = 0;

	const CString boardModel = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT Product FROM Win32_BaseBoard"), _T("Product"));
	const CString cpuModel = JoinValues(_T("ROOT\\CIMV2"), _T("SELECT Name FROM Win32_Processor"), _T("Name"));
	const CString gpuModel = JoinValues(_T("ROOT\\CIMV2"), _T("SELECT Name FROM Win32_VideoController"), _T("Name"));
	const CString monitorModel = ResolveMonitorModel();
	const CString memoryInfo = ResolveMemoryInfo();
	const CString ssdInfo = ResolveDiskInfo();
	const CString biosVersion = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT SMBIOSBIOSVersion FROM Win32_BIOS"), _T("SMBIOSBIOSVersion"));
	const CString ecVersion = ResolveEcVersion();
	const CString boardSn = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT SerialNumber FROM Win32_BaseBoard"), _T("SerialNumber"));
	const CString systemSn = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT SerialNumber FROM Win32_BIOS"), _T("SerialNumber"));
	const CString uuid = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT UUID FROM Win32_ComputerSystemProduct"), _T("UUID"));
	const CString wiredMac = ResolveMacAddress(false);
	const CString wirelessMac = ResolveMacAddress(true);
	const CString bluetoothAddress = ResolveBluetoothAddress();
	const CString tpmModel = ResolveTpmModel();

	AddSystemInfoRow(_T("主板型号"), boardModel);
	AddSystemInfoRow(_T("CPU型号"), cpuModel);
	AddSystemInfoRow(_T("GPU型号"), gpuModel);
	AddSystemInfoRow(_T("显示器型号"), monitorModel);
	AddSystemInfoRow(_T("内存信息"), memoryInfo);
	AddSystemInfoRow(_T("SSD信息"), ssdInfo);
	AddSystemInfoRow(_T("BIOS版本"), biosVersion);
	AddSystemInfoRow(_T("EC版本"), ecVersion);
	AddSystemInfoRow(_T("主板SN"), boardSn);
	AddSystemInfoRow(_T("系统SN"), systemSn);
	AddSystemInfoRow(_T("UUID"), uuid);
	AddSystemInfoRow(_T("有线网卡地址"), wiredMac);
	AddSystemInfoRow(_T("无线网卡地址"), wirelessMac);
	AddSystemInfoRow(_T("蓝牙地址"), bluetoothAddress);
	AddSystemInfoRow(_T("TPM型号"), tpmModel);

	m_loading = false;
	Invalidate();
}

BOOL CMFCApplication1Dlg::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}

void CMFCApplication1Dlg::EnsureUiFonts()
{
	if (m_titleFont.GetSafeHandle() == nullptr)
	{
		m_titleFont.CreatePointFont(138, _T("Microsoft YaHei UI"));
	}
	if (m_subtitleFont.GetSafeHandle() == nullptr)
	{
		m_subtitleFont.CreatePointFont(92, _T("Microsoft YaHei UI"));
	}
	if (m_labelFont.GetSafeHandle() == nullptr)
	{
		m_labelFont.CreatePointFont(118, _T("Microsoft YaHei UI"));
	}
	if (m_valueFont.GetSafeHandle() == nullptr)
	{
		m_valueFont.CreatePointFont(118, _T("Microsoft YaHei UI"));
	}
	if (m_menuFont.GetSafeHandle() == nullptr)
	{
		m_menuFont.CreatePointFont(145, _T("Microsoft YaHei UI"));
	}
	if (m_settingsFont.GetSafeHandle() == nullptr)
	{
		m_settingsFont.CreatePointFont(98, _T("Microsoft YaHei UI"));
	}
	if (m_uiBackgroundBrush.GetSafeHandle() == nullptr)
	{
		m_uiBackgroundBrush.CreateSolidBrush(RGB(240, 244, 249));
	}
}

void CMFCApplication1Dlg::DrawRoundedCard(CDC& dc, const CRect& rect, COLORREF fillColor, int radius)
{
	CPen pen(PS_SOLID, 1, fillColor);
	CBrush brush(fillColor);
	CPen* oldPen = dc.SelectObject(&pen);
	CBrush* oldBrush = dc.SelectObject(&brush);
	dc.RoundRect(rect, CPoint(radius, radius));
	dc.SelectObject(oldPen);
	dc.SelectObject(oldBrush);
}

void CMFCApplication1Dlg::DrawSystemInformation(CDC& dc, const CRect& clientRect)
{
	EnsureUiFonts();

	CDC memDc;
	memDc.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* oldBitmap = memDc.SelectObject(&bitmap);

	memDc.FillSolidRect(clientRect, RGB(240, 244, 249));
	memDc.SetBkMode(TRANSPARENT);

	DrawRoundedCard(memDc, m_sideRect, RGB(212, 224, 239), 14);
	DrawRoundedCard(memDc, m_contentRect, RGB(236, 242, 249), 14);

	const COLORREF selectedColor = RGB(88, 130, 190);
	const COLORREF unselectedColor = RGB(200, 214, 230);

	DrawRoundedCard(memDc, m_infoMenuRect, m_activePage == PAGE_SYSTEM_INFO ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_settingsMenuRect, m_activePage == PAGE_SYSTEM_SETTINGS ? selectedColor : unselectedColor, 10);

	memDc.SelectObject(&m_menuFont);
	memDc.SetTextColor(m_activePage == PAGE_SYSTEM_INFO ? RGB(255, 255, 255) : RGB(52, 72, 98));
	memDc.DrawText(_T("系统信息"), m_infoMenuRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	memDc.SetTextColor(m_activePage == PAGE_SYSTEM_SETTINGS ? RGB(255, 255, 255) : RGB(52, 72, 98));
	memDc.DrawText(_T("系统设置"), m_settingsMenuRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	if (m_activePage == PAGE_SYSTEM_SETTINGS)
	{
		DrawSystemSettings(memDc, clientRect);
	}
	else
	{
		CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 104);
		CRect listRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
		DrawRoundedCard(memDc, headerRect, RGB(222, 230, 240), 12);
		DrawRoundedCard(memDc, listRect, RGB(236, 242, 249), 12);

		memDc.SelectObject(&m_titleFont);
		memDc.SetTextColor(RGB(19, 33, 52));
		CRect titleRect(headerRect.left + 18, headerRect.top + 12, headerRect.right - 16, headerRect.top + 48);
		memDc.DrawText(_T("系统信息概览"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

		memDc.SelectObject(&m_subtitleFont);
		memDc.SetTextColor(RGB(85, 100, 120));
		CRect subtitleRect(headerRect.left + 18, headerRect.top + 56, headerRect.right - 16, headerRect.bottom - 10);
		memDc.DrawText(_T("快速查看当前设备的关键硬件、固件与网络标识信息"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

		const int labelX = listRect.left + 16;
		const int labelWidth = 178;
		const int valueX = labelX + labelWidth + 34;
		const int contentTop = listRect.top + 14;
		const int contentBottom = listRect.bottom - 12;
		const int contentWidth = (listRect.right - 16) - valueX;
		const int valueRight = valueX + contentWidth;

		TEXTMETRIC labelTm = {};
		memDc.SelectObject(&m_labelFont);
		memDc.GetTextMetrics(&labelTm);
		const int labelLineHeight = max(static_cast<int>(labelTm.tmHeight + labelTm.tmExternalLeading), 24);

		TEXTMETRIC valueTm = {};
		memDc.SelectObject(&m_valueFont);
		memDc.GetTextMetrics(&valueTm);
		const int valueLineHeight = max(static_cast<int>(valueTm.tmHeight + valueTm.tmExternalLeading), 24);
		const int rowPadding = 8;
		const int rowGap = 10;

		std::vector<int> rowHeights;
		rowHeights.reserve(m_systemRows.size());
		int totalHeight = 14;
		for (const InfoRow& row : m_systemRows)
		{
			CRect calcRect(0, 0, max(contentWidth, 80), 0);
			memDc.SelectObject(&m_valueFont);
			memDc.DrawText(row.value, calcRect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
			const int valueHeight = max(valueLineHeight, calcRect.Height());
			const int rowHeight = max(labelLineHeight, valueHeight) + rowPadding;
			rowHeights.push_back(rowHeight);
			totalHeight += rowHeight + rowGap;
		}
		totalHeight += 10;
		UpdateVerticalScrollBar(totalHeight, max(1, listRect.Height() - 4));
		int y = contentTop - m_scrollPos;

		if (m_loading)
		{
			memDc.SelectObject(&m_valueFont);
			memDc.SetTextColor(RGB(60, 75, 95));
			memDc.TextOutW(labelX, y, _T("正在加载系统信息，请稍候..."));
		}
		else
		{
			const int oldDc = memDc.SaveDC();
			memDc.IntersectClipRect(listRect.left + 6, listRect.top + 6, listRect.right - 6, listRect.bottom - 6);

			for (size_t i = 0; i < m_systemRows.size(); ++i)
			{
				const InfoRow& row = m_systemRows[i];
				const int rowHeight = i < rowHeights.size() ? rowHeights[i] : (labelLineHeight + rowPadding);
				memDc.SelectObject(&m_labelFont);
				memDc.SetTextColor(RGB(93, 108, 128));
				CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
				memDc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

				memDc.SelectObject(&m_valueFont);
				memDc.SetTextColor(RGB(50, 64, 83));
				CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
				memDc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

				CPen linePen(PS_SOLID, 1, RGB(220, 228, 238));
				CPen* oldLinePen = memDc.SelectObject(&linePen);
				const int separatorY = y + rowHeight + (rowGap / 2);
				memDc.MoveTo(labelX, separatorY);
				memDc.LineTo(valueRight, separatorY);
				memDc.SelectObject(oldLinePen);

				y += rowHeight + rowGap;
				if (y > contentBottom + 20)
				{
					break;
				}
			}
			memDc.RestoreDC(oldDc);
		}
	}

	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDc, 0, 0, SRCCOPY);
	memDc.SelectObject(oldBitmap);
}

void CMFCApplication1Dlg::DrawSystemSettings(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	TEXTMETRIC tmLabel = {};
	TEXTMETRIC tmSettings = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&tmLabel);
	dc.SelectObject(&m_settingsFont);
	dc.GetTextMetrics(&tmSettings);

	const int titleHeight = max(24, static_cast<int>(tmLabel.tmHeight + tmLabel.tmExternalLeading + 6));
	const int itemHeight = max(22, static_cast<int>(tmSettings.tmHeight + tmSettings.tmExternalLeading + 6));
	const int itemGap = 4;
	const int titleSpace = titleHeight + 8;
	const int groupBottomPadding = 10;
	const int groupHeightLarge = titleSpace + itemHeight * 3 + itemGap * 2 + groupBottomPadding;
	const int groupHeightSmall = titleSpace + itemHeight + groupBottomPadding;
	const int optionInnerMargin = 12;
	const int groupGapY = 10;

	const int top = m_contentRect.top + 10 - m_scrollPos;
	const int headerHeight = 84;
	const int buttonHeight = 52;
	const int statusHeight = max(64, static_cast<int>(tmSettings.tmHeight + tmSettings.tmExternalLeading) * 2 + 22);
	const int gap1 = 10;
	const int gap2 = 10;
	const int gap3 = 8;

	CRect headerRect(m_contentRect.left + 12, top, m_contentRect.right - 12, top + headerHeight);
	const int optionsWidth = headerRect.Width() - optionInnerMargin * 2;
	const bool singleColumn = optionsWidth < 560;
	const int optionsHeight = singleColumn
		? optionInnerMargin * 2 + groupHeightLarge + groupGapY + groupHeightLarge + groupGapY + groupHeightSmall + groupGapY + groupHeightSmall
		: optionInnerMargin * 2 + groupHeightLarge + groupGapY + groupHeightSmall;

	CRect optionsCardRect(m_contentRect.left + 12, headerRect.bottom + gap1, m_contentRect.right - 12, headerRect.bottom + gap1 + optionsHeight);
	CRect buttonCardRect(m_contentRect.left + 12, optionsCardRect.bottom + gap2, m_contentRect.right - 12, optionsCardRect.bottom + gap2 + buttonHeight);
	CRect statusCardRect(m_contentRect.left + 12, buttonCardRect.bottom + gap3, m_contentRect.right - 12, buttonCardRect.bottom + gap3 + statusHeight);

	const int totalHeight = 10 + headerHeight + gap1 + optionsHeight + gap2 + buttonHeight + gap3 + statusHeight + 10;
	UpdateVerticalScrollBar(totalHeight, max(1, m_contentRect.Height() - 16));

	DrawRoundedCard(dc, headerRect, RGB(222, 230, 240), 12);
	DrawRoundedCard(dc, optionsCardRect, RGB(250, 252, 254), 10);
	DrawRoundedCard(dc, buttonCardRect, RGB(247, 250, 253), 8);
	DrawRoundedCard(dc, statusCardRect, RGB(248, 250, 252), 8);

	const int optionTop = optionsCardRect.top + optionInnerMargin;
	const int optionWidth = optionsCardRect.Width() - optionInnerMargin * 2;
	const int colGap = 14;
	const int colWidth = singleColumn ? optionWidth : (optionWidth - colGap) / 2;

	CRect grpSecurity(optionsCardRect.left + optionInnerMargin, optionTop, optionsCardRect.left + optionInnerMargin + colWidth, optionTop + groupHeightLarge);
	CRect grpBehavior = singleColumn
		? CRect(grpSecurity.left, grpSecurity.bottom + groupGapY, grpSecurity.right, grpSecurity.bottom + groupGapY + groupHeightLarge)
		: CRect(grpSecurity.right + colGap, optionTop, grpSecurity.right + colGap + colWidth, optionTop + groupHeightLarge);
	CRect grpPower = singleColumn
		? CRect(grpBehavior.left, grpBehavior.bottom + groupGapY, grpBehavior.right, grpBehavior.bottom + groupGapY + groupHeightSmall)
		: CRect(grpSecurity.left, grpSecurity.bottom + groupGapY, grpSecurity.right, grpSecurity.bottom + groupGapY + groupHeightSmall);
	CRect grpUpdate = singleColumn
		? CRect(grpPower.left, grpPower.bottom + groupGapY, grpPower.right, grpPower.bottom + groupGapY + groupHeightSmall)
		: CRect(grpBehavior.left, grpBehavior.bottom + groupGapY, grpBehavior.right, grpBehavior.bottom + groupGapY + groupHeightSmall);

	DrawRoundedCard(dc, grpSecurity, RGB(240, 244, 249), 8);
	DrawRoundedCard(dc, grpBehavior, RGB(240, 244, 249), 8);
	DrawRoundedCard(dc, grpPower, RGB(240, 244, 249), 8);
	DrawRoundedCard(dc, grpUpdate, RGB(240, 244, 249), 8);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(RGB(19, 33, 52));
	CRect titleRect(headerRect.left + 16, headerRect.top + 10, headerRect.right - 16, headerRect.top + 40);
	dc.DrawText(_T("系统设置优化"), titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(RGB(85, 100, 120));
	CRect subtitleRect(headerRect.left + 16, headerRect.top + 40, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("勾选后可执行系统优化项"), subtitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	dc.SelectObject(&m_labelFont);
	dc.SetTextColor(RGB(64, 84, 106));
	CRect groupTitleRect = grpSecurity;
	groupTitleRect.DeflateRect(10, 8);
	groupTitleRect.bottom = groupTitleRect.top + titleHeight;
	dc.DrawText(_T("安全相关"), groupTitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	groupTitleRect = grpBehavior;
	groupTitleRect.DeflateRect(10, 8);
	groupTitleRect.bottom = groupTitleRect.top + titleHeight;
	dc.DrawText(_T("系统行为"), groupTitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	groupTitleRect = grpPower;
	groupTitleRect.DeflateRect(10, 8);
	groupTitleRect.bottom = groupTitleRect.top + titleHeight;
	dc.DrawText(_T("电源管理"), groupTitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	groupTitleRect = grpUpdate;
	groupTitleRect.DeflateRect(10, 8);
	groupTitleRect.bottom = groupTitleRect.top + titleHeight;
	dc.DrawText(_T("更新与维护"), groupTitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

void CMFCApplication1Dlg::UpdateVerticalScrollBar(int contentHeight, int viewHeight)
{
	m_contentHeight = max(contentHeight, 0);
	const int maxPos = max(0, m_contentHeight - viewHeight);
	m_scrollPos = max(0, min(m_scrollPos, maxPos));

	SCROLLINFO si = {};
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nMax = max(0, m_contentHeight - 1);
	si.nPage = static_cast<UINT>(max(viewHeight, 1));
	si.nPos = m_scrollPos;
	SetScrollInfo(SB_VERT, &si, TRUE);
	ShowScrollBar(SB_VERT, m_contentHeight > viewHeight);
}

void CMFCApplication1Dlg::RecalcLayoutRects(const CRect& clientRect)
{
	const int margin = 12;
	const int availableWidth = max(320, clientRect.Width() - margin * 2);
	const int sideWidth = max(108, min(220, availableWidth / 4));
	m_sideRect = CRect(margin, margin, margin + sideWidth, clientRect.bottom - margin);
	m_contentRect = CRect(m_sideRect.right + 10, margin, clientRect.right - margin, clientRect.bottom - margin);
	m_infoMenuRect = CRect(m_sideRect.left + 10, m_sideRect.top + 18, m_sideRect.right - 10, m_sideRect.top + 70);
	m_settingsMenuRect = CRect(m_sideRect.left + 10, m_infoMenuRect.bottom + 10, m_sideRect.right - 10, m_infoMenuRect.bottom + 62);
}

void CMFCApplication1Dlg::CreateSettingsControls()
{
	const DWORD chkStyle = WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX;
	m_chkUAC.Create(_T("禁用 UAC（用户账户控制）"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_UAC);
	m_chkFirewall.Create(_T("禁用 Windows 防火墙"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_FIREWALL);
	m_chkSecCenter.Create(_T("禁用安全中心警告（防病毒/防火墙/更新）"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_SEC_CENTER);
	m_chkAutoReboot.Create(_T("禁用系统自动重启（崩溃时）"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_AUTO_REBOOT);
	m_chkCrashDump.Create(_T("启用完整内存转储（崩溃诊断）"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_CRASH_DUMP);
	m_chkScreenSaver.Create(_T("禁用屏幕保护程序"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_SCREEN_SAVER);
	m_chkPower.Create(_T("电源设置为永不睡眠/关屏/关硬盘"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_POWER);
	m_chkWindowsUpdate.Create(_T("禁用 Windows 自动更新"), chkStyle, CRect(0, 0, 10, 10), this, IDC_CHK_WINDOWS_UPDATE);

	for (CButton* checkBox : GetOptionCheckBoxes())
	{
		checkBox->SetFont(&m_settingsFont);
		checkBox->SetCheck(BST_CHECKED);
		checkBox->ModifyStyleEx(0, WS_EX_TRANSPARENT);
	}

	m_btnApply.Create(_T("应用选中的设置"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BTN_APPLY_SETTINGS);
	m_btnReboot.Create(_T("重启系统"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BTN_REBOOT);
	m_btnToggleSelect.Create(_T("取消全选"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BTN_TOGGLE_SELECT);
	m_btnApply.SetFont(&m_settingsFont);
	m_btnReboot.SetFont(&m_settingsFont);
	m_btnToggleSelect.SetFont(&m_settingsFont);

	m_statusText.Create(_T("就绪"), WS_CHILD | SS_LEFT, CRect(0, 0, 10, 10), this, IDC_STATUS_TEXT);
	m_adminHintText.Create(_T("* 请以管理员身份运行本程序，否则部分设置可能无效"), WS_CHILD | SS_LEFT, CRect(0, 0, 10, 10), this, IDC_ADMIN_HINT_TEXT);
	m_statusText.SetFont(&m_subtitleFont);
	m_adminHintText.SetFont(&m_subtitleFont);

	if (!IsRunningAsAdmin())
	{
		SetStatusText(_T("警告：未以管理员身份运行，部分功能可能失败。"), RGB(180, 50, 50));
	}

	UpdateToggleSelectButton();
	UpdateSettingsControlLayout();
}

void CMFCApplication1Dlg::UpdateSettingsControlLayout()
{
	if (!::IsWindow(m_chkUAC.GetSafeHwnd()))
	{
		return;
	}

	CClientDC dc(this);
	TEXTMETRIC tmLabel = {};
	TEXTMETRIC tmSettings = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&tmLabel);
	dc.SelectObject(&m_settingsFont);
	dc.GetTextMetrics(&tmSettings);

	const int titleHeight = max(24, static_cast<int>(tmLabel.tmHeight + tmLabel.tmExternalLeading + 6));
	const int itemHeight = max(22, static_cast<int>(tmSettings.tmHeight + tmSettings.tmExternalLeading + 6));
	const int itemGap = 4;
	const int titleSpace = titleHeight + 8;
	const int groupBottomPadding = 10;
	const int groupHeightLarge = titleSpace + itemHeight * 3 + itemGap * 2 + groupBottomPadding;
	const int groupHeightSmall = titleSpace + itemHeight + groupBottomPadding;
	const int optionInnerMargin = 12;
	const int groupGapY = 10;

	const int top = m_contentRect.top + 10;
	const int contentTop = top - m_scrollPos;
	const int headerHeight = 84;
	const int buttonHeight = 52;
	const int statusHeight = max(64, static_cast<int>(tmSettings.tmHeight + tmSettings.tmExternalLeading) * 2 + 22);
	const int gap1 = 10;
	const int gap2 = 10;
	const int gap3 = 8;

	const CRect headerRect(m_contentRect.left + 12, contentTop, m_contentRect.right - 12, contentTop + headerHeight);
	const int optionsWidth = headerRect.Width() - optionInnerMargin * 2;
	const bool singleColumn = optionsWidth < 560;
	const int optionsHeight = singleColumn
		? optionInnerMargin * 2 + groupHeightLarge + groupGapY + groupHeightLarge + groupGapY + groupHeightSmall + groupGapY + groupHeightSmall
		: optionInnerMargin * 2 + groupHeightLarge + groupGapY + groupHeightSmall;
	const CRect optionsCardRect(m_contentRect.left + 12, headerRect.bottom + gap1, m_contentRect.right - 12, headerRect.bottom + gap1 + optionsHeight);
	const CRect buttonCardRect(m_contentRect.left + 12, optionsCardRect.bottom + gap2, m_contentRect.right - 12, optionsCardRect.bottom + gap2 + buttonHeight);
	const CRect statusCardRect(m_contentRect.left + 12, buttonCardRect.bottom + gap3, m_contentRect.right - 12, buttonCardRect.bottom + gap3 + statusHeight);

	const int optionTop = optionsCardRect.top + optionInnerMargin;
	const int optionWidth = optionsCardRect.Width() - optionInnerMargin * 2;
	const int colGap = 14;
	const int colWidth = singleColumn ? optionWidth : (optionWidth - colGap) / 2;

	const CRect grpSecurity(optionsCardRect.left + optionInnerMargin, optionTop, optionsCardRect.left + optionInnerMargin + colWidth, optionTop + groupHeightLarge);
	const CRect grpBehavior = singleColumn
		? CRect(grpSecurity.left, grpSecurity.bottom + groupGapY, grpSecurity.right, grpSecurity.bottom + groupGapY + groupHeightLarge)
		: CRect(grpSecurity.right + colGap, optionTop, grpSecurity.right + colGap + colWidth, optionTop + groupHeightLarge);
	const CRect grpPower = singleColumn
		? CRect(grpBehavior.left, grpBehavior.bottom + groupGapY, grpBehavior.right, grpBehavior.bottom + groupGapY + groupHeightSmall)
		: CRect(grpSecurity.left, grpSecurity.bottom + groupGapY, grpSecurity.right, grpSecurity.bottom + groupGapY + groupHeightSmall);
	const CRect grpUpdate = singleColumn
		? CRect(grpPower.left, grpPower.bottom + groupGapY, grpPower.right, grpPower.bottom + groupGapY + groupHeightSmall)
		: CRect(grpBehavior.left, grpBehavior.bottom + groupGapY, grpBehavior.right, grpBehavior.bottom + groupGapY + groupHeightSmall);

	const int groupPadding = 10;

	m_chkUAC.MoveWindow(grpSecurity.left + groupPadding, grpSecurity.top + titleSpace, grpSecurity.Width() - groupPadding * 2, itemHeight, TRUE);
	m_chkFirewall.MoveWindow(grpSecurity.left + groupPadding, grpSecurity.top + titleSpace + itemHeight + itemGap, grpSecurity.Width() - groupPadding * 2, itemHeight, TRUE);
	m_chkSecCenter.MoveWindow(grpSecurity.left + groupPadding, grpSecurity.top + titleSpace + (itemHeight + itemGap) * 2, grpSecurity.Width() - groupPadding * 2, itemHeight, TRUE);

	m_chkAutoReboot.MoveWindow(grpBehavior.left + groupPadding, grpBehavior.top + titleSpace, grpBehavior.Width() - groupPadding * 2, itemHeight, TRUE);
	m_chkCrashDump.MoveWindow(grpBehavior.left + groupPadding, grpBehavior.top + titleSpace + itemHeight + itemGap, grpBehavior.Width() - groupPadding * 2, itemHeight, TRUE);
	m_chkScreenSaver.MoveWindow(grpBehavior.left + groupPadding, grpBehavior.top + titleSpace + (itemHeight + itemGap) * 2, grpBehavior.Width() - groupPadding * 2, itemHeight, TRUE);

	m_chkPower.MoveWindow(grpPower.left + groupPadding, grpPower.top + titleSpace, grpPower.Width() - groupPadding * 2, itemHeight, TRUE);
	m_chkWindowsUpdate.MoveWindow(grpUpdate.left + groupPadding, grpUpdate.top + titleSpace, grpUpdate.Width() - groupPadding * 2, itemHeight, TRUE);

	const int buttonLeft = buttonCardRect.left + 10;
	const int buttonWidth = max(88, (buttonCardRect.Width() - 28) / 3);
	const int buttonTop = buttonCardRect.top + 8;
	m_btnApply.MoveWindow(buttonLeft, buttonTop, buttonWidth, 36, TRUE);
	m_btnReboot.MoveWindow(buttonLeft + buttonWidth + 4, buttonTop, buttonWidth, 36, TRUE);
	m_btnToggleSelect.MoveWindow(buttonLeft + (buttonWidth + 4) * 2, buttonTop, buttonWidth, 36, TRUE);

	const int statusLineHeight = max(20, static_cast<int>(tmSettings.tmHeight + tmSettings.tmExternalLeading + 2));
	m_statusText.MoveWindow(statusCardRect.left + 10, statusCardRect.top + 6, statusCardRect.Width() - 20, statusLineHeight, TRUE);
	m_adminHintText.MoveWindow(statusCardRect.left + 10, statusCardRect.top + 8 + statusLineHeight, statusCardRect.Width() - 20, statusLineHeight, TRUE);
}

void CMFCApplication1Dlg::UpdatePageVisibility()
{
	const bool showSettings = (m_activePage == PAGE_SYSTEM_SETTINGS);
	const int settingsCmd = showSettings ? SW_SHOW : SW_HIDE;
	for (CButton* checkBox : GetOptionCheckBoxes())
	{
		if (::IsWindow(checkBox->GetSafeHwnd()))
		{
			checkBox->ShowWindow(settingsCmd);
		}
	}

	if (::IsWindow(m_btnApply.GetSafeHwnd()))
	{
		m_btnApply.ShowWindow(settingsCmd);
		m_btnReboot.ShowWindow(settingsCmd);
		m_btnToggleSelect.ShowWindow(settingsCmd);
		m_statusText.ShowWindow(settingsCmd);
		m_adminHintText.ShowWindow(settingsCmd);
	}

	if (showSettings)
	{
		m_scrollPos = 0;
		UpdateSettingsControlLayout();
	}
}

void CMFCApplication1Dlg::SetStatusText(const CString& text, COLORREF color)
{
	UNREFERENCED_PARAMETER(color);
	if (::IsWindow(m_statusText.GetSafeHwnd()))
	{
		m_statusText.SetWindowText(text);
	}
}

HBRUSH CMFCApplication1Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_activePage != PAGE_SYSTEM_SETTINGS || pWnd == nullptr)
	{
		return hbr;
	}

	const UINT id = static_cast<UINT>(pWnd->GetDlgCtrlID());
	const bool isSettingsCtrl =
		(id >= IDC_CHK_UAC && id <= IDC_CHK_WINDOWS_UPDATE) ||
		id == IDC_STATUS_TEXT ||
		id == IDC_ADMIN_HINT_TEXT;

	if (!isSettingsCtrl)
	{
		return hbr;
	}

	pDC->SetBkMode(OPAQUE);
	pDC->SetBkColor(RGB(240, 244, 249));
	pDC->SetTextColor(id == IDC_ADMIN_HINT_TEXT ? RGB(180, 106, 78) : RGB(47, 58, 68));
	return static_cast<HBRUSH>(m_uiBackgroundBrush.GetSafeHandle());
}

std::vector<CButton*> CMFCApplication1Dlg::GetOptionCheckBoxes()
{
	return {
		&m_chkUAC, &m_chkFirewall, &m_chkSecCenter, &m_chkAutoReboot,
		&m_chkCrashDump, &m_chkScreenSaver, &m_chkPower, &m_chkWindowsUpdate
	};
}

bool CMFCApplication1Dlg::AreAllOptionsSelected() const
{
	const CButton* options[] = {
		&m_chkUAC, &m_chkFirewall, &m_chkSecCenter, &m_chkAutoReboot,
		&m_chkCrashDump, &m_chkScreenSaver, &m_chkPower, &m_chkWindowsUpdate
	};

	for (const CButton* option : options)
	{
		if (!::IsWindow(option->GetSafeHwnd()) || option->GetCheck() != BST_CHECKED)
		{
			return false;
		}
	}
	return true;
}

void CMFCApplication1Dlg::SetAllOptions(bool isChecked)
{
	for (CButton* option : GetOptionCheckBoxes())
	{
		option->SetCheck(isChecked ? BST_CHECKED : BST_UNCHECKED);
	}
}

void CMFCApplication1Dlg::UpdateToggleSelectButton()
{
	if (::IsWindow(m_btnToggleSelect.GetSafeHwnd()))
	{
		m_btnToggleSelect.SetWindowText(AreAllOptionsSelected() ? _T("取消全选") : _T("全选"));
	}
}

bool CMFCApplication1Dlg::IsRunningAsAdmin() const
{
	BOOL isMember = FALSE;
	PSID adminGroup = nullptr;
	SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
	if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
	{
		CheckTokenMembership(nullptr, adminGroup, &isMember);
		FreeSid(adminGroup);
	}
	return isMember == TRUE;
}

void CMFCApplication1Dlg::OnSettingsOptionChanged(UINT nID)
{
	UNREFERENCED_PARAMETER(nID);
	UpdateToggleSelectButton();
}

void CMFCApplication1Dlg::OnBnClickedToggleSelect()
{
	const bool selectAll = !AreAllOptionsSelected();
	SetAllOptions(selectAll);
	UpdateToggleSelectButton();
	SetStatusText(selectAll ? _T("已全选所有设置项。") : _T("已取消所有选择。"), RGB(60, 80, 110));
}

void CMFCApplication1Dlg::OnBnClickedApplySettings()
{
	bool hasSelection = false;
	for (CButton* checkBox : GetOptionCheckBoxes())
	{
		if (checkBox->GetCheck() == BST_CHECKED)
		{
			hasSelection = true;
			break;
		}
	}

	if (!hasSelection)
	{
		SetStatusText(_T("请先选择要应用的设置项。"), RGB(160, 120, 30));
		return;
	}

	SetStatusText(_T("正在应用设置..."), RGB(60, 80, 110));
	bool allSuccess = true;
	if (m_chkUAC.GetCheck() == BST_CHECKED) allSuccess &= ApplyUAC(true);
	if (m_chkFirewall.GetCheck() == BST_CHECKED) allSuccess &= ApplyFirewall(true);
	if (m_chkSecCenter.GetCheck() == BST_CHECKED) allSuccess &= ApplySecurityCenter(true);
	if (m_chkAutoReboot.GetCheck() == BST_CHECKED) allSuccess &= ApplyAutoReboot(true);
	if (m_chkCrashDump.GetCheck() == BST_CHECKED) allSuccess &= ApplyCrashDump(true);
	if (m_chkScreenSaver.GetCheck() == BST_CHECKED) allSuccess &= ApplyScreenSaver(true);
	if (m_chkPower.GetCheck() == BST_CHECKED) allSuccess &= ApplyPowerSettings(true);
	if (m_chkWindowsUpdate.GetCheck() == BST_CHECKED) allSuccess &= ApplyWindowsUpdate(true);

	SetStatusText(allSuccess ? _T("所有设置已应用成功。") : _T("部分设置应用失败，请检查管理员权限。"),
		allSuccess ? RGB(30, 120, 40) : RGB(180, 50, 50));
}

void CMFCApplication1Dlg::OnBnClickedRebootSystem()
{
	if (AfxMessageBox(_T("系统即将重启，确认继续？"), MB_ICONWARNING | MB_YESNO) == IDYES)
	{
		if (RunProcessAndWait(_T("shutdown.exe"), _T("-r -t 5 -c \"系统将在5秒后重启\""), nullptr))
		{
			SetStatusText(_T("已发出重启指令。"), RGB(60, 80, 110));
		}
		else
		{
			SetStatusText(_T("重启指令执行失败。"), RGB(180, 50, 50));
		}
	}
}

bool CMFCApplication1Dlg::RunProcessAndWait(const CString& fileName, const CString& args, const CString* workingDirectory)
{
	CString commandLine;
	commandLine.Format(_T("\"%s\" %s"), static_cast<LPCTSTR>(fileName), static_cast<LPCTSTR>(args));

	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	CString mutableCmd(commandLine);
	BOOL created = CreateProcess(
		nullptr,
		mutableCmd.GetBuffer(),
		nullptr,
		nullptr,
		FALSE,
		CREATE_NO_WINDOW,
		nullptr,
		workingDirectory ? static_cast<LPCTSTR>(*workingDirectory) : nullptr,
		&si,
		&pi);
	mutableCmd.ReleaseBuffer();

	if (!created)
	{
		return false;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = 1;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return exitCode == 0;
}

bool CMFCApplication1Dlg::ApplyUAC(bool disable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	return key.SetDWORDValue(_T("EnableLUA"), disable ? 0 : 1) == ERROR_SUCCESS;
}

bool CMFCApplication1Dlg::ApplyFirewall(bool disable)
{
	const CString state = disable ? _T("off") : _T("on");
	return RunProcessAndWait(_T("netsh.exe"), _T("advfirewall set privateprofile state ") + state, nullptr) &&
		RunProcessAndWait(_T("netsh.exe"), _T("advfirewall set publicprofile state ") + state, nullptr);
}

bool CMFCApplication1Dlg::ApplySecurityCenter(bool disable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Security Center"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	const DWORD val = disable ? 1 : 0;
	return key.SetDWORDValue(_T("AntiVirusDisableNotify"), val) == ERROR_SUCCESS &&
		key.SetDWORDValue(_T("FirewallDisableNotify"), val) == ERROR_SUCCESS &&
		key.SetDWORDValue(_T("UpdatesDisableNotify"), val) == ERROR_SUCCESS;
}

bool CMFCApplication1Dlg::ApplyAutoReboot(bool disable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	return key.SetDWORDValue(_T("AutoReboot"), disable ? 0 : 1) == ERROR_SUCCESS;
}

bool CMFCApplication1Dlg::ApplyCrashDump(bool enable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	return key.SetDWORDValue(_T("CrashDumpEnabled"), enable ? 1 : 0) == ERROR_SUCCESS;
}

bool CMFCApplication1Dlg::ApplyScreenSaver(bool disable)
{
	CRegKey key;
	if (key.Create(HKEY_CURRENT_USER, _T("Software\\Policies\\Microsoft\\Windows\\Control Panel\\Desktop")) == ERROR_SUCCESS)
	{
		key.SetDWORDValue(_T("ScreenSaveActive"), disable ? 0 : 1);
	}

	CRegKey userKey;
	if (userKey.Open(HKEY_CURRENT_USER, _T("Control Panel\\Desktop"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}

	const CString active = disable ? _T("0") : _T("1");
	if (userKey.SetStringValue(_T("ScreenSaveActive"), active) != ERROR_SUCCESS)
	{
		return false;
	}
	if (disable)
	{
		userKey.DeleteValue(_T("SCRNSAVE.EXE"));
	}
	return true;
}

bool CMFCApplication1Dlg::ApplyPowerSettings(bool setNever)
{
	const CString timeout = setNever ? _T("0") : _T("20");
	return RunProcessAndWait(_T("powercfg.exe"), _T("/change monitor-timeout-ac ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change monitor-timeout-dc ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change disk-timeout-ac ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change disk-timeout-dc ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change standby-timeout-ac ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change standby-timeout-dc ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change hibernate-timeout-ac ") + timeout, nullptr) &&
		RunProcessAndWait(_T("powercfg.exe"), _T("/change hibernate-timeout-dc ") + timeout, nullptr);
}

bool CMFCApplication1Dlg::ApplyWindowsUpdate(bool disable)
{
	if (!disable)
	{
		return true;
	}

	CString exePath = ExtractResourceToTempFile(IDR_WUB_X64, _T("Wub_x64.exe"));
	if (exePath.IsEmpty())
	{
		return false;
	}

	CString iniPath = CString(exePath);
	const int slash = iniPath.ReverseFind(_T('\\'));
	const CString dirPath = (slash >= 0) ? iniPath.Left(slash + 1) : _T("");
	const CString exeName = (slash >= 0) ? iniPath.Mid(slash + 1) : iniPath;
	CString iniName;
	if (exeName.GetLength() > 8 && exeName.Right(8).CompareNoCase(_T("_x64.exe")) == 0)
	{
		iniName = exeName.Left(exeName.GetLength() - 8) + _T(".ini");
	}
	else
	{
		const int dot = exeName.ReverseFind(_T('.'));
		iniName = (dot > 0) ? (exeName.Left(dot) + _T(".ini")) : _T("Wub.ini");
	}
	iniPath = dirPath + iniName;

	bool ok = ExtractResourceToPath(IDR_WUB_INI, iniPath);
	if (ok)
	{
		const int pos = exePath.ReverseFind(_T('\\'));
		CString workDir = pos >= 0 ? exePath.Left(pos) : _T("");
		ok = RunProcessAndWait(exePath, _T("/D"), &workDir);
	}

	DeleteTempFile(iniPath);
	DeleteTempFile(exePath);
	return ok;
}

CString CMFCApplication1Dlg::ExtractResourceToTempFile(UINT resourceId, const CString& fileName)
{
	TCHAR tempPath[MAX_PATH] = {};
	if (GetTempPath(MAX_PATH, tempPath) == 0)
	{
		return _T("");
	}

	TCHAR tempFile[MAX_PATH] = {};
	if (GetTempFileName(tempPath, _T("cc"), 0, tempFile) == 0)
	{
		return _T("");
	}

	CString outputPath = tempFile;
	outputPath += _T("_");
	outputPath += fileName;

	return ExtractResourceToPath(resourceId, outputPath) ? outputPath : _T("");
}

bool CMFCApplication1Dlg::ExtractResourceToPath(UINT resourceId, const CString& outputPath)
{
	HRSRC hResource = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if (hResource == nullptr)
	{
		return false;
	}

	HGLOBAL hLoaded = LoadResource(AfxGetResourceHandle(), hResource);
	if (hLoaded == nullptr)
	{
		return false;
	}

	const DWORD size = SizeofResource(AfxGetResourceHandle(), hResource);
	const void* data = LockResource(hLoaded);
	if (data == nullptr || size == 0)
	{
		return false;
	}

	CFile file;
	if (!file.Open(outputPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
	{
		return false;
	}

	file.Write(data, size);
	file.Close();
	return true;
}

void CMFCApplication1Dlg::DeleteTempFile(const CString& filePath)
{
	if (!filePath.IsEmpty())
	{
		DeleteFile(filePath);
	}
}
