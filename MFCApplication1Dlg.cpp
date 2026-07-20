// MFCApplication1Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "AtaSmartCore\CompileOptions.h"
#include "AtaSmartCore\AtaSmart.h"
#include "AtaSmartCore\Priscilla\UtilityFx.h"
#include "AtaSmartCore\SlotSpeedGetter.h"

#include <Wbemidl.h>
#include <comdef.h>
#include <atlbase.h>
#include <atlstr.h>
#include <winioctl.h>
#include <Shlwapi.h>
#include <shlobj.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <ShlObj.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <intrin.h>
#include <cwctype>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	// 自定义消息：在窗口初始化后异步加载系统信息，避免阻塞首屏渲染。
	constexpr UINT WM_APP_LOAD_SYSTEM_INFO = WM_APP + 100;
	constexpr UINT WM_APP_LOAD_SSD_INFO = WM_APP + 101;
	constexpr UINT WM_APP_LOAD_SCREEN_INFO = WM_APP + 102;
	constexpr UINT WM_APP_APPLY_SYSTEM_INFO = WM_APP + 103;
	constexpr UINT WM_APP_APPLY_SSD_INFO = WM_APP + 104;
	constexpr UINT WM_APP_APPLY_SCREEN_INFO = WM_APP + 105;
	constexpr UINT WM_APP_LOAD_SYSTEM_EXCEPTION_INFO = WM_APP + 106;
	constexpr UINT WM_APP_APPLY_SYSTEM_EXCEPTION_INFO = WM_APP + 107;
	constexpr UINT WM_APP_LOAD_BATTERY_LOG_INFO = WM_APP + 108;
	constexpr UINT WM_APP_APPLY_BATTERY_LOG_INFO = WM_APP + 109;
	constexpr UINT WM_APP_LOAD_POWER_LOG_INFO = WM_APP + 110;
	constexpr UINT WM_APP_APPLY_POWER_LOG_INFO = WM_APP + 111;
	constexpr UINT WM_APP_LOAD_DRIVER_DETAILS = WM_APP + 112;
	constexpr UINT WM_APP_APPLY_DRIVER_DETAILS = WM_APP + 113;
	constexpr int PAGE_SYSTEM_INFO = 0;
	constexpr int PAGE_SYSTEM_SETTINGS = 1;
	constexpr int PAGE_SSD_INFO = 2;
	constexpr int PAGE_SCREEN_INFO = 3;
	constexpr int PAGE_SYSTEM_STATUS = 4;
	constexpr int PAGE_STARTUP_ITEMS = 5;
	constexpr int PAGE_ACPI_INFO = 6;
	constexpr int PAGE_BATTERY_LOG = 7;
	constexpr int PAGE_POWER_LOG = 8;
	constexpr int PAGE_SYSTEM_EXCEPTION = 9;
	constexpr int PAGE_UTILITY_TOOLS = 10;
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
	constexpr UINT IDC_SSD_TAB = 3015;
	constexpr UINT IDC_DRIVER_TREE = 3016;
	constexpr UINT IDC_STARTUP_LIST = 3020;
	constexpr UINT IDC_STARTUP_ENABLE = 3021;
	constexpr UINT IDC_STARTUP_DISABLE = 3022;
	constexpr UINT IDC_STARTUP_REFRESH = 3023;
	constexpr UINT IDC_STARTUP_NAME = 3024;
	constexpr UINT IDC_STARTUP_PATH = 3025;
	constexpr UINT IDC_STARTUP_BROWSE = 3026;
	constexpr UINT IDC_STARTUP_ADD = 3027;
	constexpr UINT IDC_STARTUP_STATUS = 3028;
	constexpr UINT IDC_STARTUP_DELETE = 3029;
	constexpr UINT IDC_STARTUP_NAME_LABEL = 3030;
	constexpr UINT IDC_STARTUP_PATH_LABEL = 3031;
	constexpr UINT ID_STARTUP_MENU_ENABLE = 3040;
	constexpr UINT ID_STARTUP_MENU_DISABLE = 3041;
	constexpr UINT ID_STARTUP_MENU_DELETE = 3042;
	constexpr UINT ID_STARTUP_MENU_REFRESH = 3043;
	constexpr UINT IDC_BATTERY_LOG_DETAILS = 3050;
	constexpr UINT IDC_POWER_LOG_DETAILS = 3051;
	constexpr UINT IDC_BATTERY_LOG_REFRESH = 3052;
	constexpr UINT IDC_POWER_LOG_REFRESH = 3053;
	constexpr UINT IDC_OPTIONS_START = IDC_CHK_UAC;
	constexpr UINT IDC_OPTIONS_END = IDC_CHK_WINDOWS_UPDATE;
	const COLORREF UiBackground = RGB(243, 243, 243);
	const COLORREF UiSurface = RGB(255, 255, 255);
	const COLORREF UiSurfaceAlt = RGB(249, 249, 249);
	const COLORREF UiSubtleSurface = RGB(247, 249, 252);
	const COLORREF UiBorder = RGB(229, 229, 229);
	const COLORREF UiText = RGB(32, 32, 32);
	const COLORREF UiSecondaryText = RGB(96, 96, 96);
	const COLORREF UiTertiaryText = RGB(115, 115, 115);
	const COLORREF UiAccent = RGB(0, 95, 184);
	const COLORREF UiAccentSoft = RGB(232, 241, 251);
	const COLORREF UiWarningText = RGB(156, 86, 38);

	bool EnsureComInitialized();
	CString VariantToCString(VARIANT& var);
	CString JoinUniqueValues(const std::set<CString, std::less<>>& values, const CString& separator);
	CString FormatWmiDateTime(const CString& wmiDateTime);
	int ExtractPhysicalDriveId(const CString& devicePath);
	std::vector<std::vector<CString>> QueryWmiRows(
		const CString& wmiNamespace,
		const CString& wql,
		const std::vector<CString>& propertyNames);
	CString GetPowerCfgReportPath(const CString& fileName);
	CString FormatFileSize(ULONGLONG bytes);
	CString FormatFileTime(const CTime& time);
	CString FormatWmiQueryTimeDaysAgo(int days);
	CString ConfigManagerErrorText(const CString& code);
	CString BuildSystemExceptionEventText(const std::vector<CString>& row);

	void EnableWin11Chrome(HWND hwnd)
	{
		const DWORD cornerPreference = DWMWCP_ROUND;
		DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));

		const DWORD backdropType = DWMSBT_MAINWINDOW;
		DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
	}

	void CenterWindowAtTwoThirdsOfScreen(CWnd& window)
	{
		HMONITOR monitor = MonitorFromWindow(window.GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo = {};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (!GetMonitorInfo(monitor, &monitorInfo))
		{
			window.CenterWindow();
			return;
		}

		const CRect workArea(monitorInfo.rcWork);
		const int windowWidth = max(640, workArea.Width() * 2 / 3);
		const int windowHeight = max(480, workArea.Height() * 2 / 3);
		const int left = workArea.left + (workArea.Width() - windowWidth) / 2;
		const int top = workArea.top + (workArea.Height() - windowHeight) / 2;
		window.SetWindowPos(nullptr, left, top, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOACTIVATE);
	}

	void DrawAccentStrip(CDC& dc, const CRect& rect, COLORREF color)
	{
		CRect strip(rect.left + 10, rect.top + 13, rect.left + 14, rect.bottom - 13);
		CBrush brush(color);
		CPen pen(PS_SOLID, 1, color);
		CBrush* oldBrush = dc.SelectObject(&brush);
		CPen* oldPen = dc.SelectObject(&pen);
		dc.RoundRect(strip, CPoint(4, 4));
		dc.SelectObject(oldPen);
		dc.SelectObject(oldBrush);
	}

	// 去除字符串首尾空白字符，返回规整后的结果。
	CString Trimmed(const CString& value)
	{
		CString result(value);
		result.Trim();
		return result;
	}

	// 判断字符串是否为有效值（非空且不为 N/A）。
	bool HasValue(const CString& value)
	{
		const CString trimmed = Trimmed(value);
		return !trimmed.IsEmpty() && trimmed.CompareNoCase(_T("N/A")) != 0;
	}

	bool IsPlaceholderValue(const CString& value)
	{
		const CString trimmed = Trimmed(value);
		if (!HasValue(trimmed))
		{
			return true;
		}

		bool hasOnlyDashes = true;
		for (int i = 0; i < trimmed.GetLength(); ++i)
		{
			if (trimmed[i] != _T('-'))
			{
				hasOnlyDashes = false;
				break;
			}
		}
		return hasOnlyDashes;
	}

	bool HasDisplayValue(const CString& value)
	{
		return !IsPlaceholderValue(value);
	}

	// 将字符串转换为小写，便于做不区分大小写的关键字匹配。
	CString ToLower(const CString& value)
	{
		CString lowered(value);
		lowered.MakeLower();
		return lowered;
	}

	// 将分隔符格式化为多行文本，便于在界面中按行展示。
	CString NormalizeMultilineValue(const CString& value)
	{
		CString normalized(value);
		normalized.Replace(_T(" / "), _T("\r\n"));
		normalized.Replace(_T(" | "), _T("\r\n"));
		return normalized;
	}

	CString GetPowerCfgReportPath(const CString& fileName)
	{
		TCHAR modulePath[MAX_PATH] = {};
		const DWORD length = GetModuleFileName(nullptr, modulePath, _countof(modulePath));
		CString directory = (length > 0 && length < _countof(modulePath)) ? CString(modulePath) : CString(_T("."));
		const int slash = directory.ReverseFind(_T('\\'));
		if (slash >= 0)
		{
			directory = directory.Left(slash + 1);
		}
		else if (!directory.IsEmpty() && directory.Right(1) != _T("\\"))
		{
			directory += _T("\\");
		}
		return directory + fileName;
	}

	CString FormatFileSize(ULONGLONG bytes)
	{
		CString result;
		if (bytes >= 1024ULL * 1024ULL)
		{
			result.Format(_T("%.2f MB"), static_cast<double>(bytes) / (1024.0 * 1024.0));
		}
		else if (bytes >= 1024ULL)
		{
			result.Format(_T("%.1f KB"), static_cast<double>(bytes) / 1024.0);
		}
		else
		{
			result.Format(_T("%llu B"), bytes);
		}
		return result;
	}

	CString FormatFileTime(const CTime& time)
	{
		return time.Format(_T("%Y-%m-%d %H:%M:%S"));
	}

	CString FormatWmiQueryTimeDaysAgo(int days)
	{
		const CTime since = CTime::GetCurrentTime() - CTimeSpan(max(days, 1), 0, 0, 0);
		TIME_ZONE_INFORMATION timeZone = {};
		DWORD zoneId = GetTimeZoneInformation(&timeZone);
		LONG bias = timeZone.Bias;
		if (zoneId == TIME_ZONE_ID_DAYLIGHT)
		{
			bias += timeZone.DaylightBias;
		}
		else if (zoneId == TIME_ZONE_ID_STANDARD)
		{
			bias += timeZone.StandardBias;
		}

		const LONG offsetMinutes = -bias;
		CString text;
		text.Format(_T("%s.000000%c%03ld"),
			since.Format(_T("%Y%m%d%H%M%S")).GetString(),
			offsetMinutes >= 0 ? _T('+') : _T('-'),
			labs(offsetMinutes));
		return text;
	}

	CString ConfigManagerErrorText(const CString& code)
	{
		const int value = _ttoi(code);
		switch (value)
		{
		case 1: return _T("设备配置不正确");
		case 3: return _T("驱动程序可能已损坏或系统资源不足");
		case 10: return _T("设备无法启动");
		case 12: return _T("设备资源不足");
		case 14: return _T("需要重启计算机");
		case 18: return _T("需要重新安装驱动程序");
		case 19: return _T("注册表配置信息不完整或损坏");
		case 21: return _T("Windows 正在删除此设备");
		case 22: return _T("设备已被禁用");
		case 24: return _T("设备不存在、工作异常或驱动未安装");
		case 28: return _T("未安装设备驱动程序");
		case 31: return _T("设备驱动无法正常工作");
		case 32: return _T("驱动服务已被禁用");
		case 37: return _T("Windows 无法初始化设备驱动程序");
		case 39: return _T("Windows 无法加载设备驱动程序");
		case 43: return _T("设备报告问题，Windows 已停止该设备");
		case 45: return _T("设备当前未连接到计算机");
		case 48: return _T("驱动因存在问题已被阻止启动");
		case 52: return _T("无法验证驱动程序的数字签名");
		default:
		{
			CString text;
			text.Format(_T("设备管理器错误码 %d"), value);
			return text;
		}
		}
	}

	CString BuildSystemExceptionEventText(const std::vector<CString>& row)
	{
		CString timeText = row.size() > 0 ? FormatWmiDateTime(row[0]) : _T("");
		CString source = row.size() > 2 ? row[2] : _T("");
		CString type = row.size() > 3 ? row[3] : _T("");
		CString message = row.size() > 4 ? row[4] : _T("");
		CString recordNumber = row.size() > 5 ? row[5] : _T("");
		CString category = row.size() > 6 ? row[6] : _T("");
		CString user = row.size() > 7 ? row[7] : _T("");
		CString computer = row.size() > 8 ? row[8] : _T("");

		CString text;
		if (HasValue(timeText)) text += _T("时间: ") + timeText + _T("\r\n");
		if (HasValue(source)) text += _T("来源: ") + source + _T("\r\n");
		if (HasValue(type)) text += _T("类型: ") + type + _T("\r\n");
		if (HasValue(recordNumber)) text += _T("记录号: ") + recordNumber + _T("\r\n");
		if (HasValue(category)) text += _T("类别: ") + category + _T("\r\n");
		if (HasValue(user)) text += _T("用户: ") + user + _T("\r\n");
		if (HasValue(computer)) text += _T("计算机: ") + computer + _T("\r\n");
		if (HasValue(message))
		{
			text += _T("详细信息:\r\n");
			text += message;
		}
		text.TrimRight();
		return text;
	}

	// 判断是否为“通用显示器”这类无辨识度名称。
	bool IsGenericMonitorLabel(const CString& value)
	{
		const CString lowered = ToLower(Trimmed(value));
		return lowered.IsEmpty() ||
			lowered == _T("generic pnp monitor") ||
			lowered == _T("generic monitor") ||
			lowered == _T("default monitor") ||
			lowered == _T("通用即插即用监视器") ||
			lowered == _T("默认监视器");
	}

	CString NormalizeMonitorLabel(const CString& value)
	{
		CString label = Trimmed(value);
		if (!HasValue(label))
		{
			return _T("");
		}

		const CString lowered = ToLower(label);
		const int leftParen = label.Find(_T('('));
		const int rightParen = label.ReverseFind(_T(')'));
		const bool hasWrappedModel = leftParen >= 0 && rightParen > leftParen;
		if ((lowered.Find(_T("generic monitor")) == 0 ||
			lowered.Find(_T("generic pnp monitor")) == 0 ||
			lowered.Find(_T("通用")) == 0) &&
			hasWrappedModel)
		{
			const CString inner = Trimmed(label.Mid(leftParen + 1, rightParen - leftParen - 1));
			if (HasValue(inner) && !IsGenericMonitorLabel(inner))
			{
				return inner;
			}
		}

		return IsGenericMonitorLabel(label) ? _T("") : label;
	}

	bool IsLikelyMonitorIdToken(const CString& value)
	{
		const CString token = Trimmed(value);
		if (token.GetLength() < 6 || token.GetLength() > 12)
		{
			return false;
		}
		if (!(token[0] >= L'A' && token[0] <= L'Z') ||
			!(token[1] >= L'A' && token[1] <= L'Z') ||
			!(token[2] >= L'A' && token[2] <= L'Z'))
		{
			return false;
		}
		bool hasDigit = false;
		for (int i = 3; i < token.GetLength(); ++i)
		{
			const wchar_t ch = token[i];
			if (ch >= L'0' && ch <= L'9')
			{
				hasDigit = true;
			}
			else if (!iswalnum(ch))
			{
				return false;
			}
		}
		return hasDigit;
	}

	struct EdidInfo
	{
		CString monitorId;
		CString edidDate;
		CString edidVersion;
		CString serialNumber;
		CString refreshRate;
		CString monitorName;
		CString checksum;
		CString fullEdidHex;
	};

	CString DecodeEdidManufacturerId(const BYTE* edid, size_t size)
	{
		if (edid == nullptr || size < 10)
		{
			return _T("");
		}

		const WORD code = static_cast<WORD>((edid[8] << 8) | edid[9]);
		CString id;
		for (int shift = 10; shift >= 0; shift -= 5)
		{
			const int value = (code >> shift) & 0x1F;
			if (value <= 0 || value > 26)
			{
				return _T("");
			}
			id.AppendChar(static_cast<TCHAR>(_T('A') + value - 1));
		}
		return id;
	}

	CString DecodeEdidDescriptorText(const BYTE* descriptor)
	{
		CString text;
		if (descriptor == nullptr)
		{
			return text;
		}

		for (int i = 5; i < 18; ++i)
		{
			const BYTE ch = descriptor[i];
			if (ch == 0x0A || ch == 0x00)
			{
				break;
			}
			text.AppendChar(static_cast<TCHAR>(ch));
		}
		text.Trim();
		return text;
	}

	CString FormatEdidHex(const std::vector<BYTE>& edid)
	{
		CString text;
		for (size_t i = 0; i < edid.size(); ++i)
		{
			CString byteText;
			byteText.Format(_T("%02x"), edid[i]);
			if (i > 0)
			{
				text += (i % 16 == 0) ? _T("\r\n") : _T(" ");
			}
			text += byteText;
		}
		return text;
	}

	bool ParseEdid(const std::vector<BYTE>& edid, EdidInfo& info)
	{
		if (edid.size() < 128)
		{
			return false;
		}

		const BYTE expectedHeader[] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };
		if (memcmp(edid.data(), expectedHeader, sizeof(expectedHeader)) != 0)
		{
			return false;
		}

		info.monitorId = DecodeEdidManufacturerId(edid.data(), edid.size());
		if (HasValue(info.monitorId))
		{
			CString productCode;
			productCode.Format(_T("%02X%02X"), edid[11], edid[10]);
			info.monitorId += productCode;
		}

		const UINT serial = static_cast<UINT>(edid[12]) |
			(static_cast<UINT>(edid[13]) << 8) |
			(static_cast<UINT>(edid[14]) << 16) |
			(static_cast<UINT>(edid[15]) << 24);
		if (serial > 0)
		{
			info.serialNumber.Format(_T("%u"), serial);
		}

		info.edidDate.Format(_T("Week%u, Year%u"), edid[16], 1990 + edid[17]);
		info.edidVersion.Format(_T("%u.%u"), edid[18], edid[19]);
		info.checksum.Format(_T("0x%02X"), edid[127]);
		info.fullEdidHex = FormatEdidHex(edid);

		if (edid[54] != 0 || edid[55] != 0)
		{
			const UINT pixelClock10Khz = static_cast<UINT>(edid[54]) | (static_cast<UINT>(edid[55]) << 8);
			const UINT hActive = static_cast<UINT>(edid[56]) | ((static_cast<UINT>(edid[58]) & 0xF0) << 4);
			const UINT hBlank = static_cast<UINT>(edid[57]) | ((static_cast<UINT>(edid[58]) & 0x0F) << 8);
			const UINT vActive = static_cast<UINT>(edid[59]) | ((static_cast<UINT>(edid[61]) & 0xF0) << 4);
			const UINT vBlank = static_cast<UINT>(edid[60]) | ((static_cast<UINT>(edid[61]) & 0x0F) << 8);
			const UINT hTotal = hActive + hBlank;
			const UINT vTotal = vActive + vBlank;
			if (pixelClock10Khz > 0 && hTotal > 0 && vTotal > 0)
			{
				const double refreshRate = static_cast<double>(pixelClock10Khz) * 10000.0 / (static_cast<double>(hTotal) * static_cast<double>(vTotal));
				info.refreshRate.Format(_T("%.2f"), refreshRate);
			}
		}

		for (int offset = 54; offset <= 108; offset += 18)
		{
			if (edid[offset] != 0x00 || edid[offset + 1] != 0x00 || edid[offset + 2] != 0x00)
			{
				continue;
			}

			const BYTE descriptorType = edid[offset + 3];
			if (descriptorType == 0xFC)
			{
				const CString monitorName = DecodeEdidDescriptorText(&edid[offset]);
				if (HasValue(monitorName) && !IsLikelyMonitorIdToken(monitorName))
				{
					info.monitorName = monitorName;
				}
			}
			else if (descriptorType == 0xFF && !HasValue(info.serialNumber))
			{
				const CString serialText = DecodeEdidDescriptorText(&edid[offset]);
				if (HasValue(serialText))
				{
					info.serialNumber = serialText;
				}
			}
		}

		return true;
	}

	CString NormalizeDisplayRegistryInstancePath(const CString& instanceName)
	{
		CString path = Trimmed(instanceName);
		path.Replace(_T("\\\\"), _T("\\"));
		const int suffix = path.ReverseFind(_T('_'));
		if (suffix > 0)
		{
			path = path.Left(suffix);
		}
		return path;
	}

	bool ReadEdidFromRegistryInstancePath(const CString& instancePath, std::vector<BYTE>& edid)
	{
		edid.clear();
		CString paramsPath = _T("SYSTEM\\CurrentControlSet\\Enum\\");
		paramsPath += NormalizeDisplayRegistryInstancePath(instancePath);
		paramsPath += _T("\\Device Parameters");

		CRegKey paramsKey;
		if (paramsKey.Open(HKEY_LOCAL_MACHINE, paramsPath, KEY_READ) != ERROR_SUCCESS)
		{
			return false;
		}

		DWORD type = 0;
		DWORD byteCount = 0;
		if (RegQueryValueEx(paramsKey, _T("EDID"), nullptr, &type, nullptr, &byteCount) != ERROR_SUCCESS ||
			type != REG_BINARY ||
			byteCount < 128)
		{
			return false;
		}

		edid.assign(byteCount, 0);
		if (RegQueryValueEx(paramsKey, _T("EDID"), nullptr, &type, edid.data(), &byteCount) != ERROR_SUCCESS)
		{
			edid.clear();
			return false;
		}
		edid.resize(byteCount);
		return true;
	}

	std::vector<CString> GetActiveMonitorInstancePathsFromWmi()
	{
		std::vector<CString> instancePaths;
		const auto rows = QueryWmiRows(
			_T("ROOT\\WMI"),
			_T("SELECT InstanceName, Active FROM WmiMonitorID"),
			{ _T("InstanceName"), _T("Active") });

		for (const auto& row : rows)
		{
			if (row.size() < 2 || row[1].CompareNoCase(_T("True")) != 0)
			{
				continue;
			}
			const CString instancePath = NormalizeDisplayRegistryInstancePath(row[0]);
			if (HasValue(instancePath))
			{
				instancePaths.push_back(instancePath);
			}
		}
		return instancePaths;
	}

	std::vector<CString> GetActiveMonitorInstancePathsFromDisplayDevices()
	{
		std::vector<CString> instancePaths;
		for (DWORD adapterIndex = 0;; ++adapterIndex)
		{
			DISPLAY_DEVICE adapter = {};
			adapter.cb = sizeof(adapter);
			if (!EnumDisplayDevices(nullptr, adapterIndex, &adapter, 0))
			{
				break;
			}

			for (DWORD monitorIndex = 0;; ++monitorIndex)
			{
				DISPLAY_DEVICE monitor = {};
				monitor.cb = sizeof(monitor);
				if (!EnumDisplayDevices(adapter.DeviceName, monitorIndex, &monitor, 0))
				{
					break;
				}
				if ((monitor.StateFlags & DISPLAY_DEVICE_ACTIVE) == 0)
				{
					continue;
				}

				const CString deviceId = NormalizeDisplayRegistryInstancePath(monitor.DeviceID);
				if (deviceId.Find(_T("DISPLAY\\")) == 0)
				{
					instancePaths.push_back(deviceId);
				}
			}
		}
		return instancePaths;
	}

	std::vector<std::vector<BYTE>> ReadActiveEdidBlocksFromRegistry()
	{
		std::vector<std::vector<BYTE>> edids;
		std::vector<CString> instancePaths = GetActiveMonitorInstancePathsFromWmi();
		const std::vector<CString> displayDevicePaths = GetActiveMonitorInstancePathsFromDisplayDevices();
		instancePaths.insert(instancePaths.end(), displayDevicePaths.begin(), displayDevicePaths.end());

		std::set<CString, std::less<>> seen;
		for (const CString& instancePath : instancePaths)
		{
			if (!seen.insert(instancePath).second)
			{
				continue;
			}

			std::vector<BYTE> edid;
			if (ReadEdidFromRegistryInstancePath(instancePath, edid))
			{
				edids.push_back(edid);
			}
		}
		return edids;
	}

	CString ResolveMonitorNameFromEdid()
	{
		std::set<CString, std::less<>> monitorNames;
		for (const auto& edid : ReadActiveEdidBlocksFromRegistry())
		{
			EdidInfo info;
			if (ParseEdid(edid, info) && HasValue(info.monitorName))
			{
				monitorNames.insert(info.monitorName);
			}
		}
		return JoinUniqueValues(monitorNames, _T("\r\n"));
	}

	// 将字节数转换为 GB 文本（保留 1 位小数）。
	CString FormatBytesToGB(unsigned long long bytes)
	{
		const double sizeGb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
		CString text;
		text.Format(_T("%.1f GB"), sizeGb);
		return text;
	}

	// 将字符串安全转换为无符号 64 位整数，失败或空值返回 0。
	unsigned long long ParseUnsignedLongLong(const CString& text)
	{
		const CString trimmed = Trimmed(text);
		if (trimmed.IsEmpty())
		{
			return 0;
		}
		return _wcstoui64(trimmed, nullptr, 10);
	}

	// 拼装磁盘概要文本（型号/容量/接口），供系统信息与 SSD 信息页复用。
	CString BuildDiskSummary(const CString& model, const CString& size, const CString& interfaceType)
	{
		CString diskText = HasValue(model) ? model : _T("未知型号");
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
		return diskText;
	}

	// 基于型号/介质类型/接口类型识别 SSD（兼容 NVMe 标识）。
	bool IsLikelySsdDevice(const CString& model, const CString& mediaType, const CString& interfaceType)
	{
		const CString matcher = ToLower(model + _T(" ") + mediaType + _T(" ") + interfaceType);
		return matcher.Find(_T("ssd")) >= 0 ||
			matcher.Find(_T("solid state")) >= 0 ||
			matcher.Find(_T("nvme")) >= 0;
	}

	// 将 AtaSmart 的磁盘状态枚举映射到可读文本。
	CString AtaSmartDiskStatusText(DWORD diskStatus)
	{
		switch (diskStatus)
		{
		case CAtaSmart::DISK_STATUS_GOOD:
			return _T("良好");
		case CAtaSmart::DISK_STATUS_CAUTION:
			return _T("警告");
		case CAtaSmart::DISK_STATUS_BAD:
			return _T("故障");
		default:
			return _T("未知");
		}
	}

	CString AtaSmartHostRwText(INT value, DWORD unit)
	{
		if (value <= 0)
		{
			return _T("N/A");
		}

		double sizeGb = 0.0;
		switch (unit)
		{
		case CAtaSmart::HOST_READS_WRITES_512B:
			sizeGb = static_cast<double>(value) * 512.0 / (1024.0 * 1024.0 * 1024.0);
			break;
		case CAtaSmart::HOST_READS_WRITES_1MB:
			sizeGb = static_cast<double>(value) / 1024.0;
			break;
		case CAtaSmart::HOST_READS_WRITES_16MB:
			sizeGb = static_cast<double>(value) * 16.0 / 1024.0;
			break;
		case CAtaSmart::HOST_READS_WRITES_32MB:
			sizeGb = static_cast<double>(value) * 32.0 / 1024.0;
			break;
		case CAtaSmart::HOST_READS_WRITES_GB:
			sizeGb = static_cast<double>(value);
			break;
		default:
		{
			CString unknownUnitText;
			unknownUnitText.Format(_T("%d (单位未知)"), value);
			return unknownUnitText;
		}
		}

		if (sizeGb < 0.05)
		{
			return _T("N/A");
		}

		CString text;
		if (sizeGb >= 1.0)
		{
			text.Format(_T("%.1f GB"), sizeGb);
		}
		else
		{
			text.Format(_T("%.1f MB"), sizeGb * 1024.0);
		}
		return text;
	}

	bool IsAtaSmartSsdCandidate(const CAtaSmart::ATA_SMART_INFO& info)
	{
		return info.IsSsd == TRUE
			|| info.IsNVMe == TRUE
			|| info.DiskVendorId != CAtaSmart::HDD_GENERAL
			|| info.NominalMediaRotationRate == 1;
	}

	CString BuildAtaSmartTabTitle(const CAtaSmart::ATA_SMART_INFO& info, int index)
	{
		CString tabTitle = HasValue(info.Model) ? info.Model : _T("");
		if (tabTitle.IsEmpty())
		{
			tabTitle.Format(_T("SSD %d"), index + 1);
		}
		if (tabTitle.GetLength() > 28)
		{
			tabTitle = tabTitle.Left(28) + _T("...");
		}
		return tabTitle;
	}

	// 将 VT_UI1 数组提取为字节向量，供 SMART 原始属性解析使用。
	std::vector<unsigned char> VariantToBytes(const VARIANT& var)
	{
		std::vector<unsigned char> bytes;
		if (!((var.vt & VT_ARRAY) && ((var.vt & VT_TYPEMASK) == VT_UI1)) || var.parray == nullptr)
		{
			return bytes;
		}

		SAFEARRAY* arr = var.parray;
		LONG lower = 0;
		LONG upper = -1;
		if (FAILED(SafeArrayGetLBound(arr, 1, &lower)) || FAILED(SafeArrayGetUBound(arr, 1, &upper)) || upper < lower)
		{
			return bytes;
		}

		bytes.reserve(static_cast<size_t>(upper - lower + 1));
		for (LONG i = lower; i <= upper; ++i)
		{
			unsigned char value = 0;
			if (SUCCEEDED(SafeArrayGetElement(arr, &i, &value)))
			{
				bytes.push_back(value);
			}
		}
		return bytes;
	}

	// 以 CrystalDiskInfo 同源的 SMART 表结构解析原始属性：每项 12 字节。
	bool TryGetSmartRawValue(const std::vector<unsigned char>& vendorSpecific, unsigned char attributeId, unsigned long long& rawValue, unsigned char& currentValue)
	{
		// VendorSpecific 通常是 512 字节；属性区从偏移 2 开始，每条 12 字节，最多 30 条。
		const size_t begin = 2;
		const size_t stride = 12;
		const size_t maxCount = 30;
		if (vendorSpecific.size() < begin + stride)
		{
			return false;
		}

		for (size_t i = 0; i < maxCount; ++i)
		{
			const size_t offset = begin + i * stride;
			if (offset + stride > vendorSpecific.size())
			{
				break;
			}
			if (vendorSpecific[offset] != attributeId)
			{
				continue;
			}

			currentValue = vendorSpecific[offset + 3];
			rawValue = 0;
			for (size_t b = 0; b < 6; ++b)
			{
				rawValue |= (static_cast<unsigned long long>(vendorSpecific[offset + 5 + b]) << (8 * b));
			}
			return true;
		}
		return false;
	}

	struct SmartWmiEntry
	{
		CString instanceName;
		std::vector<unsigned char> vendorSpecific;
	};

	// 查询 ROOT\WMI 下的 SMART 原始数据，作为 CrystalDiskInfo 同路径的最小集成方案。
	std::vector<SmartWmiEntry> QuerySmartWmiEntries()
	{
		std::vector<SmartWmiEntry> entries;
		if (!EnsureComInitialized())
		{
			return entries;
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
			return entries;
		}

		IWbemServices* services = nullptr;
		hr = locator->ConnectServer(
			_bstr_t(L"ROOT\\WMI"),
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
			return entries;
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
			return entries;
		}

		IEnumWbemClassObject* enumerator = nullptr;
		hr = services->ExecQuery(
			_bstr_t(L"WQL"),
			_bstr_t(L"SELECT InstanceName, VendorSpecific FROM MSStorageDriver_FailurePredictData"),
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
			nullptr,
			&enumerator);
		if (FAILED(hr) || enumerator == nullptr)
		{
			services->Release();
			locator->Release();
			return entries;
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

			VARIANT instanceVar;
			VariantInit(&instanceVar);
			VARIANT smartVar;
			VariantInit(&smartVar);

			const HRESULT instanceHr = object->Get(_bstr_t(L"InstanceName"), 0, &instanceVar, nullptr, nullptr);
			const HRESULT smartHr = object->Get(_bstr_t(L"VendorSpecific"), 0, &smartVar, nullptr, nullptr);
			if (SUCCEEDED(instanceHr) && SUCCEEDED(smartHr))
			{
				SmartWmiEntry entry;
				entry.instanceName = Trimmed(VariantToCString(instanceVar));
				entry.vendorSpecific = VariantToBytes(smartVar);
				if (!entry.instanceName.IsEmpty() && !entry.vendorSpecific.empty())
				{
					entries.push_back(entry);
				}
			}

			VariantClear(&smartVar);
			VariantClear(&instanceVar);
			object->Release();
		}

		enumerator->Release();
		services->Release();
		locator->Release();
		return entries;
	}

	// 规整硬件标识字符串，便于跨数据源做宽松匹配。
	CString NormalizeHardwareKey(const CString& value)
	{
		CString normalized;
		for (int i = 0; i < value.GetLength(); ++i)
		{
			const wchar_t ch = value[i];
			if (iswalnum(ch))
			{
				normalized.AppendChar(static_cast<wchar_t>(towupper(ch)));
			}
		}
		return normalized;
	}

	bool IsLikelySameDisk(const CAtaSmart::ATA_SMART_INFO& info, const std::vector<CString>& diskRow)
	{
		if (diskRow.size() < 9)
		{
			return false;
		}

		const int rowPhysicalDriveId = ExtractPhysicalDriveId(diskRow[8]);
		if (info.PhysicalDriveId >= 0 && rowPhysicalDriveId == info.PhysicalDriveId)
		{
			return true;
		}

		const CString infoModel = NormalizeHardwareKey(info.Model);
		const CString rowModel = NormalizeHardwareKey(diskRow[0]);
		const CString infoSerial = NormalizeHardwareKey(info.SerialNumber);
		const CString rowSerial = NormalizeHardwareKey(diskRow[4]);
		const bool modelMatched =
			!infoModel.IsEmpty() &&
			(rowModel.Find(infoModel) >= 0 || infoModel.Find(rowModel) >= 0);
		const bool serialMatched =
			!infoSerial.IsEmpty() &&
			(rowSerial.Find(infoSerial) >= 0 || infoSerial.Find(rowSerial) >= 0);
		return serialMatched || (modelMatched && info.NumberOfSectors > 0);
	}

	CString ResolveAtaSmartDeviceId(
		const CAtaSmart::ATA_SMART_INFO& info,
		const std::vector<std::vector<CString>>& diskRows,
		const std::map<int, CString>& pnpByPhysicalDriveId)
	{
		if (HasDisplayValue(info.PnpDeviceId))
		{
			return info.PnpDeviceId;
		}

		const auto byDriveId = pnpByPhysicalDriveId.find(info.PhysicalDriveId);
		if (byDriveId != pnpByPhysicalDriveId.end() && HasDisplayValue(byDriveId->second))
		{
			return byDriveId->second;
		}

		for (const auto& diskRow : diskRows)
		{
			if (!IsLikelySameDisk(info, diskRow))
			{
				continue;
			}
			if (diskRow.size() > 7 && HasDisplayValue(diskRow[7]))
			{
				return diskRow[7];
			}
			if (diskRow.size() > 8 && HasDisplayValue(diskRow[8]))
			{
				return diskRow[8];
			}
		}

		if (info.PhysicalDriveId >= 0)
		{
			CString devicePath;
			devicePath.Format(_T("\\\\.\\PhysicalDrive%d"), info.PhysicalDriveId);
			return devicePath;
		}
		return _T("N/A");
	}

	void FillNvmeTransferModeFallback(const CAtaSmart::ATA_SMART_INFO& info, CString& transferMode, CString& maxTransferMode)
	{
		const CString interfaceLower = ToLower(info.Interface);
		const bool isNvme = info.IsNVMe ||
			interfaceLower.Find(_T("nvm express")) >= 0 ||
			interfaceLower.Find(_T("nvme")) >= 0;
		if (!isNvme)
		{
			return;
		}

		if (info.PhysicalDriveId >= 0 && (!HasDisplayValue(transferMode) || !HasDisplayValue(maxTransferMode)))
		{
			const SlotMaxCurrSpeed slotSpeed = GetPCIeSlotSpeed(info.PhysicalDriveId, TRUE);
			const CString currentSlotSpeed = SlotSpeedToString(slotSpeed.Current);
			const CString maxSlotSpeed = SlotSpeedToString(slotSpeed.Maximum);
			if (!HasDisplayValue(transferMode) && HasDisplayValue(currentSlotSpeed))
			{
				transferMode = currentSlotSpeed;
			}
			if (!HasDisplayValue(maxTransferMode) && HasDisplayValue(maxSlotSpeed))
			{
				maxTransferMode = maxSlotSpeed;
			}
		}

		if (!HasDisplayValue(transferMode))
		{
			transferMode = _T("PCIe/NVMe");
		}
		if (!HasDisplayValue(maxTransferMode))
		{
			maxTransferMode = _T("PCIe/NVMe");
		}
	}

	bool WriteUtf8TextFile(const CString& filePath, const CString& text, CString& errorMessage)
	{
		errorMessage.Empty();

		const int wideCharCount = text.GetLength();
		if (wideCharCount <= 0)
		{
			errorMessage = _T("报告内容为空，无法写入文件。");
			return false;
		}

		const int utf8Bytes = WideCharToMultiByte(CP_UTF8, 0, text, wideCharCount, nullptr, 0, nullptr, nullptr);
		if (utf8Bytes <= 0)
		{
			errorMessage = _T("报告编码长度计算失败。");
			return false;
		}

		std::vector<char> utf8(static_cast<size_t>(utf8Bytes), 0);
		const int converted = WideCharToMultiByte(
			CP_UTF8, 0, text, wideCharCount, utf8.data(), utf8Bytes, nullptr, nullptr);
		if (converted != utf8Bytes)
		{
			errorMessage = _T("报告编码转换失败。");
			return false;
		}

		CFileException exception;
		CFile file;
		if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary, &exception))
		{
			TCHAR err[512] = {};
			exception.GetErrorMessage(err, _countof(err));
			errorMessage.Format(_T("无法写入报告文件：%s"), err);
			return false;
		}

		const BYTE utf8Bom[] = { 0xEF, 0xBB, 0xBF };
		file.Write(utf8Bom, sizeof(utf8Bom));
		file.Write(utf8.data(), static_cast<UINT>(utf8.size()));
		file.Close();
		return true;
	}

	struct DirectSmartData
	{
		bool hasValue = false;
		CString predictFailure = _T("未知");
		CString reasonCode = _T("N/A");
		CString temperature = _T("N/A");
		CString powerOnHours = _T("N/A");
		CString powerOnCount = _T("N/A");
		CString life = _T("N/A");
		CString hostReads = _T("N/A");
		CString hostWrites = _T("N/A");
		CString source = _T("AtaSmart直连");
	};

	int ExtractPhysicalDriveId(const CString& devicePath)
	{
		CString lower = ToLower(devicePath);
		const int keyPos = lower.Find(_T("physicaldrive"));
		if (keyPos < 0)
		{
			return -1;
		}

		CString digits;
		for (int i = keyPos + 13; i < lower.GetLength(); ++i)
		{
			const wchar_t ch = lower[i];
			if (ch >= L'0' && ch <= L'9')
			{
				digits.AppendChar(ch);
			}
			else
			{
				break;
			}
		}
		return digits.IsEmpty() ? -1 : _wtoi(digits);
	}

	bool QueryAtaSmartVendorData(int physicalDriveId, std::vector<unsigned char>& vendorData)
	{
		vendorData.clear();
		if (physicalDriveId < 0)
		{
			return false;
		}

		CString path;
		path.Format(_T("\\\\.\\PhysicalDrive%d"), physicalDriveId);
		HANDLE hDevice = CreateFile(
			path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		SENDCMDINPARAMS inParams = {};
		inParams.cBufferSize = READ_ATTRIBUTE_BUFFER_SIZE;
		inParams.irDriveRegs.bFeaturesReg = READ_ATTRIBUTES;
		inParams.irDriveRegs.bSectorCountReg = 1;
		inParams.irDriveRegs.bSectorNumberReg = 1;
		inParams.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
		inParams.irDriveRegs.bCylHighReg = SMART_CYL_HI;
		inParams.irDriveRegs.bCommandReg = SMART_CMD;
		inParams.bDriveNumber = static_cast<BYTE>(physicalDriveId);

		std::vector<unsigned char> outBuffer(sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE, 0);
		DWORD bytesReturned = 0;
		const BOOL ok = DeviceIoControl(
			hDevice,
			SMART_RCV_DRIVE_DATA,
			&inParams,
			sizeof(SENDCMDINPARAMS) - 1,
			outBuffer.data(),
			static_cast<DWORD>(outBuffer.size()),
			&bytesReturned,
			nullptr);
		CloseHandle(hDevice);
		if (!ok || bytesReturned < sizeof(SENDCMDOUTPARAMS))
		{
			return false;
		}

		SENDCMDOUTPARAMS* outParams = reinterpret_cast<SENDCMDOUTPARAMS*>(outBuffer.data());
		vendorData.assign(outParams->bBuffer, outParams->bBuffer + READ_ATTRIBUTE_BUFFER_SIZE);
		return true;
	}

	bool QueryNvmeHealthData(int physicalDriveId, std::vector<unsigned char>& logData)
	{
		logData.clear();
		if (physicalDriveId < 0)
		{
			return false;
		}

		CString path;
		path.Format(_T("\\\\.\\PhysicalDrive%d"), physicalDriveId);
		HANDLE hDevice = CreateFile(
			path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		const DWORD protocolDataLength = 512;
		const DWORD bufferLength = sizeof(STORAGE_PROPERTY_QUERY) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) + protocolDataLength;
		std::vector<unsigned char> buffer(bufferLength, 0);

		STORAGE_PROPERTY_QUERY* query = reinterpret_cast<STORAGE_PROPERTY_QUERY*>(buffer.data());
		query->PropertyId = StorageDeviceProtocolSpecificProperty;
		query->QueryType = PropertyStandardQuery;

		STORAGE_PROTOCOL_SPECIFIC_DATA* protocol = reinterpret_cast<STORAGE_PROTOCOL_SPECIFIC_DATA*>(query->AdditionalParameters);
		protocol->ProtocolType = ProtocolTypeNvme;
		protocol->DataType = NVMeDataTypeLogPage;
		protocol->ProtocolDataRequestValue = 0x02; // NVMe Health Information
		protocol->ProtocolDataRequestSubValue = 0;
		protocol->ProtocolDataOffset = sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA);
		protocol->ProtocolDataLength = protocolDataLength;

		DWORD bytesReturned = 0;
		const BOOL ok = DeviceIoControl(
			hDevice,
			IOCTL_STORAGE_QUERY_PROPERTY,
			buffer.data(),
			bufferLength,
			buffer.data(),
			bufferLength,
			&bytesReturned,
			nullptr);
		CloseHandle(hDevice);
		if (!ok || bytesReturned < sizeof(STORAGE_PROPERTY_QUERY) + sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA))
		{
			return false;
		}

		STORAGE_PROTOCOL_SPECIFIC_DATA* outProtocol = reinterpret_cast<STORAGE_PROTOCOL_SPECIFIC_DATA*>(
			reinterpret_cast<unsigned char*>(buffer.data()) + sizeof(STORAGE_PROPERTY_QUERY));
		if (outProtocol->ProtocolDataLength < protocolDataLength ||
			outProtocol->ProtocolDataOffset < sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) ||
			outProtocol->ProtocolDataOffset + protocolDataLength > buffer.size() - sizeof(STORAGE_PROPERTY_QUERY))
		{
			return false;
		}

		unsigned char* payload = reinterpret_cast<unsigned char*>(outProtocol) + outProtocol->ProtocolDataOffset;
		logData.assign(payload, payload + protocolDataLength);
		return true;
	}

	unsigned long long ParseLeInteger(const std::vector<unsigned char>& data, size_t offset, size_t length)
	{
		unsigned long long value = 0;
		if (offset + length > data.size())
		{
			return 0;
		}
		const size_t maxBytes = min(length, sizeof(unsigned long long));
		for (size_t i = 0; i < maxBytes; ++i)
		{
			value |= static_cast<unsigned long long>(data[offset + i]) << (8 * i);
		}
		return value;
	}

	long double ParseLeInteger128AsLongDouble(const std::vector<unsigned char>& data, size_t offset, size_t length)
	{
		if (offset + length > data.size() || length == 0)
		{
			return 0.0L;
		}

		const size_t maxBytes = min(length, static_cast<size_t>(16));
		long double value = 0.0L;
		long double factor = 1.0L;
		for (size_t i = 0; i < maxBytes; ++i)
		{
			value += static_cast<long double>(data[offset + i]) * factor;
			factor *= 256.0L;
		}
		return value;
	}

	long double ParseLeInteger128FromBuffer(const BYTE* data, size_t dataSize, size_t offset, size_t length)
	{
		if (data == nullptr || length == 0 || offset + length > dataSize)
		{
			return 0.0L;
		}

		const size_t maxBytes = min(length, static_cast<size_t>(16));
		long double value = 0.0L;
		long double factor = 1.0L;
		for (size_t i = 0; i < maxBytes; ++i)
		{
			value += static_cast<long double>(data[offset + i]) * factor;
			factor *= 256.0L;
		}
		return value;
	}

	CString FormatNvmeDataUnits(long double dataUnits)
	{
		if (dataUnits <= 0.0L)
		{
			return _T("N/A");
		}

		const long double totalGb = (dataUnits * 512000.0L) / (1024.0L * 1024.0L * 1024.0L);
		CString text;
		if (totalGb >= 1.0L)
		{
			text.Format(_T("%.1Lf GB"), totalGb);
		}
		else
		{
			text.Format(_T("%.1Lf MB"), totalGb * 1024.0L);
		}
		return text;
	}

	DirectSmartData ReadDirectSmartData(int physicalDriveId)
	{
		DirectSmartData info;
		std::vector<unsigned char> nvmeLog;
		if (QueryNvmeHealthData(physicalDriveId, nvmeLog))
		{
			info.hasValue = true;
			info.source = _T("AtaSmart直连 (NVMe Storage Query)");
			const unsigned int temperatureKelvin = static_cast<unsigned int>(ParseLeInteger(nvmeLog, 1, 2));
			if (temperatureKelvin > 0)
			{
				const int temperatureCelsius = static_cast<int>(temperatureKelvin) - 273;
				info.temperature.Format(_T("%d °C"), temperatureCelsius);
			}

			const unsigned char criticalWarning = nvmeLog.size() > 0 ? nvmeLog[0] : 0;
			info.predictFailure = criticalWarning == 0 ? _T("否") : _T("是");
			info.reasonCode.Format(_T("0x%02X"), criticalWarning);

			const unsigned long long powerCycles = ParseLeInteger(nvmeLog, 112, 16);
			const unsigned long long powerOnHours = ParseLeInteger(nvmeLog, 128, 16);
			if (powerCycles > 0)
			{
				info.powerOnCount.Format(_T("%llu 次"), powerCycles);
			}
			if (powerOnHours > 0)
			{
				info.powerOnHours.Format(_T("%llu 小时"), powerOnHours);
			}

			if (nvmeLog.size() > 5 && nvmeLog[5] <= 100)
			{
				const int lifeRemain = max(0, 100 - static_cast<int>(nvmeLog[5]));
				info.life.Format(_T("%d %%"), lifeRemain);
			}

			const long double dataUnitsRead128 = ParseLeInteger128AsLongDouble(nvmeLog, 32, 16);
			const long double dataUnitsWritten128 = ParseLeInteger128AsLongDouble(nvmeLog, 48, 16);
			if (dataUnitsRead128 > 0.0L)
			{
				const long double totalGb = (dataUnitsRead128 * 512000.0L) / (1024.0L * 1024.0L * 1024.0L);
				if (totalGb >= 1.0L)
				{
					info.hostReads.Format(_T("%.1Lf GB"), totalGb);
				}
				else if (totalGb > 0.0L)
				{
					info.hostReads.Format(_T("%.1Lf MB"), totalGb * 1024.0L);
				}
			}
			if (dataUnitsWritten128 > 0.0L)
			{
				const long double totalGb = (dataUnitsWritten128 * 512000.0L) / (1024.0L * 1024.0L * 1024.0L);
				if (totalGb >= 1.0L)
				{
					info.hostWrites.Format(_T("%.1Lf GB"), totalGb);
				}
				else if (totalGb > 0.0L)
				{
					info.hostWrites.Format(_T("%.1Lf MB"), totalGb * 1024.0L);
				}
			}

			// 部分固件不填 Data Units，回退到命令计数避免显示 0.0 GB。
			if (!HasValue(info.hostReads))
			{
				const unsigned long long readCommands = ParseLeInteger(nvmeLog, 80, 16);
				if (readCommands > 0)
				{
					info.hostReads.Format(_T("%llu 次命令"), readCommands);
				}
			}
			if (!HasValue(info.hostWrites))
			{
				const unsigned long long writeCommands = ParseLeInteger(nvmeLog, 96, 16);
				if (writeCommands > 0)
				{
					info.hostWrites.Format(_T("%llu 次命令"), writeCommands);
				}
			}
			return info;
		}

		std::vector<unsigned char> smartVendorData;
		if (QueryAtaSmartVendorData(physicalDriveId, smartVendorData))
		{
			info.hasValue = true;
			info.source = _T("AtaSmart直连 (ATA SMART)");
			unsigned long long rawValue = 0;
			unsigned char currentValue = 0;
			if (TryGetSmartRawValue(smartVendorData, 0xC2, rawValue, currentValue) ||
				TryGetSmartRawValue(smartVendorData, 0xBE, rawValue, currentValue))
			{
				info.temperature.Format(_T("%llu °C"), rawValue & 0xFFULL);
			}
			if (TryGetSmartRawValue(smartVendorData, 0x09, rawValue, currentValue))
			{
				info.powerOnHours.Format(_T("%llu 小时"), rawValue);
			}
			if (TryGetSmartRawValue(smartVendorData, 0x0C, rawValue, currentValue))
			{
				info.powerOnCount.Format(_T("%llu 次"), rawValue);
			}
			if (TryGetSmartRawValue(smartVendorData, 0xE7, rawValue, currentValue) ||
				TryGetSmartRawValue(smartVendorData, 0xE8, rawValue, currentValue))
			{
				info.life.Format(_T("%u %%"), static_cast<unsigned>(currentValue));
			}
			info.predictFailure = _T("未知");
			info.reasonCode = _T("N/A");
		}
		return info;
	}

	// 初始化 COM/WMI 安全上下文；成功后供后续查询复用。
	bool EnsureComInitialized()
	{
		// WMI 查询依赖 COM；COM 初始化状态是线程级的，后台加载线程需要各自初始化。
		thread_local bool initialized = false;
		thread_local bool attempted = false;
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

	// 将 WMI VARIANT 值转换为 CString，兼容多种基础类型与数组类型。
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
		// 通用 WMI 查询入口：按属性顺序返回二维表，供各类硬件信息拼装复用。
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

	// 执行查询并返回首个有效属性值。
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

	// 聚合查询结果中的去重值，并用分隔符拼接成单行文本。
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

	CString JoinUniqueValues(const std::set<CString, std::less<>>& values, const CString& separator)
	{
		CString result;
		for (const CString& value : values)
		{
			if (!result.IsEmpty())
			{
				result += separator;
			}
			result += value;
		}
		return result;
	}

	bool TryParsePositiveInt(const CString& text, int& value)
	{
		value = 0;
		const CString trimmed = Trimmed(text);
		if (trimmed.IsEmpty())
		{
			return false;
		}

		wchar_t* end = nullptr;
		const long parsed = wcstol(trimmed, &end, 10);
		if (end == nullptr || *end != L'\0' || parsed <= 0)
		{
			return false;
		}

		value = static_cast<int>(parsed);
		return true;
	}

	CString DecodeMonitorManufacturer(const CString& value)
	{
		CString text = Trimmed(value);
		if (!HasValue(text))
		{
			return _T("");
		}

		CString upper(text);
		upper.MakeUpper();
		if (upper.GetLength() == 3 &&
			iswalnum(upper[0]) &&
			iswalnum(upper[1]) &&
			iswalnum(upper[2]))
		{
			if (upper == _T("VSC")) return _T("优派");
			if (upper == _T("DEL")) return _T("戴尔");
			if (upper == _T("ACR")) return _T("宏碁");
			if (upper == _T("AOC")) return _T("AOC");
			if (upper == _T("SAM")) return _T("三星");
			if (upper == _T("GSM")) return _T("LG");
			if (upper == _T("BNQ")) return _T("明基");
			if (upper == _T("SNY")) return _T("索尼");
			if (upper == _T("PHL")) return _T("飞利浦");
			if (upper == _T("HWP")) return _T("惠普");
			if (upper == _T("LEN")) return _T("联想");
			if (upper == _T("MSI")) return _T("微星");
			if (upper == _T("ASU")) return _T("华硕");
		}

		return text;
	}

	CString DetectGpuBoardVendorFromName(const CString& gpuName)
	{
		const CString matcher = ToLower(gpuName);
		if (matcher.Find(_T("影驰")) >= 0 || matcher.Find(_T("galax")) >= 0 || matcher.Find(_T("kfa2")) >= 0) return _T("影驰");
		if (matcher.Find(_T("华硕")) >= 0 || matcher.Find(_T("asus")) >= 0) return _T("华硕");
		if (matcher.Find(_T("微星")) >= 0 || matcher.Find(_T("msi")) >= 0) return _T("微星");
		if (matcher.Find(_T("技嘉")) >= 0 || matcher.Find(_T("gigabyte")) >= 0 || matcher.Find(_T("aorus")) >= 0) return _T("技嘉");
		if (matcher.Find(_T("七彩虹")) >= 0 || matcher.Find(_T("colorful")) >= 0) return _T("七彩虹");
		if (matcher.Find(_T("索泰")) >= 0 || matcher.Find(_T("zotac")) >= 0) return _T("索泰");
		if (matcher.Find(_T("映众")) >= 0 || matcher.Find(_T("inno3d")) >= 0) return _T("映众");
		if (matcher.Find(_T("耕升")) >= 0 || matcher.Find(_T("gainward")) >= 0) return _T("耕升");
		if (matcher.Find(_T("蓝宝石")) >= 0 || matcher.Find(_T("sapphire")) >= 0) return _T("蓝宝石");
		if (matcher.Find(_T("讯景")) >= 0 || matcher.Find(_T("xfx")) >= 0) return _T("讯景");
		if (matcher.Find(_T("撼讯")) >= 0 || matcher.Find(_T("powercolor")) >= 0) return _T("撼讯");
		if (matcher.Find(_T("盈通")) >= 0 || matcher.Find(_T("yeston")) >= 0) return _T("盈通");
		if (matcher.Find(_T("铭瑄")) >= 0 || matcher.Find(_T("maxsun")) >= 0) return _T("铭瑄");
		return _T("");
	}

	CString NormalizeGpuChipVendor(const CString& text)
	{
		const CString matcher = ToLower(text);
		if (matcher.Find(_T("nvidia")) >= 0 || matcher.Find(_T("geforce")) >= 0 || matcher.Find(_T("quadro")) >= 0 || matcher.Find(_T("rtx")) >= 0) return _T("NVIDIA");
		if (matcher.Find(_T("amd")) >= 0 || matcher.Find(_T("radeon")) >= 0 || matcher.Find(_T("advanced micro devices")) >= 0 || matcher.Find(_T("ati")) >= 0) return _T("AMD");
		if (matcher.Find(_T("intel")) >= 0 || matcher.Find(_T("arc")) >= 0 || matcher.Find(_T("iris")) >= 0 || matcher.Find(_T("uhd")) >= 0) return _T("Intel");
		return _T("");
	}

	bool IsVirtualGpuAdapter(const CString& gpuName, const CString& pnpDeviceId)
	{
		const CString nameLower = ToLower(gpuName);
		const CString pnpLower = ToLower(pnpDeviceId);
		return nameLower.Find(_T("idd")) >= 0 ||
			nameLower.Find(_T("oray")) >= 0 ||
			nameLower.Find(_T("virtual")) >= 0 ||
			nameLower.Find(_T("remote")) >= 0 ||
			pnpLower.Find(_T("root\\")) == 0 ||
			pnpLower.Find(_T("swd\\")) == 0;
	}

	CString ResolveGpuModel()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, PNPDeviceID FROM Win32_VideoController"),
			{ _T("Name"), _T("PNPDeviceID") });

		std::set<CString, std::less<>> models;
		for (const auto& row : rows)
		{
			if (row.size() < 2 || !HasValue(row[0]))
			{
				continue;
			}

			if (IsVirtualGpuAdapter(row[0], row[1]))
			{
				continue;
			}

			models.insert(Trimmed(row[0]));
		}

		return JoinUniqueValues(models, _T(" / "));
	}

	CString ExtractMonitorCodeFromInstance(const CString& instanceName)
	{
		const CString source = Trimmed(instanceName);
		if (!HasValue(source))
		{
			return _T("");
		}

		const CString lowered = ToLower(source);
		int start = -1;
		int tokenLength = 0;
		const int displayPos = lowered.Find(_T("display\\"));
		if (displayPos >= 0)
		{
			start = displayPos;
			tokenLength = 8;
		}
		const int monitorPos = lowered.Find(_T("monitor\\"));
		if (monitorPos >= 0 && (start < 0 || monitorPos < start))
		{
			start = monitorPos;
			tokenLength = 8;
		}
		if (start < 0)
		{
			return _T("");
		}

		start += tokenLength;
		int end = source.Find(_T('\\'), start);
		if (end < 0)
		{
			end = source.Find(_T('_'), start);
		}
		if (end < 0)
		{
			end = source.GetLength();
		}
		if (end <= start)
		{
			return _T("");
		}

		const CString modelCode = Trimmed(source.Mid(start, end - start));
		if (!HasValue(modelCode) || IsGenericMonitorLabel(modelCode))
		{
			return _T("");
		}
		return modelCode;
	}

	CString ResolveGpuVendor()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, AdapterCompatibility, PNPDeviceID, VideoProcessor FROM Win32_VideoController"),
			{ _T("Name"), _T("AdapterCompatibility"), _T("PNPDeviceID"), _T("VideoProcessor") });

		std::set<CString, std::less<>> vendors;
		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			if (IsVirtualGpuAdapter(row[0], row[2]))
			{
				continue;
			}

			const CString chipVendor = NormalizeGpuChipVendor(row[1] + _T(" ") + row[2] + _T(" ") + row[3] + _T(" ") + row[0]);
			const CString boardVendor = DetectGpuBoardVendorFromName(row[0]);
			CString vendor;

			if (HasValue(chipVendor))
			{
				vendor = chipVendor;
			}
			if (HasValue(boardVendor) && boardVendor.CompareNoCase(chipVendor) != 0)
			{
				if (!vendor.IsEmpty())
				{
					vendor += _T(" / ");
				}
				vendor += boardVendor;
			}

			if (HasValue(vendor))
			{
				vendors.insert(vendor);
			}
		}

		return JoinUniqueValues(vendors, _T(" / "));
	}

	CString ResolveMonitorSizeInfo()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\WMI"),
			_T("SELECT MaxHorizontalImageSize, MaxVerticalImageSize FROM WmiMonitorBasicDisplayParams"),
			{ _T("MaxHorizontalImageSize"), _T("MaxVerticalImageSize") });

		std::set<CString, std::less<>> sizeValues;
		for (const auto& row : rows)
		{
			if (row.size() < 2)
			{
				continue;
			}

			int widthCm = 0;
			int heightCm = 0;
			if (!TryParsePositiveInt(row[0], widthCm) || !TryParsePositiveInt(row[1], heightCm))
			{
				continue;
			}

			const double diagonal = std::sqrt(static_cast<double>(widthCm * widthCm + heightCm * heightCm)) / 2.54;
			CString sizeText;
			if (diagonal >= 1.0)
			{
				sizeText.Format(_T("%.1f英寸 (%d x %d cm)"), diagonal, widthCm, heightCm);
			}
			else
			{
				sizeText.Format(_T("%d x %d cm"), widthCm, heightCm);
			}

			sizeValues.insert(sizeText);
		}

		return JoinUniqueValues(sizeValues, _T(" / "));
	}

	CString ResolveMonitorManufacturer()
	{
		std::set<CString, std::less<>> manufacturers;

		const auto monitorIdRows = QueryWmiRows(
			_T("ROOT\\WMI"),
			_T("SELECT ManufacturerName FROM WmiMonitorID"),
			{ _T("ManufacturerName") });
		for (const auto& row : monitorIdRows)
		{
			if (row.empty())
			{
				continue;
			}

			const CString decoded = DecodeMonitorManufacturer(row[0]);
			if (HasValue(decoded))
			{
				manufacturers.insert(decoded);
			}
		}

		if (manufacturers.empty())
		{
			const auto pnpRows = QueryWmiRows(
				_T("ROOT\\CIMV2"),
				_T("SELECT Manufacturer FROM Win32_PnPEntity WHERE PNPClass='Monitor'"),
				{ _T("Manufacturer") });
			for (const auto& row : pnpRows)
			{
				if (row.empty() || !HasValue(row[0]))
				{
					continue;
				}

				const CString manufacturer = Trimmed(row[0]);
				const CString lower = ToLower(manufacturer);
				if (lower.Find(_T("generic")) >= 0 || lower.Find(_T("monitor")) >= 0)
				{
					continue;
				}
				manufacturers.insert(manufacturer);
			}
		}

		return JoinUniqueValues(manufacturers, _T(" / "));
	}

	// 解析显示器型号，按数据源可靠性进行多级回退。
	CString ResolveMonitorModel()
	{
		// 优先使用注册表 EDID 解析出的 Monitor Name；拿不到再降级到 WMI。
		const CString edidMonitorName = ResolveMonitorNameFromEdid();
		if (HasValue(edidMonitorName))
		{
			return edidMonitorName;
		}

		const auto monitorRows = QueryWmiRows(
			_T("ROOT\\WMI"),
			_T("SELECT UserFriendlyName, InstanceName, Active FROM WmiMonitorID"),
			{ _T("UserFriendlyName"), _T("InstanceName"), _T("Active") });

		std::set<CString, std::less<>> activeMonitorNames;
		std::set<CString, std::less<>> allMonitorNames;
		bool hasActiveMonitor = false;
		for (const auto& row : monitorRows)
		{
			if (row.size() < 3)
			{
				continue;
			}

			const bool isActive = row[2].CompareNoCase(_T("True")) == 0;
			if (isActive)
			{
				hasActiveMonitor = true;
			}

			const CString normalizedName = NormalizeMonitorLabel(row[0]);
			const CString fallbackCode = ExtractMonitorCodeFromInstance(row[1]);
			const CString resolvedName = HasValue(normalizedName) ? normalizedName : fallbackCode;
			if (HasValue(resolvedName))
			{
				allMonitorNames.insert(resolvedName);
				if (isActive)
				{
					activeMonitorNames.insert(resolvedName);
				}
			}
		}

		bool activeLooksLikeIdOnly = hasActiveMonitor && !activeMonitorNames.empty();
		for (const CString& name : activeMonitorNames)
		{
			if (!IsLikelyMonitorIdToken(name))
			{
				activeLooksLikeIdOnly = false;
				break;
			}
		}
		if (hasActiveMonitor && !activeMonitorNames.empty() && !activeLooksLikeIdOnly)
		{
			return JoinUniqueValues(activeMonitorNames, _T("\r\n"));
		}

		bool allLooksLikeIdOnly = !allMonitorNames.empty();
		for (const CString& name : allMonitorNames)
		{
			if (!IsLikelyMonitorIdToken(name))
			{
				allLooksLikeIdOnly = false;
				break;
			}
		}
		if (!allMonitorNames.empty() && !allLooksLikeIdOnly)
		{
			return JoinUniqueValues(allMonitorNames, _T("\r\n"));
		}

		// 回退到 PnP/DesktopMonitor 名称，并对 Generic 包装名做提纯。
		const auto pnpNameRows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name FROM Win32_PnPEntity WHERE PNPClass='Monitor'"),
			{ _T("Name") });
		std::set<CString, std::less<>> pnpNames;
		for (const auto& row : pnpNameRows)
		{
			if (row.empty())
			{
				continue;
			}
			const CString normalizedName = NormalizeMonitorLabel(row[0]);
			if (HasValue(normalizedName))
			{
				pnpNames.insert(normalizedName);
			}
		}
		if (!pnpNames.empty())
		{
			return JoinUniqueValues(pnpNames, _T("\r\n"));
		}

		const auto desktopNameRows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name FROM Win32_DesktopMonitor"),
			{ _T("Name") });
		std::set<CString, std::less<>> desktopNames;
		for (const auto& row : desktopNameRows)
		{
			if (row.empty())
			{
				continue;
			}
			const CString normalizedName = NormalizeMonitorLabel(row[0]);
			if (HasValue(normalizedName))
			{
				desktopNames.insert(normalizedName);
			}
		}
		if (!desktopNames.empty())
		{
			return JoinUniqueValues(desktopNames, _T("\r\n"));
		}

		// 最后一层才回退到 PNP 设备标识中的型号代码，避免把显示器 ID 当名称展示。
		const auto desktopRows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT PNPDeviceID FROM Win32_DesktopMonitor"),
			{ _T("PNPDeviceID") });
		std::set<CString, std::less<>> monitorCodes;
		for (const auto& row : desktopRows)
		{
			if (row.empty())
			{
				continue;
			}
			const CString modelCode = ExtractMonitorCodeFromInstance(row[0]);
			if (HasValue(modelCode))
			{
				monitorCodes.insert(modelCode);
			}
		}
		return JoinUniqueValues(monitorCodes, _T("\r\n"));
	}

	// 解析内存总容量与每条内存参数，生成可读摘要。
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

		std::vector<CString> modules;
		modules.reserve(rows.size());

		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			const unsigned long long bytes = ParseUnsignedLongLong(row[0]);

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

	CString VideoModeText(const CString& width, const CString& height, const CString& refreshRate, const CString& bitsPerPixel)
	{
		if (!HasValue(width) || !HasValue(height))
		{
			return _T("");
		}

		CString text;
		text.Format(_T("%s x %s"), width.GetString(), height.GetString());
		if (HasValue(refreshRate))
		{
			text += _T(" @ ");
			text += refreshRate;
			text += _T(" Hz");
		}
		if (HasValue(bitsPerPixel))
		{
			text += _T(" / ");
			text += bitsPerPixel;
			text += _T("-bit");
		}
		return text;
	}

	unsigned long long ResolveDisplayAdapterMemoryBytes(const CString& pnpDeviceId, const CString& adapterRam)
	{
		const CString deviceParamsKey = _T("SYSTEM\\CurrentControlSet\\Enum\\") + pnpDeviceId + _T("\\Device Parameters");
		ULONGLONG qwordValue = 0;
		DWORD valueBytes = sizeof(qwordValue);
		LSTATUS status = RegGetValue(
			HKEY_LOCAL_MACHINE,
			deviceParamsKey,
			_T("HardwareInformation.qwMemorySize"),
			RRF_RT_REG_QWORD,
			nullptr,
			&qwordValue,
			&valueBytes);
		if (status == ERROR_SUCCESS && qwordValue > 0)
		{
			return static_cast<unsigned long long>(qwordValue);
		}

		DWORD dwordValue = 0;
		valueBytes = sizeof(dwordValue);
		status = RegGetValue(
			HKEY_LOCAL_MACHINE,
			deviceParamsKey,
			_T("HardwareInformation.MemorySize"),
			RRF_RT_REG_DWORD,
			nullptr,
			&dwordValue,
			&valueBytes);
		if (status == ERROR_SUCCESS && dwordValue > 0)
		{
			return static_cast<unsigned long long>(dwordValue);
		}

		const unsigned long long bytes = ParseUnsignedLongLong(adapterRam);
		const unsigned long long minReasonable = 32ull * 1024ull * 1024ull;
		const unsigned long long maxReasonable = 128ull * 1024ull * 1024ull * 1024ull;
		return (bytes >= minReasonable && bytes <= maxReasonable) ? bytes : 0;
	}

	CString ResolveGpuDetails()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, AdapterCompatibility, AdapterRAM, DriverVersion, DriverDate, VideoProcessor, CurrentHorizontalResolution, CurrentVerticalResolution, CurrentRefreshRate, CurrentBitsPerPixel, PNPDeviceID, Status FROM Win32_VideoController"),
			{ _T("Name"), _T("AdapterCompatibility"), _T("AdapterRAM"), _T("DriverVersion"), _T("DriverDate"), _T("VideoProcessor"), _T("CurrentHorizontalResolution"), _T("CurrentVerticalResolution"), _T("CurrentRefreshRate"), _T("CurrentBitsPerPixel"), _T("PNPDeviceID"), _T("Status") });

		CString result;
		unsigned int index = 0;
		for (const auto& row : rows)
		{
			if (row.size() < 12 || !HasValue(row[0]) || IsVirtualGpuAdapter(row[0], row[10]))
			{
				continue;
			}

			++index;
			if (!result.IsEmpty())
			{
				result += _T("\r\n\r\n");
			}

			CString title;
			title.Format(_T("[%u] %s"), index, row[0].GetString());
			result += title;
			if (HasValue(row[1]))
			{
				result += _T("\r\n厂商: ");
				result += row[1];
			}

			const unsigned long long adapterRam = ResolveDisplayAdapterMemoryBytes(row[10], row[2]);
			if (adapterRam > 0)
			{
				result += _T("\r\n显存: ");
				result += FormatBytesToGB(adapterRam);
			}
			else
			{
				result += _T("\r\n显存: N/A");
			}
			if (HasValue(row[3]))
			{
				result += _T("\r\n驱动版本: ");
				result += row[3];
			}
			if (HasValue(row[4]))
			{
				const CString driverDate = FormatWmiDateTime(row[4]);
				result += _T("\r\n驱动日期: ");
				result += HasValue(driverDate) ? driverDate : row[4];
			}
			if (HasValue(row[5]))
			{
				result += _T("\r\n视频处理器: ");
				result += row[5];
			}

			const CString videoMode = VideoModeText(row[6], row[7], row[8], row[9]);
			if (HasValue(videoMode))
			{
				result += _T("\r\n当前模式: ");
				result += videoMode;
			}
			if (HasValue(row[11]))
			{
				result += _T("\r\n状态: ");
				result += row[11];
			}
			if (HasValue(row[10]))
			{
				result += _T("\r\nPNP ID: ");
				result += row[10];
			}
		}
		return result;
	}

	CString MemoryTypeText(const CString& smbiosType, const CString& memoryType)
	{
		switch (_ttoi(smbiosType))
		{
		case 20: return _T("DDR");
		case 21: return _T("DDR2");
		case 24: return _T("DDR3");
		case 26: return _T("DDR4");
		case 34: return _T("DDR5");
		case 35: return _T("LPDDR");
		case 36: return _T("LPDDR2");
		case 37: return _T("LPDDR3");
		case 38: return _T("LPDDR4");
		case 40: return _T("HBM");
		case 41: return _T("HBM2");
		case 42: return _T("DDR5");
		case 43: return _T("LPDDR5");
		default: break;
		}

		switch (_ttoi(memoryType))
		{
		case 20: return _T("DDR");
		case 21: return _T("DDR2");
		case 24: return _T("DDR3");
		case 26: return _T("DDR4");
		default: return HasValue(smbiosType) ? smbiosType : memoryType;
		}
	}

	CString FormFactorText(const CString& formFactor)
	{
		switch (_ttoi(formFactor))
		{
		case 8: return _T("DIMM");
		case 12: return _T("SODIMM");
		case 13: return _T("SRIMM");
		case 15: return _T("FB-DIMM");
		default: return formFactor;
		}
	}

	CString MemoryManufacturerText(const CString& manufacturer)
	{
		const CString value = Trimmed(manufacturer);
		if (value.CompareNoCase(_T("0x0B92")) == 0 || value.CompareNoCase(_T("0B92")) == 0)
		{
			return _T("Kingbank");
		}
		if (value.CompareNoCase(_T("0x2C00")) == 0 || value.CompareNoCase(_T("2C00")) == 0)
		{
			return _T("Micron");
		}
		if (value.CompareNoCase(_T("0xCE00")) == 0 || value.CompareNoCase(_T("CE00")) == 0)
		{
			return _T("Samsung");
		}
		if (value.CompareNoCase(_T("0xAD00")) == 0 || value.CompareNoCase(_T("AD00")) == 0)
		{
			return _T("SK hynix");
		}
		return value;
	}

	std::vector<CString> ResolveMemoryDetailLines()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Capacity, Speed, ConfiguredClockSpeed, Manufacturer, PartNumber, SerialNumber, DeviceLocator, BankLabel, SMBIOSMemoryType, MemoryType, FormFactor FROM Win32_PhysicalMemory"),
			{ _T("Capacity"), _T("Speed"), _T("ConfiguredClockSpeed"), _T("Manufacturer"), _T("PartNumber"), _T("SerialNumber"), _T("DeviceLocator"), _T("BankLabel"), _T("SMBIOSMemoryType"), _T("MemoryType"), _T("FormFactor") });

		if (rows.empty())
		{
			return {};
		}

		unsigned long long totalBytes = 0;
		for (const auto& row : rows)
		{
			if (!row.empty())
			{
				totalBytes += ParseUnsignedLongLong(row[0]);
			}
		}

		std::vector<CString> lines;
		if (totalBytes > 0)
		{
			CString summary;
			summary.Format(_T("%s / %u 条"), FormatBytesToGB(totalBytes).GetString(), static_cast<unsigned>(rows.size()));
			lines.push_back(summary);
		}

		for (size_t i = 0; i < rows.size(); ++i)
		{
			const auto& row = rows[i];
			if (row.size() < 11)
			{
				continue;
			}

			CString line;
			line = HasValue(row[6]) ? row[6] : _T("Memory Slot");

			const unsigned long long capacity = ParseUnsignedLongLong(row[0]);
			if (capacity > 0)
			{
				line += _T(" | ");
				line += FormatBytesToGB(capacity);
			}
			const CString type = MemoryTypeText(row[8], row[9]);
			if (HasValue(type))
			{
				line += _T(" ");
				line += type;
			}
			const CString formFactor = FormFactorText(row[10]);
			if (HasValue(formFactor))
			{
				line += _T(" ");
				line += formFactor;
			}
			if (HasValue(row[1]))
			{
				line += _T(" | 标称 ");
				line += row[1];
				line += _T(" MHz");
			}
			if (HasValue(row[2]))
			{
				line += _T(" / 配置 ");
				line += row[2];
				line += _T(" MHz");
			}
			const CString manufacturer = MemoryManufacturerText(row[3]);
			if (HasValue(manufacturer))
			{
				line += _T(" | ");
				line += manufacturer;
			}
			if (HasValue(row[4]))
			{
				line += _T(" | PN ");
				line += row[4];
			}
			if (HasValue(row[5]))
			{
				line += _T(" | SN ");
				line += row[5];
			}
			if (HasValue(row[7]))
			{
				line += _T(" | ");
				line += row[7];
			}
			lines.push_back(line);
		}
		return lines;
	}

	// 解析磁盘型号、容量与接口类型，生成可读摘要。
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

		std::vector<CString> ssdDisks;
		std::vector<CString> allDisks;

		for (const auto& row : rows)
		{
			if (row.size() < 4)
			{
				continue;
			}

			const CString& model = row[0];
			const CString& size = row[1];
			const CString& mediaType = row[2];
			const CString& interfaceType = row[3];

			CString diskText = BuildDiskSummary(model, size, interfaceType);
			allDisks.push_back(diskText);
			if (IsLikelySsdDevice(model, mediaType, interfaceType))
			{
				ssdDisks.push_back(diskText);
			}
		}

		const std::vector<CString>* displayDisks = &allDisks;
		if (!ssdDisks.empty() &&
			(ssdDisks.size() == allDisks.size() || ssdDisks.size() >= 2))
		{
			displayDisks = &ssdDisks;
		}

		CString result;
		for (size_t i = 0; i < displayDisks->size(); ++i)
		{
			if (i != 0)
			{
				result += _T("\r\n");
			}
			result += (*displayDisks)[i];
		}
		return result;
	}

	// 解析 EC（Embedded Controller）版本号。
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

	// 按网卡类型（有线/无线）筛选并汇总 MAC 地址。
	CString ResolveMacAddress(bool wireless)
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, MACAddress, PhysicalAdapter, PNPDeviceID FROM Win32_NetworkAdapter WHERE MACAddress IS NOT NULL"),
			{ _T("Name"), _T("MACAddress"), _T("PhysicalAdapter"), _T("PNPDeviceID") });

		std::vector<CString> entries;
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
			bool skipVirtual = false;
			const TCHAR* virtualKeys[] = {
				_T("virtual"), _T("hyper-v"), _T("virtualbox"), _T("vmware"),
				_T("wan miniport"), _T("kernel debug"), _T("isatap"),
				_T("teredo"), _T("6to4"), _T("loopback"), _T("tap-"),
				_T("tap "), _T("vpn"), _T("tunnel"), _T("pseudo"),
				_T("miniport"), _T("debugger"), _T("ndis"), _T("host"),
			};
			for (const TCHAR* key : virtualKeys)
			{
				if (matcher.Find(key) >= 0)
				{
					skipVirtual = true;
					break;
				}
			}
			if (skipVirtual)
			{
				continue;
			}
			const bool isWireless =
				matcher.Find(_T("wireless")) >= 0 ||
				matcher.Find(_T("wi-fi")) >= 0 ||
				matcher.Find(_T("wifi")) >= 0 ||
				matcher.Find(_T("802.11")) >= 0 ||
				matcher.Find(_T("wlan")) >= 0;
			const bool isBluetooth = matcher.Find(_T("bluetooth")) >= 0;

			if (wireless)
			{
				if (!isWireless)
				{
					continue;
				}
			}
			else
			{
				if (isWireless || isBluetooth)
				{
					continue;
				}
			}

			CString entry = HasValue(name) ? name : _T("未知网卡");
			entry += _T(" (");
			entry += mac;
			entry += _T(")");
			entries.push_back(entry);
		}

		CString result;
		for (const CString& entry : entries)
		{
			if (!result.IsEmpty())
			{
				result += _T("\r\n");
			}
			result += entry;
		}
		return result;
	}

	// 解析蓝牙适配器地址列表。
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

	// 解析 TPM 厂商与规范版本信息。
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

	CString ReadRegistryStringValue(HKEY rootKey, const CString& subKey, const CString& valueName)
	{
		TCHAR buffer[512] = {};
		DWORD bufferBytes = sizeof(buffer);
		const LSTATUS status = RegGetValue(
			rootKey,
			subKey,
			valueName,
			RRF_RT_REG_SZ,
			nullptr,
			buffer,
			&bufferBytes);
		return status == ERROR_SUCCESS ? Trimmed(CString(buffer)) : _T("");
	}

	bool ReadRegistryDwordValue(HKEY rootKey, const CString& subKey, const CString& valueName, DWORD& value)
	{
		DWORD valueBytes = sizeof(value);
		const LSTATUS status = RegGetValue(
			rootKey,
			subKey,
			valueName,
			RRF_RT_REG_DWORD,
			nullptr,
			&value,
			&valueBytes);
		return status == ERROR_SUCCESS;
	}

	bool TryParseWmiDateTime(const CString& wmiDateTime, CTime& time)
	{
		const CString value = Trimmed(wmiDateTime);
		if (value.GetLength() < 14)
		{
			return false;
		}

		const int year = _ttoi(value.Mid(0, 4));
		const int month = _ttoi(value.Mid(4, 2));
		const int day = _ttoi(value.Mid(6, 2));
		const int hour = _ttoi(value.Mid(8, 2));
		const int minute = _ttoi(value.Mid(10, 2));
		const int second = _ttoi(value.Mid(12, 2));
		if (year <= 0 || month <= 0 || day <= 0)
		{
			return false;
		}

		time = CTime(year, month, day, hour, minute, second);
		return true;
	}

	CString FormatWmiDateTime(const CString& wmiDateTime)
	{
		CTime time;
		if (!TryParseWmiDateTime(wmiDateTime, time))
		{
			return _T("");
		}
		return time.Format(_T("%Y-%m-%d %H:%M:%S"));
	}

	CString FormatUptime(const CString& lastBootWmiDateTime)
	{
		CTime bootTime;
		if (!TryParseWmiDateTime(lastBootWmiDateTime, bootTime))
		{
			return _T("");
		}

		const CTimeSpan uptime = CTime::GetCurrentTime() - bootTime;
		CString text;
		text.Format(_T("%lld 天 %02d:%02d:%02d"),
			static_cast<long long>(uptime.GetDays()),
			uptime.GetHours(),
			uptime.GetMinutes(),
			uptime.GetSeconds());
		return text;
	}

	CString FormatUnixTime(DWORD seconds)
	{
		if (seconds == 0)
		{
			return _T("");
		}
		const CTime time(static_cast<__time64_t>(seconds));
		return time.Format(_T("%Y-%m-%d %H:%M:%S"));
	}

	CString WindowsLicenseStatusText(const CString& status)
	{
		const int value = _ttoi(status);
		switch (value)
		{
		case 0: return _T("未授权");
		case 1: return _T("已激活");
		case 2: return _T("OOB 宽限期");
		case 3: return _T("OOT 宽限期");
		case 4: return _T("非正版宽限期");
		case 5: return _T("通知模式");
		case 6: return _T("扩展宽限期");
		default: return HasValue(status) ? status : _T("");
		}
	}

	CString ResolveWindowsActivationStatus()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Name, LicenseStatus, PartialProductKey FROM SoftwareLicensingProduct WHERE PartialProductKey IS NOT NULL"),
			{ _T("Name"), _T("LicenseStatus"), _T("PartialProductKey") });

		for (const auto& row : rows)
		{
			if (row.size() < 3)
			{
				continue;
			}

			const CString nameLower = ToLower(row[0]);
			if (nameLower.Find(_T("windows")) < 0)
			{
				continue;
			}

			CString text = WindowsLicenseStatusText(row[1]);
			if (HasValue(row[2]))
			{
				text += _T("（尾号 ");
				text += row[2];
				text += _T("）");
			}
			return text;
		}
		return _T("");
	}

	CString ResolveLocaleText()
	{
		WCHAR userLocale[LOCALE_NAME_MAX_LENGTH] = {};
		WCHAR systemLocale[LOCALE_NAME_MAX_LENGTH] = {};
		GetUserDefaultLocaleName(userLocale, _countof(userLocale));
		GetSystemDefaultLocaleName(systemLocale, _countof(systemLocale));

		CString text;
		if (userLocale[0] != L'\0')
		{
			text += _T("用户 ");
			text += userLocale;
		}
		if (systemLocale[0] != L'\0')
		{
			if (!text.IsEmpty())
			{
				text += _T(" / ");
			}
			text += _T("系统 ");
			text += systemLocale;
		}
		return text;
	}

	CString ResolveTimeZoneText()
	{
		DYNAMIC_TIME_ZONE_INFORMATION timeZone = {};
		const DWORD result = GetDynamicTimeZoneInformation(&timeZone);
		if (result == TIME_ZONE_ID_INVALID)
		{
			return _T("");
		}

		CString text = HasValue(timeZone.TimeZoneKeyName) ? CString(timeZone.TimeZoneKeyName) : CString(timeZone.StandardName);
		const LONG biasMinutes = -timeZone.Bias;
		CString offset;
		offset.Format(_T("UTC%+03ld:%02ld"), biasMinutes / 60, labs(biasMinutes % 60));
		if (HasValue(text))
		{
			text += _T(" (");
			text += offset;
			text += _T(")");
			return text;
		}
		return offset;
	}

	CString ResolveSecureBootStatus()
	{
		DWORD enabled = 0;
		if (!ReadRegistryDwordValue(
			HKEY_LOCAL_MACHINE,
			_T("SYSTEM\\CurrentControlSet\\Control\\SecureBoot\\State"),
			_T("UEFISecureBootEnabled"),
			enabled))
		{
			return _T("不支持或未知");
		}
		return enabled != 0 ? _T("已启用") : _T("未启用");
	}

	CString ResolveActivePowerPlan()
	{
		CString result;
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2\\power"),
			_T("SELECT ElementName, IsActive FROM Win32_PowerPlan WHERE IsActive = TRUE"),
			{ _T("ElementName") });
		for (const auto& row : rows)
		{
			if (!row.empty() && HasValue(row[0]))
			{
				if (!result.IsEmpty())
					result += _T(" / ");
				result += row[0];
			}
		}
		if (result.IsEmpty())
		{
			result = _T("未检测到");
		}
		return result;
	}

	CString MapDeviceClassGuid(const CString& guid)
	{
		static const std::pair<const TCHAR*, const TCHAR*> kMap[] = {
			{ _T("{4d36e968-e325-11ce-bfc1-08002be10318}"), _T("显示适配器") },
			{ _T("{4d36e967-e325-11ce-bfc1-08002be10318}"), _T("磁盘驱动器") },
			{ _T("{4d36e972-e325-11ce-bfc1-08002be10318}"), _T("网络适配器") },
			{ _T("{4d36e97d-e325-11ce-bfc1-08002be10318}"), _T("系统设备") },
			{ _T("{4d36e96e-e325-11ce-bfc1-08002be10318}"), _T("监视器") },
			{ _T("{4d36e96c-e325-11ce-bfc1-08002be10318}"), _T("声音、视频和游戏控制器") },
			{ _T("{4d36e96a-e325-11ce-bfc1-08002be10318}"), _T("IDE ATA/ATAPI 控制器") },
			{ _T("{4d36e97b-e325-11ce-bfc1-08002be10318}"), _T("存储控制器") },
			{ _T("{36fc9e60-c465-11cf-8056-444553540000}"), _T("通用串行总线控制器") },
			{ _T("{4d36e96f-e325-11ce-bfc1-08002be10318}"), _T("鼠标和其他指针设备") },
			{ _T("{4d36e96b-e325-11ce-bfc1-08002be10318}"), _T("键盘") },
			{ _T("{50127dc3-0f36-415e-a6cc-4cb3be910b65}"), _T("处理器") },
			{ _T("{4d36e966-e325-11ce-bfc1-08002be10318}"), _T("计算机") },
			{ _T("{4d36e96d-e325-11ce-bfc1-08002be10318}"), _T("调制解调器") },
			{ _T("{4d36e978-e325-11ce-bfc1-08002be10318}"), _T("端口(COM / LPT)") },
			{ _T("{4d36e979-e325-11ce-bfc1-08002be10318}"), _T("打印机") },
			{ _T("{4d36e97e-e325-11ce-bfc1-08002be10318}"), _T("安全设备") },
			{ _T("{745a17a0-74d3-11d0-b6fe-00a0c90f57da}"), _T("人机接口设备") },
			{ _T("{4d36e971-e325-11ce-bfc1-08002be10318}"), _T("多媒体设备") },
			{ _T("{4d36e974-e325-11ce-bfc1-08002be10318}"), _T("非即插即用驱动程序") },
			{ _T("{4d36e975-e325-11ce-bfc1-08002be10318}"), _T("电池") },
			{ _T("{72631e54-78a4-11d0-bcf7-00a0c90f57da}"), _T("电池") },
			{ _T("{4d36e965-e325-11ce-bfc1-08002be10318}"), _T("CD-ROM 驱动器") },
			{ _T("{4d36e969-e325-11ce-bfc1-08002be10318}"), _T("软盘控制器") },
			{ _T("{4d36e973-e325-11ce-bfc1-08002be10318}"), _T("网络客户端") },
			{ _T("{4d36e970-e325-11ce-bfc1-08002be10318}"), _T("红外线设备") },
			{ _T("{c166523c-fe0c-4a94-a586-f1a80cfbbf3e}"), _T("音频输入和输出") },
			{ _T("{ca3e7ab9-b4c3-4ae6-8251-579ef933890f}"), _T("摄像头") },
			{ _T("{62f9c741-b25a-46ef-8aed-2b6f3127d8e3}"), _T("软件设备") },
			{ _T("{50906cb8-ba12-11d1-bf5c-0000f805f530}"), _T("多串口适配器") },
			{ _T("{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}"), _T("蓝牙") },
			{ _T("{f2e7dd72-6468-4e36-b6f1-6488f42c1b52}"), _T("固件") },
			{ _T("{88bae032-5a81-49f0-bc3d-a4ff138216d6}"), _T("软件组件") },
			{ _T("{1ed2bbf9-11f0-4084-b21f-ad83a8e6dcdc}"), _T("打印机队列") },
			{ _T("{d61ca365-5af4-4486-998b-9db4734c6ca3}"), _T("传感器") },
		};
		const CString lower = ToLower(guid);
		for (const auto& pair : kMap)
		{
			if (lower.CompareNoCase(pair.first) == 0)
			{
				return pair.second;
			}
		}
		return _T("其他设备");
	}

	CString NormalizeDeviceClassName(const CString& deviceClass, const CString& classGuid)
	{
		if (HasValue(classGuid))
		{
			const CString mapped = MapDeviceClassGuid(classGuid);
			if (HasValue(mapped) && mapped != _T("其他设备"))
			{
				return mapped;
			}
		}

		CString cls = Trimmed(deviceClass);
		if (!HasValue(cls))
		{
			return _T("其他设备");
		}

		const CString lower = ToLower(cls);
		static const std::pair<const TCHAR*, const TCHAR*> kMap[] = {
			{ _T("diskdrive"), _T("磁盘驱动器") },
			{ _T("display"), _T("显示适配器") },
			{ _T("net"), _T("网络适配器") },
			{ _T("media"), _T("声音、视频和游戏控制器") },
			{ _T("usb"), _T("通用串行总线控制器") },
			{ _T("hidclass"), _T("人机接口设备") },
			{ _T("keyboard"), _T("键盘") },
			{ _T("mouse"), _T("鼠标和其他指针设备") },
			{ _T("monitor"), _T("监视器") },
			{ _T("processor"), _T("处理器") },
			{ _T("computer"), _T("计算机") },
			{ _T("system"), _T("系统设备") },
			{ _T("scsiadapter"), _T("存储控制器") },
			{ _T("hdc"), _T("IDE ATA/ATAPI 控制器") },
			{ _T("ports"), _T("端口(COM / LPT)") },
			{ _T("printer"), _T("打印机") },
			{ _T("printqueue"), _T("打印队列") },
			{ _T("softwaredevice"), _T("软件设备") },
			{ _T("softwarecomponent"), _T("软件组件") },
			{ _T("bluetooth"), _T("蓝牙") },
			{ _T("battery"), _T("电池") },
			{ _T("firmware"), _T("固件") },
			{ _T("securitydevices"), _T("安全设备") },
			{ _T("biometric"), _T("生物识别设备") },
			{ _T("camera"), _T("摄像头") },
		};

		for (const auto& pair : kMap)
		{
			if (lower.CompareNoCase(pair.first) == 0)
			{
				return pair.second;
			}
		}
		return cls;
	}

	CString ReadSetupDeviceProperty(HDEVINFO deviceInfoSet, SP_DEVINFO_DATA& deviceInfoData, DWORD property)
	{
		DWORD dataType = 0;
		DWORD requiredSize = 0;
		SetupDiGetDeviceRegistryProperty(
			deviceInfoSet,
			&deviceInfoData,
			property,
			&dataType,
			nullptr,
			0,
			&requiredSize);

		if (requiredSize == 0)
		{
			return _T("");
		}

		std::vector<BYTE> buffer(requiredSize + sizeof(TCHAR), 0);
		if (!SetupDiGetDeviceRegistryProperty(
			deviceInfoSet,
			&deviceInfoData,
			property,
			&dataType,
			buffer.data(),
			static_cast<DWORD>(buffer.size()),
			nullptr))
		{
			return _T("");
		}

		if (dataType != REG_SZ && dataType != REG_EXPAND_SZ)
		{
			return _T("");
		}

		return Trimmed(CString(reinterpret_cast<const TCHAR*>(buffer.data())));
	}

	CString ReadDriverKeyValue(const CString& driverKey, const CString& valueName)
	{
		if (!HasValue(driverKey))
		{
			return _T("");
		}

		CString keyPath = _T("SYSTEM\\CurrentControlSet\\Control\\Class\\");
		keyPath += driverKey;

		HKEY hKey = nullptr;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, keyPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		{
			return _T("");
		}

		DWORD dataType = 0;
		DWORD dataSize = 0;
		LSTATUS status = RegQueryValueEx(hKey, valueName, nullptr, &dataType, nullptr, &dataSize);
		if (status != ERROR_SUCCESS || (dataType != REG_SZ && dataType != REG_EXPAND_SZ) || dataSize == 0)
		{
			RegCloseKey(hKey);
			return _T("");
		}

		std::vector<BYTE> buffer(dataSize + sizeof(TCHAR), 0);
		status = RegQueryValueEx(hKey, valueName, nullptr, nullptr, buffer.data(), &dataSize);
		RegCloseKey(hKey);
		if (status != ERROR_SUCCESS)
		{
			return _T("");
		}

		return Trimmed(CString(reinterpret_cast<const TCHAR*>(buffer.data())));
	}

	CString FormatDriverDateText(const CString& rawDate)
	{
		CString date = Trimmed(rawDate);
		if (!HasValue(date))
		{
			return _T("");
		}

		date.Replace(_T("-"), _T("/"));
		const int space = date.Find(_T(" "));
		if (space > 0)
		{
			date = date.Left(space);
		}
		return date;
	}

	CString BitLockerProtectionStatusText(const CString& status)
	{
		switch (_ttoi(status))
		{
		case 0: return _T("未保护");
		case 1: return _T("已保护");
		case 2: return _T("保护未知");
		default: return HasValue(status) ? status : _T("未知");
		}
	}

	CString BitLockerConversionStatusText(const CString& status)
	{
		switch (_ttoi(status))
		{
		case 0: return _T("完全解密");
		case 1: return _T("完全加密");
		case 2: return _T("正在加密");
		case 3: return _T("正在解密");
		case 4: return _T("加密暂停");
		case 5: return _T("解密暂停");
		default: return HasValue(status) ? status : _T("未知");
		}
	}

	CString ResolveBitLockerStatus()
	{
		const auto rows = QueryWmiRows(
			_T("ROOT\\CIMV2\\Security\\MicrosoftVolumeEncryption"),
			_T("SELECT DriveLetter, ProtectionStatus, ConversionStatus FROM Win32_EncryptableVolume"),
			{ _T("DriveLetter"), _T("ProtectionStatus"), _T("ConversionStatus") });

		if (rows.empty())
		{
			return _T("未检测到或无权限读取");
		}

		CString text;
		for (const auto& row : rows)
		{
			if (row.size() < 3)
			{
				continue;
			}

			CString line = HasValue(row[0]) ? row[0] : _T("无盘符卷");
			line += _T("：");
			line += BitLockerProtectionStatusText(row[1]);
			line += _T(" / ");
			line += BitLockerConversionStatusText(row[2]);

			if (!text.IsEmpty())
			{
				text += _T("\r\n");
			}
			text += line;
		}
		return text;
	}

	CString ResolveWindowsVersionText(const std::vector<CString>& osRow)
	{
		CString caption = osRow.size() > 0 ? osRow[0] : _T("");
		CString version = osRow.size() > 1 ? osRow[1] : _T("");
		CString build = osRow.size() > 2 ? osRow[2] : _T("");
		DWORD ubr = 0;
		ReadRegistryDwordValue(
			HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
			_T("UBR"),
			ubr);

		CString displayVersion = ReadRegistryStringValue(
			HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
			_T("DisplayVersion"));
		CString edition = ReadRegistryStringValue(
			HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
			_T("EditionID"));

		CString text;
		if (HasValue(caption))
		{
			text += caption;
		}
		if (HasValue(displayVersion))
		{
			text += _T(" ");
			text += displayVersion;
		}
		if (HasValue(edition))
		{
			text += _T(" (");
			text += edition;
			text += _T(")");
		}
		if (HasValue(version) || HasValue(build))
		{
			text += _T("\r\n版本 ");
			text += HasValue(version) ? version : _T("N/A");
			text += _T(" / Build ");
			text += HasValue(build) ? build : _T("N/A");
			if (ubr > 0)
			{
				CString ubrText;
				ubrText.Format(_T(".%lu"), ubr);
				text += ubrText;
			}
		}
		return text;
	}

	CString StartupRunSubKey(bool disabled)
	{
		return disabled
			? _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunDisabled")
			: _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	}

	CString StartupApprovedSubKey(const CString& approvedKey)
	{
		return _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\") + approvedKey;
	}

	bool ReadStartupApprovedEnabled(HKEY rootKey, const CString& approvedKey, const CString& valueName, bool& enabled)
	{
		HKEY key = nullptr;
		if (RegOpenKeyEx(rootKey, StartupApprovedSubKey(approvedKey), 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
		{
			return false;
		}

		BYTE data[32] = {};
		DWORD dataBytes = sizeof(data);
		DWORD type = 0;
		const LSTATUS status = RegQueryValueEx(key, valueName, nullptr, &type, data, &dataBytes);
		RegCloseKey(key);
		if (status != ERROR_SUCCESS || type != REG_BINARY || dataBytes == 0)
		{
			return false;
		}

		if (data[0] == 0x02)
		{
			enabled = true;
			return true;
		}
		if (data[0] == 0x03)
		{
			enabled = false;
			return true;
		}
		return false;
	}

	bool WriteStartupApprovedEnabled(HKEY rootKey, const CString& approvedKey, const CString& valueName, bool enabled)
	{
		HKEY key = nullptr;
		if (RegCreateKeyEx(rootKey, StartupApprovedSubKey(approvedKey), 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &key, nullptr) != ERROR_SUCCESS)
		{
			return false;
		}

		BYTE data[12] = {};
		data[0] = enabled ? 0x02 : 0x03;
		const bool ok = RegSetValueEx(key, valueName, 0, REG_BINARY, data, sizeof(data)) == ERROR_SUCCESS;
		RegCloseKey(key);
		return ok;
	}

	void DeleteStartupApprovedValue(HKEY rootKey, const CString& approvedKey, const CString& valueName)
	{
		HKEY key = nullptr;
		if (RegOpenKeyEx(rootKey, StartupApprovedSubKey(approvedKey), 0, KEY_SET_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
		{
			return;
		}
		RegDeleteValue(key, valueName);
		RegCloseKey(key);
	}

	CString KnownFolderPath(REFKNOWNFOLDERID folderId)
	{
		PWSTR path = nullptr;
		if (FAILED(SHGetKnownFolderPath(folderId, 0, nullptr, &path)) || path == nullptr)
		{
			return _T("");
		}

		CString result(path);
		CoTaskMemFree(path);
		return result;
	}

	CString CombinePath(const CString& folder, const CString& name)
	{
		CString result = folder;
		if (!result.IsEmpty() && result.Right(1) != _T("\\"))
		{
			result += _T("\\");
		}
		result += name;
		return result;
	}

	CString EnsureUniqueFilePath(const CString& folder, const CString& fileName)
	{
		CString candidate = CombinePath(folder, fileName);
		if (!PathFileExists(candidate))
		{
			return candidate;
		}

		const int dot = fileName.ReverseFind(_T('.'));
		const CString base = dot > 0 ? fileName.Left(dot) : fileName;
		const CString ext = dot > 0 ? fileName.Mid(dot) : _T("");
		for (int i = 1; i < 1000; ++i)
		{
			CString numbered;
			numbered.Format(_T("%s (%d)%s"), base.GetString(), i, ext.GetString());
			candidate = CombinePath(folder, numbered);
			if (!PathFileExists(candidate))
			{
				return candidate;
			}
		}
		return candidate;
	}

	CString FileNameFromPath(const CString& path)
	{
		return PathFindFileName(path);
	}

	CString ParentDirectory(const CString& path)
	{
		TCHAR buffer[MAX_PATH] = {};
		_tcsncpy_s(buffer, path, _TRUNCATE);
		return PathRemoveFileSpec(buffer) ? CString(buffer) : _T("");
	}

	CString QuoteCommandPath(const CString& path)
	{
		CString trimmed = Trimmed(path);
		if (trimmed.IsEmpty())
		{
			return _T("");
		}
		if (trimmed.Left(1) == _T("\""))
		{
			return trimmed;
		}
		CString quoted;
		quoted.Format(_T("\"%s\""), trimmed.GetString());
		return quoted;
	}

	CString CpuidVendor()
	{
		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 0);
		char vendor[13] = {};
		memcpy(vendor + 0, &cpuInfo[1], 4);
		memcpy(vendor + 4, &cpuInfo[3], 4);
		memcpy(vendor + 8, &cpuInfo[2], 4);
		return CString(vendor);
	}

	CString CpuidBrandString()
	{
		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 0x80000000);
		const unsigned int maxExtended = static_cast<unsigned int>(cpuInfo[0]);
		if (maxExtended < 0x80000004)
		{
			return _T("");
		}

		int brandRegs[12] = {};
		__cpuid(brandRegs + 0, 0x80000002);
		__cpuid(brandRegs + 4, 0x80000003);
		__cpuid(brandRegs + 8, 0x80000004);
		char brand[49] = {};
		memcpy(brand, brandRegs, 48);
		return Trimmed(CString(brand));
	}

	CString CpuidSignatureText()
	{
		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 1);
		const unsigned int eax = static_cast<unsigned int>(cpuInfo[0]);
		const unsigned int stepping = eax & 0xF;
		const unsigned int baseModel = (eax >> 4) & 0xF;
		const unsigned int baseFamily = (eax >> 8) & 0xF;
		const unsigned int processorType = (eax >> 12) & 0x3;
		const unsigned int extModel = (eax >> 16) & 0xF;
		const unsigned int extFamily = (eax >> 20) & 0xFF;
		const unsigned int family = baseFamily == 0xF ? baseFamily + extFamily : baseFamily;
		const unsigned int model = (baseFamily == 0x6 || baseFamily == 0xF) ? (extModel << 4) + baseModel : baseModel;

		CString text;
		text.Format(_T("0x%08X / Family %u, Model %u, Stepping %u, Type %u"),
			eax, family, model, stepping, processorType);
		return text;
	}

	CString CpuidFeatureText()
	{
		int cpuInfo[4] = {};
		__cpuid(cpuInfo, 1);
		const unsigned int ecx = static_cast<unsigned int>(cpuInfo[2]);
		const unsigned int edx = static_cast<unsigned int>(cpuInfo[3]);

		std::vector<CString> flags;
		auto addFlag = [&flags](bool hasFlag, const CString& name)
		{
			if (hasFlag)
			{
				flags.push_back(name);
			}
		};

		addFlag((edx & (1u << 23)) != 0, _T("MMX"));
		addFlag((edx & (1u << 25)) != 0, _T("SSE"));
		addFlag((edx & (1u << 26)) != 0, _T("SSE2"));
		addFlag((ecx & (1u << 0)) != 0, _T("SSE3"));
		addFlag((ecx & (1u << 9)) != 0, _T("SSSE3"));
		addFlag((ecx & (1u << 19)) != 0, _T("SSE4.1"));
		addFlag((ecx & (1u << 20)) != 0, _T("SSE4.2"));
		addFlag((ecx & (1u << 22)) != 0, _T("MOVBE"));
		addFlag((ecx & (1u << 23)) != 0, _T("POPCNT"));
		addFlag((ecx & (1u << 25)) != 0, _T("AES"));
		addFlag((ecx & (1u << 28)) != 0, _T("AVX"));
		addFlag((ecx & (1u << 12)) != 0, _T("FMA"));
		addFlag((ecx & (1u << 30)) != 0, _T("RDRAND"));

		__cpuid(cpuInfo, 0);
		const unsigned int maxLeaf = static_cast<unsigned int>(cpuInfo[0]);
		if (maxLeaf >= 7)
		{
			int leaf7[4] = {};
			__cpuidex(leaf7, 7, 0);
			const unsigned int ebx = static_cast<unsigned int>(leaf7[1]);
			addFlag((ebx & (1u << 3)) != 0, _T("BMI1"));
			addFlag((ebx & (1u << 5)) != 0, _T("AVX2"));
			addFlag((ebx & (1u << 8)) != 0, _T("BMI2"));
			addFlag((ebx & (1u << 18)) != 0, _T("RDSEED"));
			addFlag((ebx & (1u << 19)) != 0, _T("ADX"));
			addFlag((ebx & (1u << 29)) != 0, _T("SHA"));
		}

		CString text;
		for (size_t i = 0; i < flags.size(); ++i)
		{
			if (i > 0)
			{
				text += _T(", ");
			}
			text += flags[i];
		}
		return text;
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

// 构造“关于”对话框。
CAboutDlg::CAboutDlg()
	: CDialogEx(IDD_ABOUTBOX)
{
}

// 绑定“关于”对话框的数据交换逻辑。
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CMFCApplication1Dlg 对话框
// 构造主对话框并加载应用图标。
CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// 绑定主对话框的数据交换逻辑。
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
	ON_MESSAGE(WM_APP_LOAD_SSD_INFO, &CMFCApplication1Dlg::OnLoadSsdInformation)
	ON_MESSAGE(WM_APP_LOAD_SCREEN_INFO, &CMFCApplication1Dlg::OnLoadScreenInformation)
	ON_MESSAGE(WM_APP_LOAD_SYSTEM_EXCEPTION_INFO, &CMFCApplication1Dlg::OnLoadSystemExceptionInformation)
	ON_MESSAGE(WM_APP_LOAD_BATTERY_LOG_INFO, &CMFCApplication1Dlg::OnLoadBatteryLogInformation)
	ON_MESSAGE(WM_APP_LOAD_POWER_LOG_INFO, &CMFCApplication1Dlg::OnLoadPowerLogInformation)
	ON_MESSAGE(WM_APP_LOAD_DRIVER_DETAILS, &CMFCApplication1Dlg::OnLoadDriverDetails)
	ON_MESSAGE(WM_APP_APPLY_SYSTEM_INFO, &CMFCApplication1Dlg::OnApplyLoadedSystemInformation)
	ON_MESSAGE(WM_APP_APPLY_SSD_INFO, &CMFCApplication1Dlg::OnApplyLoadedSsdInformation)
	ON_MESSAGE(WM_APP_APPLY_SCREEN_INFO, &CMFCApplication1Dlg::OnApplyLoadedScreenInformation)
	ON_MESSAGE(WM_APP_APPLY_SYSTEM_EXCEPTION_INFO, &CMFCApplication1Dlg::OnApplyLoadedSystemExceptionInformation)
	ON_MESSAGE(WM_APP_APPLY_BATTERY_LOG_INFO, &CMFCApplication1Dlg::OnApplyLoadedBatteryLogInformation)
	ON_MESSAGE(WM_APP_APPLY_POWER_LOG_INFO, &CMFCApplication1Dlg::OnApplyLoadedPowerLogInformation)
	ON_MESSAGE(WM_APP_APPLY_DRIVER_DETAILS, &CMFCApplication1Dlg::OnApplyLoadedDriverDetails)
	ON_BN_CLICKED(IDC_BTN_APPLY_SETTINGS, &CMFCApplication1Dlg::OnBnClickedApplySettings)
	ON_BN_CLICKED(IDC_BTN_REBOOT, &CMFCApplication1Dlg::OnBnClickedRebootSystem)
	ON_BN_CLICKED(IDC_BTN_TOGGLE_SELECT, &CMFCApplication1Dlg::OnBnClickedToggleSelect)
	ON_BN_CLICKED(IDC_STARTUP_ENABLE, &CMFCApplication1Dlg::OnBnClickedStartupEnable)
	ON_BN_CLICKED(IDC_STARTUP_DISABLE, &CMFCApplication1Dlg::OnBnClickedStartupDisable)
	ON_BN_CLICKED(IDC_STARTUP_DELETE, &CMFCApplication1Dlg::OnBnClickedStartupDelete)
	ON_BN_CLICKED(IDC_STARTUP_REFRESH, &CMFCApplication1Dlg::OnBnClickedStartupRefresh)
	ON_BN_CLICKED(IDC_STARTUP_BROWSE, &CMFCApplication1Dlg::OnBnClickedStartupBrowse)
	ON_BN_CLICKED(IDC_STARTUP_ADD, &CMFCApplication1Dlg::OnBnClickedStartupAdd)
	ON_BN_CLICKED(IDC_BATTERY_LOG_DETAILS, &CMFCApplication1Dlg::OnBnClickedBatteryLogDetails)
	ON_BN_CLICKED(IDC_BATTERY_LOG_REFRESH, &CMFCApplication1Dlg::OnBnClickedBatteryLogRefresh)
	ON_BN_CLICKED(IDC_POWER_LOG_DETAILS, &CMFCApplication1Dlg::OnBnClickedPowerLogDetails)
	ON_BN_CLICKED(IDC_POWER_LOG_REFRESH, &CMFCApplication1Dlg::OnBnClickedPowerLogRefresh)
	ON_COMMAND_RANGE(IDC_OPTIONS_START, IDC_OPTIONS_END, &CMFCApplication1Dlg::OnSettingsOptionChanged)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SSD_TAB, &CMFCApplication1Dlg::OnTcnSelchangeSsdTab)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STARTUP_LIST, &CMFCApplication1Dlg::OnLvnItemchangedStartupList)
	ON_NOTIFY(NM_RCLICK, IDC_STARTUP_LIST, &CMFCApplication1Dlg::OnNMRClickStartupList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_STARTUP_LIST, &CMFCApplication1Dlg::OnNMCustomdrawStartupList)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DRIVER_TREE, &CMFCApplication1Dlg::OnNMCustomdrawDriverTree)
END_MESSAGE_MAP()

// 初始化主对话框：菜单、图标、布局与异步加载入口。
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
	SetWindowText(_T("SystemInspector"));
	m_hIconCpuz = AfxGetApp()->LoadIcon(IDI_TOOL_CPUZ);
	m_hIconBattery = AfxGetApp()->LoadIcon(IDI_TOOL_BATTERY);
	m_hIconCoreTemp = AfxGetApp()->LoadIcon(IDI_TOOL_CORETEMP);
	m_hIconHwinfo = AfxGetApp()->LoadIcon(IDI_TOOL_HWINFO);
	m_hIconCrystalDiskMark = AfxGetApp()->LoadIcon(IDI_TOOL_CRYSTALDISKMARK);
	m_hIconUsbTreeView = AfxGetApp()->LoadIcon(IDI_TOOL_USBTREEVIEW);

	SHSTOCKICONINFO sii = { sizeof(SHSTOCKICONINFO) };
	if (SUCCEEDED(SHGetStockIconInfo(SIID_WORLD, SHGSI_ICON | SHGSI_LARGEICON, &sii)))
	{
		m_hIconGlobe = sii.hIcon;
	}

	CenterWindowAtTwoThirdsOfScreen(*this);
	EnableWin11Chrome(GetSafeHwnd());

	BuildMainLayout();
	CreateSettingsControls();
	CreateSsdControls();
	CreateDriverControls();
	CreateStartupControls();
	CreatePowerLogControls();
	UpdatePageVisibility();
	// 先显示界面，再通过自定义消息异步加载数据，提升启动响应速度。
	m_loading = true;
	PostMessage(WM_APP_LOAD_SYSTEM_INFO, 0, 0);

	return TRUE;
}

// 处理系统命令：拦截“关于”并弹窗，其余交给基类。
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

// 处理窗口绘制：最小化时绘图标，正常时绘制主界面。
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

// 返回窗口最小化拖拽时显示的光标。
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 响应窗口大小变化并触发布局重算与重绘。
void CMFCApplication1Dlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	AdjustLayout(cx, cy);
	if (cx > 0 && cy > 0)
	{
		Invalidate();
	}
}

// 处理侧栏点击，切换“系统信息/系统设置/SSD信息”页面。
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
	else if (m_statusMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_SYSTEM_STATUS)
		{
			m_activePage = PAGE_SYSTEM_STATUS;
			UpdatePageVisibility();
			Invalidate();
		}
	}
	else if (m_exceptionMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_SYSTEM_EXCEPTION)
		{
			m_activePage = PAGE_SYSTEM_EXCEPTION;
			UpdatePageVisibility();
			Invalidate();
		}

		if (!m_systemExceptionLoaded && !m_systemExceptionLoading)
		{
			m_systemExceptionLoading = true;
			m_systemExceptionRows.clear();
			m_systemExceptionRows.push_back({ _T("状态"), _T("系统异常信息加载中...") });
			m_scrollPos = 0;
			Invalidate();
			PostMessage(WM_APP_LOAD_SYSTEM_EXCEPTION_INFO, 0, 0);
		}
	}
	else if (m_startupMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_STARTUP_ITEMS)
		{
			m_activePage = PAGE_STARTUP_ITEMS;
			UpdatePageVisibility();
			Invalidate();
		}
	}
	else if (m_acpiMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_ACPI_INFO)
		{
			m_activePage = PAGE_ACPI_INFO;
			UpdatePageVisibility();
			Invalidate();
		}
	}
	else if (m_ssdMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_SSD_INFO)
		{
			m_activePage = PAGE_SSD_INFO;
			UpdatePageVisibility();
			Invalidate();
		}

		if (!m_ssdLoaded && !m_ssdLoading)
		{
			m_ssdLoading = true;
			m_ssdDiskRows.clear();
			m_ssdTabTitles.clear();
			m_ssdDiskRows.push_back({ { _T("状态"), _T("SSD信息加载中...") } });
			m_ssdTabTitles.push_back(_T("加载中"));
			m_ssdRows = m_ssdDiskRows.front();
			RefreshSsdTabs();
			UpdateSsdControlLayout();
			Invalidate();
			PostMessage(WM_APP_LOAD_SSD_INFO, 0, 0);
		}
	}
	else if (m_screenMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_SCREEN_INFO)
		{
			m_activePage = PAGE_SCREEN_INFO;
			UpdatePageVisibility();
			Invalidate();
		}

		if (!m_screenLoaded && !m_screenLoading)
		{
			m_screenLoading = true;
			m_screenRows.clear();
			m_screenRows.push_back({ _T("状态"), _T("屏幕详情加载中...") });
			m_scrollPos = 0;
			Invalidate();
			PostMessage(WM_APP_LOAD_SCREEN_INFO, 0, 0);
		}
	}
	else if (m_batteryLogMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_BATTERY_LOG)
		{
			m_activePage = PAGE_BATTERY_LOG;
			UpdatePageVisibility();
			Invalidate();
		}
	}
	else if (m_powerLogMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_POWER_LOG)
		{
			m_activePage = PAGE_POWER_LOG;
			UpdatePageVisibility();
			Invalidate();
		}
	}
	else if (m_utilityMenuRect.PtInRect(point))
	{
		if (m_activePage != PAGE_UTILITY_TOOLS)
		{
			m_activePage = PAGE_UTILITY_TOOLS;
			UpdatePageVisibility();
			Invalidate();
		}
	}

	if (m_activePage == PAGE_UTILITY_TOOLS)
	{
		if (m_toolCpuzRect.PtInRect(point))
			LaunchUtilityTool(IDR_TOOL_CPUZ, _T("CPUZ"), _T("cpuz_x64.exe"));
		else if (m_toolBatteryRect.PtInRect(point))
			LaunchUtilityTool(IDR_TOOL_BATTERY, _T("BatteryInfoView"), _T("BatteryInfoView.exe"));
		else if (m_toolCoreTempRect.PtInRect(point))
			LaunchUtilityTool(IDR_TOOL_CORETEMP, _T("CoreTemp64"), _T("Core Temp.exe"));
		else if (m_toolHwinfoRect.PtInRect(point))
			LaunchUtilityTool(IDR_TOOL_HWINFO, _T("hwinfo"), _T("HWiNFO64.exe"));
		else if (m_toolCrystalDiskMarkRect.PtInRect(point))
			LaunchUtilityTool(IDR_TOOL_CRYSTALDISKMARK, _T("CrystalDiskMark"), _T("DiskMark64.exe"));
		else if (m_toolUsbTreeViewRect.PtInRect(point))
			LaunchUtilityTool(IDR_TOOL_USBTREEVIEW, _T("USBTreeView"), _T("UsbTreeView.exe"));
		else if (m_toolAmdDriverRect.PtInRect(point))
			ShellExecute(nullptr, _T("open"), _T("https://www.amd.com/zh-hans/support"), nullptr, nullptr, SW_SHOWNORMAL);
		else if (m_toolNvidiaDriverRect.PtInRect(point))
			ShellExecute(nullptr, _T("open"), _T("https://www.nvidia.cn/geforce/drivers/"), nullptr, nullptr, SW_SHOWNORMAL);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

// 处理垂直滚动条消息并更新当前滚动位置。
void CMFCApplication1Dlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_activePage != PAGE_SYSTEM_INFO &&
		m_activePage != PAGE_SYSTEM_STATUS &&
		m_activePage != PAGE_SYSTEM_EXCEPTION &&
		m_activePage != PAGE_SYSTEM_SETTINGS &&
		m_activePage != PAGE_SSD_INFO &&
		m_activePage != PAGE_SCREEN_INFO &&
		m_activePage != PAGE_BATTERY_LOG &&
		m_activePage != PAGE_POWER_LOG)
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
		else if (m_activePage == PAGE_SSD_INFO)
		{
			UpdateSsdControlLayout();
		}
		Invalidate();
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

// 处理鼠标滚轮滚动，实现内容区平滑滚动。
BOOL CMFCApplication1Dlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_activePage != PAGE_SYSTEM_INFO &&
		m_activePage != PAGE_SYSTEM_STATUS &&
		m_activePage != PAGE_SYSTEM_EXCEPTION &&
		m_activePage != PAGE_SYSTEM_SETTINGS &&
		m_activePage != PAGE_SSD_INFO &&
		m_activePage != PAGE_SCREEN_INFO &&
		m_activePage != PAGE_BATTERY_LOG &&
		m_activePage != PAGE_POWER_LOG)
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
		else if (m_activePage == PAGE_SSD_INFO)
		{
			UpdateSsdControlLayout();
		}
		Invalidate();
		return TRUE;
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

// 构建主界面初始布局与窗口标题。
void CMFCApplication1Dlg::BuildMainLayout()
{
	EnsureUiFonts();
	SetWindowText(_T("SystemInspector"));

	CRect clientRect;
	GetClientRect(&clientRect);
	RecalcLayoutRects(clientRect);
}

// 根据新窗口尺寸调整布局。
void CMFCApplication1Dlg::AdjustLayout(int cx, int cy)
{
	CRect clientRect(0, 0, cx, cy);
	RecalcLayoutRects(clientRect);
	UpdateSettingsControlLayout();
	UpdateSsdControlLayout();
	UpdateDriverControlLayout();
	UpdateStartupControlLayout();
	UpdatePowerLogControlLayout();
}

// 向系统信息数据源追加一条展示记录。
void CMFCApplication1Dlg::AddSystemInfoRow(const CString& item, const CString& value)
{
	InfoRow row;
	row.item = item;
	row.value = HasValue(value) ? NormalizeMultilineValue(value) : _T("N/A");
	m_systemRows.push_back(row);
}

// 响应异步加载消息，执行系统信息采集。
LRESULT CMFCApplication1Dlg::OnLoadSystemInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadSystemInformationThread, request) == nullptr)
	{
		delete request;
		m_loading = false;
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnLoadSsdInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadSsdInformationThread, request) == nullptr)
	{
		delete request;
		m_ssdLoading = false;
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnLoadScreenInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadScreenInformationThread, request) == nullptr)
	{
		delete request;
		m_screenLoading = false;
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnLoadSystemExceptionInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadSystemExceptionInformationThread, request) == nullptr)
	{
		delete request;
		m_systemExceptionLoading = false;
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnLoadBatteryLogInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	request->batteryReport = true;
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadBatteryLogInformationThread, request) == nullptr)
	{
		delete request;
		m_batteryLogLoading = false;
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnLoadPowerLogInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	request->batteryReport = false;
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadPowerLogInformationThread, request) == nullptr)
	{
		delete request;
		m_powerLogLoading = false;
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnLoadDriverDetails(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadRequest* request = new AsyncLoadRequest;
	request->targetHwnd = GetSafeHwnd();
	if (AfxBeginThread(&CMFCApplication1Dlg::LoadDriverDetailsThread, request) == nullptr)
	{
		delete request;
		m_driverDetailsLoading = false;
		if (!m_driverDetailsLoaded)
		{
			m_driverClassGroups.clear();
			m_driverComputerName = _T("本机");
			ShowDriverDetailsLoading();
		}
		Invalidate();
	}
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedSystemInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_systemRows = result->systemRows;
	m_systemStatusRows = result->systemStatusRows;
	m_acpiRows = result->acpiRows;
	m_loading = false;
	if (m_activePage == PAGE_SYSTEM_INFO)
	{
		m_scrollPos = 0;
	}
	delete result;
	Invalidate();
	StartSilentPreload();
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedSsdInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_ssdRows = result->ssdRows;
	m_ssdDiskRows = result->ssdDiskRows;
	m_ssdTabTitles = result->ssdTabTitles;
	m_activeSsdIndex = result->activeSsdIndex;
	m_ssdLoaded = true;
	m_ssdLoading = false;
	m_scrollPos = 0;
	delete result;

	RefreshSsdTabs();
	UpdateSsdControlLayout();
	Invalidate();
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedScreenInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_screenRows = result->screenRows;
	m_screenLoaded = true;
	m_screenLoading = false;
	m_scrollPos = 0;
	delete result;
	Invalidate();
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedSystemExceptionInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_systemExceptionRows = result->systemExceptionRows;
	m_systemExceptionLoaded = true;
	m_systemExceptionLoading = false;
	m_scrollPos = 0;
	delete result;
	Invalidate();
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedBatteryLogInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_batteryLogRows = result->batteryLogRows;
	m_batteryLogPath = result->batteryLogPath;
	m_batteryLogLoaded = true;
	m_batteryLogLoading = false;
	if (m_activePage == PAGE_BATTERY_LOG)
	{
		m_scrollPos = 0;
	}
	delete result;
	Invalidate();
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedPowerLogInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_powerLogRows = result->powerLogRows;
	m_powerLogPath = result->powerLogPath;
	m_powerLogLoaded = true;
	m_powerLogLoading = false;
	if (m_activePage == PAGE_POWER_LOG)
	{
		m_scrollPos = 0;
	}
	delete result;
	Invalidate();
	return 0;
}

LRESULT CMFCApplication1Dlg::OnApplyLoadedDriverDetails(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_driverComputerName = result->driverComputerName;
	m_driverClassGroups = result->driverClassGroups;
	m_driverDetailsLoaded = true;
	m_driverDetailsLoading = false;
	delete result;

	RefreshDriverTree();
	Invalidate();
	return 0;
}

void CMFCApplication1Dlg::PostAsyncLoadResult(HWND targetHwnd, UINT message, AsyncLoadResult* result)
{
	if (::IsWindow(targetHwnd) && ::PostMessage(targetHwnd, message, reinterpret_cast<WPARAM>(result), 0))
	{
		return;
	}
	delete result;
}

UINT CMFCApplication1Dlg::LoadSystemInformationThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	loader.LoadSystemInformation(false);

	AsyncLoadResult* result = new AsyncLoadResult;
	result->systemRows = loader.m_systemRows;
	result->systemStatusRows = loader.m_systemStatusRows;
	result->acpiRows = loader.m_acpiRows;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_SYSTEM_INFO, result);
	return 0;
}

UINT CMFCApplication1Dlg::LoadSsdInformationThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	loader.LoadSystemInformation(true);

	AsyncLoadResult* result = new AsyncLoadResult;
	result->ssdRows = loader.m_ssdRows;
	result->ssdDiskRows = loader.m_ssdDiskRows;
	result->ssdTabTitles = loader.m_ssdTabTitles;
	result->activeSsdIndex = loader.m_activeSsdIndex;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_SSD_INFO, result);
	return 0;
}

UINT CMFCApplication1Dlg::LoadScreenInformationThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	loader.LoadScreenInformation();

	AsyncLoadResult* result = new AsyncLoadResult;
	result->screenRows = loader.m_screenRows;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_SCREEN_INFO, result);
	return 0;
}

UINT CMFCApplication1Dlg::LoadSystemExceptionInformationThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	loader.LoadSystemExceptionInformation();

	AsyncLoadResult* result = new AsyncLoadResult;
	result->systemExceptionRows = loader.m_systemExceptionRows;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_SYSTEM_EXCEPTION_INFO, result);
	return 0;
}

UINT CMFCApplication1Dlg::LoadBatteryLogInformationThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	loader.LoadPowerCfgReportInformation(true);

	AsyncLoadResult* result = new AsyncLoadResult;
	result->batteryLogRows = loader.m_batteryLogRows;
	result->batteryLogPath = loader.m_batteryLogPath;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_BATTERY_LOG_INFO, result);
	return 0;
}

UINT CMFCApplication1Dlg::LoadPowerLogInformationThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	loader.LoadPowerCfgReportInformation(false);

	AsyncLoadResult* result = new AsyncLoadResult;
	result->powerLogRows = loader.m_powerLogRows;
	result->powerLogPath = loader.m_powerLogPath;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_POWER_LOG_INFO, result);
	return 0;
}

UINT CMFCApplication1Dlg::LoadDriverDetailsThread(LPVOID parameter)
{
	AsyncLoadRequest* request = reinterpret_cast<AsyncLoadRequest*>(parameter);
	const HWND targetHwnd = request != nullptr ? request->targetHwnd : nullptr;
	delete request;

	CMFCApplication1Dlg loader;
	CString computerName;
	std::vector<DriverClassGroup> classGroups;
	loader.CollectDriverDetails(computerName, classGroups);

	AsyncLoadResult* result = new AsyncLoadResult;
	result->driverComputerName = computerName;
	result->driverClassGroups = classGroups;
	PostAsyncLoadResult(targetHwnd, WM_APP_APPLY_DRIVER_DETAILS, result);
	return 0;
}

bool CMFCApplication1Dlg::ExportReportToFile(const CString& reportType, const CString& filePath, CString& errorMessage)
{
	errorMessage.Empty();
	const CString targetPath = Trimmed(filePath);
	if (targetPath.IsEmpty())
	{
		errorMessage = _T("报告文件路径不能为空。");
		return false;
	}

	CString type = Trimmed(reportType);
	type.MakeUpper();
	if (type == _T("SSD"))
	{
		LoadSystemInformation(true);
	}
	else if (type == _T("SYSTEM") || type == _T("STATUS") || type == _T("ACPI"))
	{
		LoadSystemInformation(false);
	}
	else if (type == _T("EDID"))
	{
		LoadScreenInformation();
	}
	else
	{
		errorMessage = _T("不支持的报告类型。仅支持：SSD、SYSTEM、STATUS、ACPI、EDID。");
		return false;
	}

	CTime now = CTime::GetCurrentTime();
	CString reportText;
	reportText.Format(_T("SystemInspector Report\r\nType: %s\r\nGeneratedAt: %s\r\n\r\n"),
		type.GetString(),
		now.Format(_T("%Y-%m-%d %H:%M:%S")).GetString());

	if (type == _T("SSD"))
	{
		for (size_t i = 0; i < m_ssdDiskRows.size(); ++i)
		{
			CString title;
			if (i < m_ssdTabTitles.size() && HasValue(m_ssdTabTitles[i]))
			{
				title = m_ssdTabTitles[i];
			}
			else
			{
				title.Format(_T("SSD %u"), static_cast<unsigned>(i + 1));
			}

			reportText.AppendFormat(_T("[%u] %s\r\n"), static_cast<unsigned>(i + 1), title.GetString());
			for (const auto& row : m_ssdDiskRows[i])
			{
				reportText += row.item;
				reportText += _T(": ");
				reportText += row.value;
				reportText += _T("\r\n");
			}
			reportText += _T("\r\n");
		}
	}
	else if (type == _T("SYSTEM"))
	{
		for (const auto& row : m_systemRows)
		{
			reportText += row.item;
			reportText += _T(": ");
			reportText += row.value;
			reportText += _T("\r\n");
		}
	}
	else if (type == _T("STATUS"))
	{
		for (const auto& row : m_systemStatusRows)
		{
			reportText += row.item;
			reportText += _T(": ");
			reportText += row.value;
			reportText += _T("\r\n");
		}
	}
	else if (type == _T("ACPI"))
	{
		for (const auto& row : m_acpiRows)
		{
			reportText += row.item;
			reportText += _T(": ");
			reportText += row.value;
			reportText += _T("\r\n");
		}
	}
	else
	{
		for (const auto& row : m_screenRows)
		{
			reportText += row.item;
			reportText += _T(": ");
			reportText += row.value;
			reportText += _T("\r\n");
		}
	}

	return WriteUtf8TextFile(targetPath, reportText, errorMessage);
}

void CMFCApplication1Dlg::LoadSystemExceptionInformation()
{
	const bool hasWindow = ::IsWindow(GetSafeHwnd());
	m_systemExceptionRows.clear();
	m_systemExceptionLoaded = false;

	auto addSystemExceptionRow = [this](const CString& item, const CString& value)
		{
			m_systemExceptionRows.push_back({ item, HasValue(value) ? NormalizeMultilineValue(value) : _T("N/A") });
		};

	const auto abnormalDeviceRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Name, Manufacturer, Service, PNPClass, DeviceID, ConfigManagerErrorCode, Status, ErrorDescription FROM Win32_PnPEntity WHERE ConfigManagerErrorCode <> 0"),
		{ _T("Name"), _T("Manufacturer"), _T("Service"), _T("PNPClass"), _T("DeviceID"), _T("ConfigManagerErrorCode"), _T("Status"), _T("ErrorDescription") });
	if (abnormalDeviceRows.empty())
	{
		addSystemExceptionRow(_T("异常驱动"), _T("未发现设备管理器异常驱动/设备"));
	}
	else
	{
		addSystemExceptionRow(_T("异常驱动数量"), [&abnormalDeviceRows]() {
			CString count;
			count.Format(_T("%u"), static_cast<unsigned>(abnormalDeviceRows.size()));
			return count;
			}());
		for (size_t i = 0; i < abnormalDeviceRows.size(); ++i)
		{
			const auto& row = abnormalDeviceRows[i];
			CString details;
			details += _T("名称: ") + (row.size() > 0 && HasValue(row[0]) ? row[0] : _T("N/A"));
			if (row.size() > 1 && HasValue(row[1])) details += _T("\r\n厂商: ") + row[1];
			if (row.size() > 2 && HasValue(row[2])) details += _T("\r\n服务: ") + row[2];
			if (row.size() > 3 && HasValue(row[3])) details += _T("\r\n类别: ") + row[3];
			if (row.size() > 5 && HasValue(row[5])) details += _T("\r\n错误码: ") + row[5] + _T(" - ") + ConfigManagerErrorText(row[5]);
			if (row.size() > 6 && HasValue(row[6])) details += _T("\r\n状态: ") + row[6];
			if (row.size() > 7 && HasValue(row[7])) details += _T("\r\n错误描述: ") + row[7];
			if (row.size() > 4 && HasValue(row[4])) details += _T("\r\n设备ID: ") + row[4];
			CString label;
			label.Format(_T("异常驱动[%u]"), static_cast<unsigned>(i + 1));
			addSystemExceptionRow(label, details);
		}
	}

	const int eventLookbackDays = 30;
	const CString sinceTime = FormatWmiQueryTimeDaysAgo(eventLookbackDays);
	CString eventWql;
	eventWql.Format(
		_T("SELECT TimeGenerated, EventCode, SourceName, Type, Message, RecordNumber, CategoryString, User, ComputerName FROM Win32_NTLogEvent WHERE Logfile = 'System' AND TimeGenerated >= '%s' AND ((EventCode = '1001' AND SourceName = 'Microsoft-Windows-WER-SystemErrorReporting') OR (EventCode = '1003' AND SourceName = 'System Error') OR (EventCode = '41' AND (SourceName = 'System Error' OR SourceName = 'Microsoft-Windows-Kernel-Power')))"),
		static_cast<LPCTSTR>(sinceTime));

	auto exceptionEventRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		eventWql,
		{ _T("TimeGenerated"), _T("EventCode"), _T("SourceName"), _T("Type"), _T("Message"), _T("RecordNumber"), _T("CategoryString"), _T("User"), _T("ComputerName") });
	std::sort(exceptionEventRows.begin(), exceptionEventRows.end(), [](const std::vector<CString>& left, const std::vector<CString>& right) {
		const CString leftTime = left.empty() ? _T("") : left[0];
		const CString rightTime = right.empty() ? _T("") : right[0];
		return leftTime.Compare(rightTime) > 0;
		});
	if (exceptionEventRows.empty())
	{
		CString note;
		note.Format(_T("最近 %d 天未发现匹配的 System 重大异常事件"), eventLookbackDays);
		addSystemExceptionRow(_T("重大异常日志"), note);
	}
	else
	{
		CString rangeText;
		rangeText.Format(_T("最近 %d 天，共匹配 %u 条，显示最近的前 12 条。"), eventLookbackDays, static_cast<unsigned>(exceptionEventRows.size()));
		addSystemExceptionRow(_T("日志查询范围"), rangeText);
		size_t shown = 0;
		for (const auto& row : exceptionEventRows)
		{
			if (shown >= 12)
			{
				break;
			}
			CString label;
			label.Format(_T("事件 %s [%u]"),
				row.size() > 1 && HasValue(row[1]) ? row[1].GetString() : _T("N/A"),
				static_cast<unsigned>(shown + 1));
			addSystemExceptionRow(label, BuildSystemExceptionEventText(row));
			++shown;
		}
		if (exceptionEventRows.size() > shown)
		{
			CString note;
			note.Format(_T("共匹配 %u 条，当前显示最近采集到的前 %u 条。"), static_cast<unsigned>(exceptionEventRows.size()), static_cast<unsigned>(shown));
			addSystemExceptionRow(_T("日志显示范围"), note);
		}
	}

	m_systemExceptionLoaded = true;
	m_systemExceptionLoading = false;
	m_scrollPos = 0;
	if (hasWindow)
	{
		Invalidate();
	}
}

void CMFCApplication1Dlg::LoadScreenInformation()
{
	const bool hasWindow = ::IsWindow(GetSafeHwnd());
	m_screenRows.clear();
	m_screenLoaded = false;

	EdidInfo edidInfo;
	bool parsed = false;
	for (const auto& edid : ReadActiveEdidBlocksFromRegistry())
	{
		if (ParseEdid(edid, edidInfo))
		{
			parsed = true;
			break;
		}
	}

	if (!parsed)
	{
		m_screenRows.push_back({ _T("状态"), _T("未读取到当前活动显示器的有效 EDID") });
		m_screenRows.push_back({ _T("读取方式"), _T("WmiMonitorID Active=True / EnumDisplayDevices ACTIVE") });
		m_screenLoaded = true;
		m_screenLoading = false;
		if (hasWindow)
		{
			Invalidate();
		}
		return;
	}

	m_screenRows.push_back({ _T("状态"), _T("已读取 EDID") });
	m_screenRows.push_back({ _T("显示器名称"), HasValue(edidInfo.monitorName) ? edidInfo.monitorName : _T("N/A") });
	m_screenRows.push_back({ _T("显示器ID"), HasValue(edidInfo.monitorId) ? edidInfo.monitorId : _T("N/A") });
	m_screenRows.push_back({ _T("EDID日期"), HasValue(edidInfo.edidDate) ? edidInfo.edidDate : _T("N/A") });
	m_screenRows.push_back({ _T("EDID版本"), HasValue(edidInfo.edidVersion) ? edidInfo.edidVersion : _T("N/A") });
	m_screenRows.push_back({ _T("序列号"), HasValue(edidInfo.serialNumber) ? edidInfo.serialNumber : _T("N/A") });
	m_screenRows.push_back({ _T("刷新率"), HasValue(edidInfo.refreshRate) ? edidInfo.refreshRate : _T("N/A") });
	m_screenRows.push_back({ _T("校验和"), HasValue(edidInfo.checksum) ? edidInfo.checksum : _T("N/A") });
	if (HasValue(edidInfo.fullEdidHex))
	{
		m_screenRows.push_back({ _T("EDID原始数据(完整)"), edidInfo.fullEdidHex });
	}

	m_screenLoaded = true;
	m_screenLoading = false;
	m_scrollPos = 0;
	if (hasWindow)
	{
		Invalidate();
	}
}

// 聚合 WMI 信息并刷新系统信息页数据。
void CMFCApplication1Dlg::LoadSystemInformation(bool loadSsdDetails)
{
	const bool hasWindow = ::IsWindow(GetSafeHwnd());
	m_systemRows.clear();
	m_systemStatusRows.clear();
	m_acpiRows.clear();
	m_ssdRows.clear();
	m_ssdDiskRows.clear();
	m_ssdTabTitles.clear();
	m_ssdLoaded = false;
	if (!loadSsdDetails)
	{
		m_ssdLoading = false;
	}
	m_activeSsdIndex = 0;
	m_scrollPos = 0;

	// 分项采集硬件、固件和网络信息，再统一写入展示列表。
	const CString boardModel = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT Product FROM Win32_BaseBoard"), _T("Product"));
	const CString cpuModel = JoinValues(_T("ROOT\\CIMV2"), _T("SELECT Name FROM Win32_Processor"), _T("Name"));
	const CString gpuModel = ResolveGpuModel();
	const CString monitorModel = ResolveMonitorModel();
	const CString monitorSize = ResolveMonitorSizeInfo();
	CString monitorInfo = monitorModel;
	if (HasValue(monitorSize))
	{
		if (HasValue(monitorInfo))
		{
			monitorInfo += _T(" (");
			monitorInfo += monitorSize;
			monitorInfo += _T(")");
		}
		else
		{
			monitorInfo = monitorSize;
		}
	}
	const CString memoryInfo = ResolveMemoryInfo();
	const CString gpuDetails = ResolveGpuDetails();
	const std::vector<CString> memoryDetailLines = ResolveMemoryDetailLines();
	const CString ssdInfo = ResolveDiskInfo();

	CString biosVersion;
	{
		const auto biosRows = QueryWmiRows(
			_T("ROOT\\CIMV2"),
			_T("SELECT Manufacturer, SMBIOSBIOSVersion, ReleaseDate FROM Win32_BIOS"),
			{ _T("Manufacturer"), _T("SMBIOSBIOSVersion"), _T("ReleaseDate") });
		if (!biosRows.empty() && !biosRows.front().empty())
		{
			const auto& row = biosRows.front();
			if (row.size() > 0 && HasValue(row[0]))
			{
				biosVersion += row[0];
				biosVersion += _T(" ");
			}
			if (row.size() > 1 && HasValue(row[1]))
			{
				biosVersion += row[1];
			}
			if (row.size() > 2 && HasValue(row[2]))
			{
				const CString& wmiDate = row[2];
				if (wmiDate.GetLength() >= 8)
				{
					biosVersion += _T(", ");
					biosVersion += wmiDate.Left(4);
					biosVersion += _T("/");
					biosVersion += wmiDate.Mid(4, 2);
					biosVersion += _T("/");
					biosVersion += wmiDate.Mid(6, 2);
				}
			}
		}
	}
	const CString ecVersion = ResolveEcVersion();
	const CString boardSn = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT SerialNumber FROM Win32_BaseBoard"), _T("SerialNumber"));
	const CString systemSn = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT SerialNumber FROM Win32_BIOS"), _T("SerialNumber"));
	const CString uuid = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT UUID FROM Win32_ComputerSystemProduct"), _T("UUID"));
	const CString wiredMac = ResolveMacAddress(false);
	const CString wirelessMac = ResolveMacAddress(true);
	const CString bluetoothAddress = ResolveBluetoothAddress();
	const CString tpmModel = ResolveTpmModel();
	const auto csRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Name, Domain, HypervisorPresent, TotalPhysicalMemory, SystemType, UserName FROM Win32_ComputerSystem"),
		{ _T("Name"), _T("Domain"), _T("HypervisorPresent"), _T("TotalPhysicalMemory"), _T("SystemType"), _T("UserName") });
	const std::vector<CString> csRow = csRows.empty() ? std::vector<CString>() : csRows.front();
	const auto osRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Caption, Version, BuildNumber, OSArchitecture, InstallDate, LastBootUpTime, WindowsDirectory, FreePhysicalMemory, TotalVirtualMemorySize, FreeVirtualMemory, BootDevice, TotalVisibleMemorySize FROM Win32_OperatingSystem"),
		{ _T("Caption"), _T("Version"), _T("BuildNumber"), _T("OSArchitecture"), _T("InstallDate"), _T("LastBootUpTime"), _T("WindowsDirectory"), _T("FreePhysicalMemory"), _T("TotalVirtualMemorySize"), _T("FreeVirtualMemory"), _T("BootDevice"), _T("TotalVisibleMemorySize") });
	const std::vector<CString> osRow = osRows.empty() ? std::vector<CString>() : osRows.front();
	const auto diskRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Model, Size, MediaType, InterfaceType, SerialNumber, FirmwareRevision, Status, PNPDeviceID, DeviceID FROM Win32_DiskDrive"),
		{ _T("Model"), _T("Size"), _T("MediaType"), _T("InterfaceType"), _T("SerialNumber"), _T("FirmwareRevision"), _T("Status"), _T("PNPDeviceID"), _T("DeviceID") });
	const auto smartRows = QueryWmiRows(
		_T("ROOT\\WMI"),
		_T("SELECT InstanceName, PredictFailure, Reason FROM MSStorageDriver_FailurePredictStatus"),
		{ _T("InstanceName"), _T("PredictFailure"), _T("Reason") });
	const auto smartDataEntries = QuerySmartWmiEntries();
	std::map<int, CString> pnpByPhysicalDriveId;
	for (const auto& row : diskRows)
	{
		if (row.size() < 9)
		{
			continue;
		}
		const int driveId = ExtractPhysicalDriveId(row[8]);
		if (driveId >= 0 && HasValue(row[7]))
		{
			pnpByPhysicalDriveId[driveId] = row[7];
		}
	}

	AddSystemInfoRow(_T("主板型号"), boardModel);
	AddSystemInfoRow(_T("CPU型号"), cpuModel);
	AddSystemInfoRow(_T("GPU型号"), gpuModel);
	AddSystemInfoRow(_T("显示器信息"), monitorInfo);
	AddSystemInfoRow(_T("内存信息"), memoryInfo);
	AddSystemInfoRow(_T("SSD信息"), ssdInfo);
	AddSystemInfoRow(_T("BIOS版本"), biosVersion);
	AddSystemInfoRow(_T("EC版本"), ecVersion);
	AddSystemInfoRow(_T("主板SN"), boardSn);
	AddSystemInfoRow(_T("系统SN"), systemSn);
	AddSystemInfoRow(_T("UUID"), uuid);
	AddSystemInfoRow(_T("有线网卡"), wiredMac);
	AddSystemInfoRow(_T("无线网卡"), wirelessMac);
	AddSystemInfoRow(_T("蓝牙地址"), bluetoothAddress);
	AddSystemInfoRow(_T("TPM型号"), tpmModel);

	auto addSystemStatusRow = [this](const CString& item, const CString& value)
		{
			m_systemStatusRows.push_back({ item, HasValue(value) ? NormalizeMultilineValue(value) : _T("N/A") });
		};
	addSystemStatusRow(_T("Windows版本"), ResolveWindowsVersionText(osRow));
	addSystemStatusRow(_T("系统架构"), osRow.size() > 3 ? osRow[3] : _T(""));
	addSystemStatusRow(_T("安装日期"), osRow.size() > 4 ? FormatWmiDateTime(osRow[4]) : _T(""));
	addSystemStatusRow(_T("最近启动时间"), osRow.size() > 5 ? FormatWmiDateTime(osRow[5]) : _T(""));
	addSystemStatusRow(_T("运行时长"), osRow.size() > 5 ? FormatUptime(osRow[5]) : _T(""));
	addSystemStatusRow(_T("激活状态"), ResolveWindowsActivationStatus());
	addSystemStatusRow(_T("系统语言"), ResolveLocaleText());
	addSystemStatusRow(_T("时区"), ResolveTimeZoneText());
	addSystemStatusRow(_T("Windows目录"), osRow.size() > 6 ? osRow[6] : _T(""));
	addSystemStatusRow(_T("Secure Boot"), ResolveSecureBootStatus());
	addSystemStatusRow(_T("BitLocker"), ResolveBitLockerStatus());
	addSystemStatusRow(_T("计算机名"), csRow.size() > 0 ? csRow[0] : _T(""));
	addSystemStatusRow(_T("域/工作组"), csRow.size() > 1 ? csRow[1] : _T(""));
	addSystemStatusRow(_T("登录用户"), csRow.size() > 5 ? csRow[5] : _T(""));
	addSystemStatusRow(_T("系统类型"), csRow.size() > 4 ? csRow[4] : _T(""));
	{
		CString totalMem;
		if (csRow.size() > 3 && HasValue(csRow[3]))
		{
			const double totalGB = _ttof(csRow[3]) / (1024.0 * 1024.0 * 1024.0);
			totalMem.Format(_T("%.1f GB"), totalGB);
		}
		addSystemStatusRow(_T("总物理内存"), totalMem);
	}
	{
		CString availMem;
		if (osRow.size() > 7 && HasValue(osRow[7]))
		{
			const double availGB = _ttof(osRow[7]) / (1024.0 * 1024.0);
			availMem.Format(_T("%.1f GB"), availGB);
		}
		addSystemStatusRow(_T("可用物理内存"), availMem);
	}
	{
		CString totalVM;
		if (osRow.size() > 8 && HasValue(osRow[8]))
		{
			const double vmGB = _ttof(osRow[8]) / (1024.0 * 1024.0);
			totalVM.Format(_T("%.1f GB"), vmGB);
		}
		addSystemStatusRow(_T("虚拟内存总计"), totalVM);
	}
	{
		CString freeVM;
		if (osRow.size() > 9 && HasValue(osRow[9]))
		{
			const double freeVMGB = _ttof(osRow[9]) / (1024.0 * 1024.0);
			freeVM.Format(_T("%.1f GB"), freeVMGB);
		}
		addSystemStatusRow(_T("可用虚拟内存"), freeVM);
	}
	{
		CString pageFile;
		if (osRow.size() > 11 && HasValue(osRow[11]))
		{
			const double pfGB = _ttof(osRow[11]) / 1024.0;
			const double totalPhysGB = (csRow.size() > 3 && HasValue(csRow[3])) ? _ttof(csRow[3]) / (1024.0 * 1024.0 * 1024.0) : 0.0;
			pageFile.Format(_T("%.1f GB (物理 %.1f GB)"), pfGB, totalPhysGB);
		}
		addSystemStatusRow(_T("页面文件"), pageFile);
	}
	addSystemStatusRow(_T("引导设备"), osRow.size() > 10 ? osRow[10] : _T(""));
	addSystemStatusRow(_T("Hyper-V"), csRow.size() > 2 ? (HasValue(csRow[2]) && csRow[2].CompareNoCase(_T("TRUE")) == 0 ? _T("已启用") : _T("未启用")) : _T(""));
	addSystemStatusRow(_T("HAL版本"), ReadRegistryStringValue(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DESCRIPTION\\System"), _T("SystemBiosVersion")));
	addSystemStatusRow(_T("电源计划"), ResolveActivePowerPlan());

	auto addAcpiRow = [this](const CString& item, const CString& value)
		{
			m_acpiRows.push_back({ item, HasValue(value) ? NormalizeMultilineValue(value) : _T("N/A") });
		};
	const auto cpuRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Manufacturer, Name, SocketDesignation, NumberOfCores, NumberOfLogicalProcessors, MaxClockSpeed, CurrentClockSpeed, ProcessorId, Revision, L2CacheSize, L3CacheSize, VirtualizationFirmwareEnabled, SecondLevelAddressTranslationExtensions, VMMonitorModeExtensions FROM Win32_Processor"),
		{ _T("Manufacturer"), _T("Name"), _T("SocketDesignation"), _T("NumberOfCores"), _T("NumberOfLogicalProcessors"), _T("MaxClockSpeed"), _T("CurrentClockSpeed"), _T("ProcessorId"), _T("Revision"), _T("L2CacheSize"), _T("L3CacheSize"), _T("VirtualizationFirmwareEnabled"), _T("SecondLevelAddressTranslationExtensions"), _T("VMMonitorModeExtensions") });
	const std::vector<CString> cpuRow = cpuRows.empty() ? std::vector<CString>() : cpuRows.front();
	const auto biosRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Manufacturer, SMBIOSBIOSVersion, Version, ReleaseDate, SerialNumber FROM Win32_BIOS"),
		{ _T("Manufacturer"), _T("SMBIOSBIOSVersion"), _T("Version"), _T("ReleaseDate"), _T("SerialNumber") });
	const std::vector<CString> biosRow = biosRows.empty() ? std::vector<CString>() : biosRows.front();
	const auto boardRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Manufacturer, Product, Version, SerialNumber FROM Win32_BaseBoard"),
		{ _T("Manufacturer"), _T("Product"), _T("Version"), _T("SerialNumber") });
	const std::vector<CString> boardRow = boardRows.empty() ? std::vector<CString>() : boardRows.front();
	const auto productRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Vendor, Name, Version, UUID FROM Win32_ComputerSystemProduct"),
		{ _T("Vendor"), _T("Name"), _T("Version"), _T("UUID") });
	const std::vector<CString> productRow = productRows.empty() ? std::vector<CString>() : productRows.front();
	const auto enclosureRows = QueryWmiRows(
		_T("ROOT\\CIMV2"),
		_T("SELECT Manufacturer, ChassisTypes, SMBIOSAssetTag FROM Win32_SystemEnclosure"),
		{ _T("Manufacturer"), _T("ChassisTypes"), _T("SMBIOSAssetTag") });
	const std::vector<CString> enclosureRow = enclosureRows.empty() ? std::vector<CString>() : enclosureRows.front();

	addAcpiRow(_T("CPU制造商"), HasValue(CpuidVendor()) ? CpuidVendor() : (cpuRow.size() > 0 ? cpuRow[0] : _T("")));
	addAcpiRow(_T("CPU名称"), HasValue(CpuidBrandString()) ? CpuidBrandString() : (cpuRow.size() > 1 ? cpuRow[1] : _T("")));
	addAcpiRow(_T("CPU插槽"), cpuRow.size() > 2 ? cpuRow[2] : _T(""));
	addAcpiRow(_T("核心/线程"), [&cpuRow]() {
		CString text;
		text.Format(_T("%s / %s"),
			cpuRow.size() > 3 ? cpuRow[3].GetString() : _T("N/A"),
			cpuRow.size() > 4 ? cpuRow[4].GetString() : _T("N/A"));
		return text;
		}());
	addAcpiRow(_T("最大频率"), cpuRow.size() > 5 && HasValue(cpuRow[5]) ? cpuRow[5] + _T(" MHz") : _T(""));
	addAcpiRow(_T("Processor ID"), cpuRow.size() > 7 ? cpuRow[7] : _T(""));
	addAcpiRow(_T("WMI修订版本"), cpuRow.size() > 8 ? cpuRow[8] : _T(""));
	addAcpiRow(_T("CPUID签名"), CpuidSignatureText());
	addAcpiRow(_T("虚拟化支持"), cpuRow.size() > 11 ? cpuRow[11] : _T(""));
	addAcpiRow(_T("SLAT支持"), cpuRow.size() > 12 ? cpuRow[12] : _T(""));
	addAcpiRow(_T("VM Monitor扩展"), cpuRow.size() > 13 ? cpuRow[13] : _T(""));
	addAcpiRow(_T("CPU指令集"), CpuidFeatureText());
	addAcpiRow(_T("GPU信息"), HasValue(gpuDetails) ? gpuDetails : gpuModel);
	if (!memoryDetailLines.empty())
	{
		addAcpiRow(_T("内存总览"), memoryDetailLines.front());
		for (size_t i = 1; i < memoryDetailLines.size(); ++i)
		{
			CString label;
			label.Format(_T("内存[%u]"), static_cast<unsigned>(i));
			addAcpiRow(label, memoryDetailLines[i]);
		}
	}
	else
	{
		addAcpiRow(_T("内存信息"), memoryInfo);
	}
	addAcpiRow(_T("BIOS厂商"), biosRow.size() > 0 ? biosRow[0] : _T(""));
	addAcpiRow(_T("SMBIOS版本"), biosRow.size() > 1 ? biosRow[1] : _T(""));
	addAcpiRow(_T("BIOS版本"), biosRow.size() > 2 ? biosRow[2] : _T(""));
	addAcpiRow(_T("BIOS发布日期"), biosRow.size() > 3 ? FormatWmiDateTime(biosRow[3]) : _T(""));
	addAcpiRow(_T("BIOS序列号"), biosRow.size() > 4 ? biosRow[4] : _T(""));
	addAcpiRow(_T("主板厂商"), boardRow.size() > 0 ? boardRow[0] : _T(""));
	addAcpiRow(_T("主板产品"), boardRow.size() > 1 ? boardRow[1] : _T(""));
	addAcpiRow(_T("主板版本"), boardRow.size() > 2 ? boardRow[2] : _T(""));
	addAcpiRow(_T("主板序列号"), boardRow.size() > 3 ? boardRow[3] : _T(""));
	addAcpiRow(_T("系统产品厂商"), productRow.size() > 0 ? productRow[0] : _T(""));
	addAcpiRow(_T("系统产品名称"), productRow.size() > 1 ? productRow[1] : _T(""));
	addAcpiRow(_T("系统产品版本"), productRow.size() > 2 ? productRow[2] : _T(""));
	addAcpiRow(_T("系统UUID"), productRow.size() > 3 ? productRow[3] : _T(""));
	addAcpiRow(_T("机箱厂商"), enclosureRow.size() > 0 ? enclosureRow[0] : _T(""));
	addAcpiRow(_T("机箱类型"), enclosureRow.size() > 1 ? enclosureRow[1] : _T(""));
	addAcpiRow(_T("资产标签"), enclosureRow.size() > 2 ? enclosureRow[2] : _T(""));

	// 启动时先加载基础信息，SSD 重采集延后到用户进入 SSD 页再执行，避免首屏卡顿感。
	if (!loadSsdDetails)
	{
		m_ssdLoaded = false;
		m_ssdLoading = false;
		m_ssdDiskRows.clear();
		m_ssdTabTitles.clear();
		m_ssdDiskRows.push_back({ { _T("状态"), _T("SSD信息将在进入该页面时加载") } });
		m_ssdTabTitles.push_back(_T("待加载"));
		m_ssdRows = m_ssdDiskRows.front();
		if (hasWindow)
		{
			RefreshSsdTabs();
			UpdateSsdControlLayout();
		}

		m_loading = false;
		if (hasWindow)
		{
			Invalidate();
		}
		return;
	}

	// 优先使用完整 CAtaSmart 路径（含桥接芯片分支）生成 SSD 多 Tab 数据。
	{
		CAtaSmart ataSmart;
		BOOL flagChangeDisk = FALSE;
#ifdef _DEBUG
		SetDebugMode(1);
#else
		SetDebugMode(0);
#endif
		int countAfterWmiOn = 0;
		int countAfterWmiOff = 0;
		// 先走 CAtaSmart 的 WMI-on 路径（你日志里能识别出 NVMe 控制器与盘型）。
		ataSmart.Init(TRUE, TRUE, &flagChangeDisk, FALSE, FALSE, FALSE, TRUE, FALSE);
		countAfterWmiOn = static_cast<int>(ataSmart.vars.GetCount());
		if (ataSmart.vars.GetCount() <= 0)
		{
			// 若仍为 0，再尝试 WMI-off 直连路径。
			ataSmart.Init(FALSE, TRUE, &flagChangeDisk, FALSE, FALSE, FALSE, TRUE, FALSE);
			countAfterWmiOff = static_cast<int>(ataSmart.vars.GetCount());
		}

		std::vector<std::vector<InfoRow>> ataDetectedSsdDisks;
		std::vector<CString> ataDetectedSsdTitles;
		std::vector<std::vector<InfoRow>> ataAllDisks;
		std::vector<CString> ataAllTitles;

		for (int i = 0; i < ataSmart.vars.GetCount(); ++i)
		{
			ataSmart.UpdateSmartInfo(static_cast<DWORD>(i));
			const auto& info = ataSmart.vars[i];
			const DirectSmartData directSmart = ReadDirectSmartData(info.PhysicalDriveId);

			CString smartTemperature = _T("N/A");
			if (info.Temperature > -1000 && info.Temperature < 200)
			{
				smartTemperature.Format(_T("%d °C"), info.Temperature);
			}

			CString smartPowerOnHours = _T("N/A");
			const int powerOnHours = info.MeasuredPowerOnHours >= 0 ? info.MeasuredPowerOnHours : info.DetectedPowerOnHours;
			if (powerOnHours >= 0)
			{
				smartPowerOnHours.Format(_T("%d 小时"), powerOnHours);
			}

			CString smartPowerOnCount = _T("N/A");
			if (info.PowerOnCount > 0)
			{
				smartPowerOnCount.Format(_T("%u 次"), info.PowerOnCount);
			}

			CString smartLife = _T("N/A");
			if (info.Life >= 0 && info.Life <= 100)
			{
				smartLife.Format(_T("%d %%"), info.Life);
			}
			if (smartTemperature == _T("N/A") && HasValue(directSmart.temperature))
			{
				smartTemperature = directSmart.temperature;
			}
			if (smartPowerOnHours == _T("N/A") && HasValue(directSmart.powerOnHours))
			{
				smartPowerOnHours = directSmart.powerOnHours;
			}
			if (smartPowerOnCount == _T("N/A") && HasValue(directSmart.powerOnCount))
			{
				smartPowerOnCount = directSmart.powerOnCount;
			}
			if (smartLife == _T("N/A") && HasValue(directSmart.life))
			{
				smartLife = directSmart.life;
			}

			ULONGLONG sizeBytes = 0;
			if (info.NumberOfSectors > 0)
			{
				const DWORD logicalSectorSize = info.LogicalSectorSize > 0 ? info.LogicalSectorSize : 512;
				sizeBytes = info.NumberOfSectors * static_cast<ULONGLONG>(logicalSectorSize);
			}
			CString sizeText = _T("");
			if (sizeBytes > 0)
			{
				sizeText = FormatBytesToGB(sizeBytes);
			}

			CString sourceText;
			sourceText.Format(_T("CAtaSmart (%s)"), HasValue(info.CommandTypeString) ? static_cast<LPCTSTR>(info.CommandTypeString) : _T("unknown"));
			if (directSmart.hasValue)
			{
				sourceText += _T(" + 直连补全");
			}

			CString transferMode = HasDisplayValue(info.CurrentTransferMode) ? info.CurrentTransferMode : _T("N/A");
			CString maxTransferMode = HasDisplayValue(info.MaxTransferMode) ? info.MaxTransferMode : _T("N/A");
			FillNvmeTransferModeFallback(info, transferMode, maxTransferMode);

			CString smartPredict = info.DiskStatus == CAtaSmart::DISK_STATUS_BAD ? _T("是") : _T("否");
			CString smartReason = _T("N/A");
			if (directSmart.hasValue)
			{
				smartPredict = directSmart.predictFailure;
				smartReason = directSmart.reasonCode;
			}

			CString hostReadsText = AtaSmartHostRwText(info.HostReads, info.HostReadsWritesUnit);
			CString hostWritesText = AtaSmartHostRwText(info.HostWrites, info.HostReadsWritesUnit);
			if (!HasValue(hostReadsText) && HasValue(directSmart.hostReads))
			{
				hostReadsText = directSmart.hostReads;
			}
			if (!HasValue(hostWritesText) && HasValue(directSmart.hostWrites))
			{
				hostWritesText = directSmart.hostWrites;
			}

			// 对 NVMe：从 CAtaSmart 已填充的 SmartReadData 再解析一层主机读写统计，覆盖直连失败场景。
			if (info.IsNVMe)
			{
				if (!HasValue(hostReadsText))
				{
					const long double readUnits = ParseLeInteger128FromBuffer(info.SmartReadData, 512, 32, 16);
					hostReadsText = FormatNvmeDataUnits(readUnits);
					if (!HasValue(hostReadsText))
					{
						const long double readCommands = ParseLeInteger128FromBuffer(info.SmartReadData, 512, 64, 16);
						if (readCommands > 0.0L)
						{
							hostReadsText.Format(_T("%.0Lf 次命令"), readCommands);
						}
					}
				}

				if (!HasValue(hostWritesText))
				{
					const long double writeUnits = ParseLeInteger128FromBuffer(info.SmartReadData, 512, 48, 16);
					hostWritesText = FormatNvmeDataUnits(writeUnits);
					if (!HasValue(hostWritesText))
					{
						const long double writeCommands = ParseLeInteger128FromBuffer(info.SmartReadData, 512, 80, 16);
						if (writeCommands > 0.0L)
						{
							hostWritesText.Format(_T("%.0Lf 次命令"), writeCommands);
						}
		}
	}
}

			const CString deviceIdText = ResolveAtaSmartDeviceId(info, diskRows, pnpByPhysicalDriveId);

			std::vector<InfoRow> diskDetailRows;
			diskDetailRows.push_back({ _T("采集方式"), sourceText });
			diskDetailRows.push_back({ _T("磁盘概览"), BuildDiskSummary(info.Model, sizeText, info.Interface) });
			diskDetailRows.push_back({ _T("型号"), HasValue(info.Model) ? info.Model : _T("N/A") });
			diskDetailRows.push_back({ _T("固件版本"), HasValue(info.FirmwareRev) ? info.FirmwareRev : _T("N/A") });
			diskDetailRows.push_back({ _T("序列号"), HasValue(info.SerialNumber) ? info.SerialNumber : _T("N/A") });
			diskDetailRows.push_back({ _T("接口类型"), HasValue(info.Interface) ? info.Interface : _T("N/A") });
			diskDetailRows.push_back({ _T("当前传输模式"), transferMode });
			diskDetailRows.push_back({ _T("最大传输模式"), maxTransferMode });
			diskDetailRows.push_back({ _T("介质类型"), info.IsSsd ? _T("SSD") : _T("HDD/其他") });
			diskDetailRows.push_back({ _T("磁盘状态"), AtaSmartDiskStatusText(info.DiskStatus) });
			diskDetailRows.push_back({ _T("SMART预测故障"), smartPredict });
			diskDetailRows.push_back({ _T("SMART原因码"), smartReason });
			diskDetailRows.push_back({ _T("SMART温度"), smartTemperature });
			diskDetailRows.push_back({ _T("SMART通电时长"), smartPowerOnHours });
			diskDetailRows.push_back({ _T("SMART通电次数"), smartPowerOnCount });
			diskDetailRows.push_back({ _T("SMART寿命(估算)"), smartLife });
			diskDetailRows.push_back({ _T("主机读取总量"), hostReadsText });
			diskDetailRows.push_back({ _T("主机写入总量"), hostWritesText });
			diskDetailRows.push_back({ _T("设备标识"), deviceIdText });

			const CString tabTitle = BuildAtaSmartTabTitle(info, i);
			ataAllDisks.push_back(diskDetailRows);
			ataAllTitles.push_back(tabTitle);
			if (IsAtaSmartSsdCandidate(info))
			{
				ataDetectedSsdDisks.push_back(diskDetailRows);
				ataDetectedSsdTitles.push_back(tabTitle);
			}
		}

		// 若 SSD 识别结果过少（例如桥接芯片返回介质标识不完整），回退展示所有已识别磁盘，避免只出现单 Tab。
		if (!ataDetectedSsdDisks.empty() &&
			(ataDetectedSsdDisks.size() == ataAllDisks.size() || ataDetectedSsdDisks.size() >= 2))
		{
			m_ssdDiskRows = ataDetectedSsdDisks;
			m_ssdTabTitles = ataDetectedSsdTitles;
		}
		else if (!ataAllDisks.empty())
		{
			m_ssdDiskRows = ataAllDisks;
			m_ssdTabTitles = ataAllTitles;
		}

		if (!m_ssdDiskRows.empty())
		{
			m_ssdLoaded = true;
			m_ssdLoading = false;
			m_ssdRows = m_ssdDiskRows.front();
			if (hasWindow)
			{
				RefreshSsdTabs();
				UpdateSsdControlLayout();
			}
			m_loading = false;
			if (hasWindow)
			{
				Invalidate();
			}
			return;
		}

		// 当前阶段强制只走 CAtaSmart；若未枚举成功，直接显示诊断信息而非回退到 WMI 路径。
		const bool allowLegacyWmiFallback = false;
		if (!allowLegacyWmiFallback)
		{
			InfoRow row1;
			row1.item = _T("采集方式");
			row1.value = _T("CAtaSmart（直连）");

			InfoRow row2;
			row2.item = _T("状态");
			row2.value = _T("未通过 CAtaSmart 枚举到可展示磁盘");

			InfoRow row3;
			row3.item = _T("建议");
			row3.value = _T("请以管理员运行，并确认未被磁盘加密/安全软件拦截设备直通");

			std::vector<InfoRow> diagRows = { row1, row2, row3 };
#ifdef _DEBUG
			InfoRow row4;
			row4.item = _T("CAtaSmart枚举数");
			row4.value.Format(_T("%d"), ataSmart.vars.GetCount());
			diagRows.push_back(row4);

			InfoRow row5;
			row5.item = _T("Win32_DiskDrive数量");
			row5.value.Format(_T("%u"), static_cast<unsigned>(diskRows.size()));
			diagRows.push_back(row5);

			InfoRow row6;
			row6.item = _T("调试日志");
			row6.value = _T("请查看程序目录下 SystemInspector.log");
			diagRows.push_back(row6);

			InfoRow row7;
			row7.item = _T("Init(WMI=on) 枚举数");
			row7.value.Format(_T("%d"), countAfterWmiOn);
			diagRows.push_back(row7);

			InfoRow row8;
			row8.item = _T("Init(WMI=off) 枚举数");
			row8.value.Format(_T("%d"), countAfterWmiOff);
			diagRows.push_back(row8);
#endif
			m_ssdDiskRows.push_back(diagRows);
			m_ssdTabTitles.push_back(_T("AtaSmart诊断"));
			m_ssdLoaded = true;
			m_ssdLoading = false;
			m_ssdRows = m_ssdDiskRows.front();
			if (hasWindow)
			{
				RefreshSsdTabs();
				UpdateSsdControlLayout();
			}
			m_loading = false;
			if (hasWindow)
			{
				Invalidate();
			}
			return;
		}
	}

	std::vector<std::vector<InfoRow>> detectedSsdDisks;
	std::vector<CString> detectedSsdTitles;
	std::vector<std::vector<InfoRow>> fallbackDisks;
	std::vector<CString> fallbackTitles;
	for (size_t i = 0; i < diskRows.size(); ++i)
	{
		const auto& row = diskRows[i];
		if (row.size() < 9)
		{
			continue;
		}

		const CString model = row[0];
		const CString size = row[1];
		const CString mediaType = row[2];
		const CString interfaceType = row[3];
		const CString serialNumber = row[4];
		const CString firmware = row[5];
		const CString diskStatus = row[6];
		const CString pnpDeviceId = row[7];
		const CString devicePath = row[8];
		const int physicalDriveId = ExtractPhysicalDriveId(devicePath);
		const DirectSmartData directSmart = ReadDirectSmartData(physicalDriveId);

		const CString normalizedModel = NormalizeHardwareKey(model);
		const CString normalizedPnp = NormalizeHardwareKey(pnpDeviceId);
		CString smartPredict = _T("未知");
		CString smartReason = _T("N/A");
		const SmartWmiEntry* matchedSmartEntry = nullptr;
		for (const auto& smartRow : smartRows)
		{
			if (smartRow.size() < 3)
			{
				continue;
			}

			const CString normalizedInstance = NormalizeHardwareKey(smartRow[0]);
			const bool pnpMatched = !normalizedPnp.IsEmpty() && normalizedInstance.Find(normalizedPnp) >= 0;
			const bool modelMatched = !normalizedModel.IsEmpty() && normalizedInstance.Find(normalizedModel) >= 0;
			if (!pnpMatched && !modelMatched)
			{
				continue;
			}

			smartPredict = smartRow[1].CompareNoCase(_T("True")) == 0 ? _T("是") : _T("否");
			smartReason = HasValue(smartRow[2]) ? smartRow[2] : _T("0");
			break;
		}
		for (const auto& smartEntry : smartDataEntries)
		{
			const CString normalizedInstance = NormalizeHardwareKey(smartEntry.instanceName);
			const bool pnpMatched = !normalizedPnp.IsEmpty() && normalizedInstance.Find(normalizedPnp) >= 0;
			const bool modelMatched = !normalizedModel.IsEmpty() && normalizedInstance.Find(normalizedModel) >= 0;
			if (pnpMatched || modelMatched)
			{
				matchedSmartEntry = &smartEntry;
				break;
			}
		}

		CString smartTemperature = directSmart.temperature;
		CString smartPowerOnHours = directSmart.powerOnHours;
		CString smartPowerOnCount = directSmart.powerOnCount;
		CString smartLife = directSmart.life;
		CString sourceText = directSmart.hasValue ? directSmart.source : _T("CrystalDiskInfo 同路径（WMI SMART回退）");
		if (directSmart.hasValue)
		{
			smartPredict = directSmart.predictFailure;
			smartReason = directSmart.reasonCode;
		}
		else if (matchedSmartEntry != nullptr)
		{
			unsigned long long rawValue = 0;
			unsigned char currentValue = 0;
			if (TryGetSmartRawValue(matchedSmartEntry->vendorSpecific, 0xC2, rawValue, currentValue) ||
				TryGetSmartRawValue(matchedSmartEntry->vendorSpecific, 0xBE, rawValue, currentValue))
			{
				smartTemperature.Format(_T("%llu °C"), rawValue & 0xFFULL);
			}
			if (TryGetSmartRawValue(matchedSmartEntry->vendorSpecific, 0x09, rawValue, currentValue))
			{
				smartPowerOnHours.Format(_T("%llu 小时"), rawValue);
			}
			if (TryGetSmartRawValue(matchedSmartEntry->vendorSpecific, 0x0C, rawValue, currentValue))
			{
				smartPowerOnCount.Format(_T("%llu 次"), rawValue);
			}
			if (TryGetSmartRawValue(matchedSmartEntry->vendorSpecific, 0xE7, rawValue, currentValue) ||
				TryGetSmartRawValue(matchedSmartEntry->vendorSpecific, 0xE8, rawValue, currentValue))
			{
				smartLife.Format(_T("%u %%"), static_cast<unsigned>(currentValue));
			}
		}

		std::vector<InfoRow> diskDetailRows;
		diskDetailRows.push_back({ _T("采集方式"), sourceText });
		diskDetailRows.push_back({ _T("磁盘概览"), BuildDiskSummary(model, size, interfaceType) });
		diskDetailRows.push_back({ _T("型号"), HasValue(model) ? model : _T("N/A") });
		diskDetailRows.push_back({ _T("固件版本"), HasValue(firmware) ? firmware : _T("N/A") });
		diskDetailRows.push_back({ _T("序列号"), HasValue(serialNumber) ? serialNumber : _T("N/A") });
		diskDetailRows.push_back({ _T("接口类型"), HasValue(interfaceType) ? interfaceType : _T("N/A") });
		diskDetailRows.push_back({ _T("介质类型"), HasValue(mediaType) ? mediaType : _T("N/A") });
		diskDetailRows.push_back({ _T("磁盘状态"), HasValue(diskStatus) ? diskStatus : _T("N/A") });
		diskDetailRows.push_back({ _T("SMART预测故障"), smartPredict });
		diskDetailRows.push_back({ _T("SMART原因码"), smartReason });
		diskDetailRows.push_back({ _T("SMART温度"), smartTemperature });
		diskDetailRows.push_back({ _T("SMART通电时长"), smartPowerOnHours });
		diskDetailRows.push_back({ _T("SMART通电次数"), smartPowerOnCount });
		diskDetailRows.push_back({ _T("SMART寿命(估算)"), smartLife });
		diskDetailRows.push_back({ _T("设备标识"), HasValue(pnpDeviceId) ? pnpDeviceId : _T("N/A") });

		CString tabTitle;
		tabTitle.Format(_T("SSD %u"), static_cast<unsigned>(i + 1));
		if (HasValue(model))
		{
			tabTitle = model;
			if (tabTitle.GetLength() > 28)
			{
				tabTitle = tabTitle.Left(28) + _T("...");
			}
		}

		fallbackDisks.push_back(diskDetailRows);
		fallbackTitles.push_back(tabTitle);
		if (IsLikelySsdDevice(model, mediaType, interfaceType))
		{
			detectedSsdDisks.push_back(diskDetailRows);
			detectedSsdTitles.push_back(tabTitle);
		}
	}

	if (!detectedSsdDisks.empty())
	{
		m_ssdDiskRows = detectedSsdDisks;
		m_ssdTabTitles = detectedSsdTitles;
	}
	else
	{
		m_ssdDiskRows = fallbackDisks;
		m_ssdTabTitles = fallbackTitles;
	}

	if (m_ssdDiskRows.empty())
	{
		InfoRow emptyRow;
		emptyRow.item = _T("SSD状态");
		emptyRow.value = _T("未检测到可展示的磁盘信息");
		m_ssdDiskRows.push_back({ emptyRow });
		m_ssdTabTitles.push_back(_T("无SSD"));
	}
	m_ssdLoaded = true;
	m_ssdLoading = false;
	m_ssdRows = m_ssdDiskRows.front();
	if (hasWindow)
	{
		RefreshSsdTabs();
		UpdateSsdControlLayout();
	}

	m_loading = false;
	if (hasWindow)
	{
		Invalidate();
	}
}

// 关闭默认背景擦除，配合双缓冲降低闪烁。
BOOL CMFCApplication1Dlg::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}

// 按需创建字体与背景画刷资源（仅初始化一次）。
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
	if (m_startupListFont.GetSafeHandle() == nullptr)
	{
		m_startupListFont.CreatePointFont(110, _T("Microsoft YaHei UI"));
	}
	if (m_uiBackgroundBrush.GetSafeHandle() == nullptr)
	{
		m_uiBackgroundBrush.CreateSolidBrush(UiBackground);
	}
}

// 绘制圆角卡片背景块。
void CMFCApplication1Dlg::DrawRoundedCard(CDC& dc, const CRect& rect, COLORREF fillColor, int radius, COLORREF borderColor)
{
	CPen pen(PS_SOLID, 1, borderColor == CLR_NONE ? fillColor : borderColor);
	CBrush brush(fillColor);
	CPen* oldPen = dc.SelectObject(&pen);
	CBrush* oldBrush = dc.SelectObject(&brush);
	dc.RoundRect(rect, CPoint(radius, radius));
	dc.SelectObject(oldPen);
	dc.SelectObject(oldBrush);
}

// 绘制主内容区域（系统信息页、SSD信息页或系统设置页）。
void CMFCApplication1Dlg::DrawSystemInformation(CDC& dc, const CRect& clientRect)
{
	EnsureUiFonts();

	// 使用内存 DC 双缓冲绘制，减少重绘闪烁。
	CDC memDc;
	memDc.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* oldBitmap = memDc.SelectObject(&bitmap);

	memDc.FillSolidRect(clientRect, UiBackground);
	memDc.SetBkMode(TRANSPARENT);

	DrawRoundedCard(memDc, m_sideRect, UiSurfaceAlt, 16, UiBorder);
	DrawRoundedCard(memDc, m_contentRect, UiSurface, 16, UiBorder);

	const COLORREF selectedColor = UiAccentSoft;
	const COLORREF unselectedColor = UiSurfaceAlt;

	DrawRoundedCard(memDc, m_infoMenuRect, m_activePage == PAGE_SYSTEM_INFO ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_statusMenuRect, m_activePage == PAGE_SYSTEM_STATUS ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_exceptionMenuRect, m_activePage == PAGE_SYSTEM_EXCEPTION ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_startupMenuRect, m_activePage == PAGE_STARTUP_ITEMS ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_acpiMenuRect, m_activePage == PAGE_ACPI_INFO ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_settingsMenuRect, m_activePage == PAGE_SYSTEM_SETTINGS ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_ssdMenuRect, m_activePage == PAGE_SSD_INFO ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_screenMenuRect, m_activePage == PAGE_SCREEN_INFO ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_batteryLogMenuRect, m_activePage == PAGE_BATTERY_LOG ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_powerLogMenuRect, m_activePage == PAGE_POWER_LOG ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_utilityMenuRect, m_activePage == PAGE_UTILITY_TOOLS ? selectedColor : unselectedColor, 10);

	if (m_activePage == PAGE_SYSTEM_INFO) DrawAccentStrip(memDc, m_infoMenuRect, UiAccent);
	if (m_activePage == PAGE_SYSTEM_STATUS) DrawAccentStrip(memDc, m_statusMenuRect, UiAccent);
	if (m_activePage == PAGE_SYSTEM_EXCEPTION) DrawAccentStrip(memDc, m_exceptionMenuRect, UiAccent);
	if (m_activePage == PAGE_STARTUP_ITEMS) DrawAccentStrip(memDc, m_startupMenuRect, UiAccent);
	if (m_activePage == PAGE_ACPI_INFO) DrawAccentStrip(memDc, m_acpiMenuRect, UiAccent);
	if (m_activePage == PAGE_SYSTEM_SETTINGS) DrawAccentStrip(memDc, m_settingsMenuRect, UiAccent);
	if (m_activePage == PAGE_SSD_INFO) DrawAccentStrip(memDc, m_ssdMenuRect, UiAccent);
	if (m_activePage == PAGE_SCREEN_INFO) DrawAccentStrip(memDc, m_screenMenuRect, UiAccent);
	if (m_activePage == PAGE_BATTERY_LOG) DrawAccentStrip(memDc, m_batteryLogMenuRect, UiAccent);
	if (m_activePage == PAGE_POWER_LOG) DrawAccentStrip(memDc, m_powerLogMenuRect, UiAccent);
	if (m_activePage == PAGE_UTILITY_TOOLS) DrawAccentStrip(memDc, m_utilityMenuRect, UiAccent);

	memDc.SelectObject(&m_subtitleFont);
	memDc.SetTextColor(UiSecondaryText);
	CRect appLabelRect(m_sideRect.left + 16, m_sideRect.top + 14, m_sideRect.right - 16, m_sideRect.top + 38);
	memDc.DrawText(_T("SystemInspector"), appLabelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	memDc.SelectObject(&m_menuFont);
	CRect menuTextRect = m_infoMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_SYSTEM_INFO ? UiText : UiSecondaryText);
	memDc.DrawText(_T("系统信息"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_statusMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_SYSTEM_STATUS ? UiText : UiSecondaryText);
	memDc.DrawText(_T("系统状态"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_exceptionMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_SYSTEM_EXCEPTION ? UiText : UiSecondaryText);
	memDc.DrawText(_T("系统异常"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_startupMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_STARTUP_ITEMS ? UiText : UiSecondaryText);
	memDc.DrawText(_T("开机自启"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_acpiMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_ACPI_INFO ? UiText : UiSecondaryText);
	memDc.DrawText(_T("驱动详情"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_settingsMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_SYSTEM_SETTINGS ? UiText : UiSecondaryText);
	memDc.DrawText(_T("系统设置"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_ssdMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_SSD_INFO ? UiText : UiSecondaryText);
	memDc.DrawText(_T("SSD信息"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_screenMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_SCREEN_INFO ? UiText : UiSecondaryText);
	memDc.DrawText(_T("屏幕详情"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_batteryLogMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_BATTERY_LOG ? UiText : UiSecondaryText);
	memDc.DrawText(_T("电池日志"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_powerLogMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_POWER_LOG ? UiText : UiSecondaryText);
	memDc.DrawText(_T("电源日志"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	menuTextRect = m_utilityMenuRect;
	menuTextRect.left += 28;
	menuTextRect.right -= 12;
	memDc.SetTextColor(m_activePage == PAGE_UTILITY_TOOLS ? UiText : UiSecondaryText);
	memDc.DrawText(_T("实用工具"), menuTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	if (m_activePage == PAGE_SYSTEM_SETTINGS)
	{
		DrawSystemSettings(memDc, clientRect);
	}
	else if (m_activePage == PAGE_SYSTEM_STATUS)
	{
		DrawSystemStatus(memDc, clientRect);
	}
	else if (m_activePage == PAGE_SYSTEM_EXCEPTION)
	{
		DrawSystemExceptionInformation(memDc, clientRect);
	}
	else if (m_activePage == PAGE_STARTUP_ITEMS)
	{
		DrawStartupItems(memDc, clientRect);
	}
	else if (m_activePage == PAGE_ACPI_INFO)
	{
		DrawAcpiInformation(memDc, clientRect);
	}
	else if (m_activePage == PAGE_SSD_INFO)
	{
		DrawSsdInformation(memDc, clientRect);
	}
	else if (m_activePage == PAGE_SCREEN_INFO)
	{
		DrawScreenInformation(memDc, clientRect);
	}
	else if (m_activePage == PAGE_BATTERY_LOG)
	{
		DrawPowerCfgReportPage(memDc, clientRect, _T("电池日志"), _T("使用 powercfg /batteryreport 导出电池使用与容量报告"), m_batteryLogRows);
	}
	else if (m_activePage == PAGE_POWER_LOG)
	{
		DrawPowerCfgReportPage(memDc, clientRect, _T("电源日志"), _T("使用 powercfg /sleepstudy 导出睡眠与电源行为报告"), m_powerLogRows);
	}
	else if (m_activePage == PAGE_UTILITY_TOOLS)
	{
		DrawUtilityTools(memDc, clientRect);
	}
	else
	{
		CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 104);
		CRect listRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
		DrawRoundedCard(memDc, headerRect, UiSubtleSurface, 12, UiBorder);
		DrawRoundedCard(memDc, listRect, UiSurface, 12, UiBorder);

		memDc.SelectObject(&m_titleFont);
		memDc.SetTextColor(UiText);
		CRect titleRect(headerRect.left + 18, headerRect.top + 12, headerRect.right - 16, headerRect.top + 48);
		memDc.DrawText(_T("系统信息概览"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

		memDc.SelectObject(&m_subtitleFont);
		memDc.SetTextColor(UiSecondaryText);
		CRect subtitleRect(headerRect.left + 18, headerRect.top + 56, headerRect.right - 16, headerRect.bottom - 10);
		memDc.DrawText(_T("快速查看当前设备的关键硬件、固件与网络标识信息"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

		const int labelX = listRect.left + 16;
		const int labelWidth = 310;
		const int valueX = labelX + labelWidth + 46;
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
			memDc.SetTextColor(UiSecondaryText);
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
				memDc.SetTextColor(UiTertiaryText);
				CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
				memDc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

				memDc.SelectObject(&m_valueFont);
				memDc.SetTextColor(UiText);
				CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
				memDc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

				CPen linePen(PS_SOLID, 1, UiBorder);
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

// 绘制 SSD 信息页（列表风格与系统信息页保持一致）。
void CMFCApplication1Dlg::DrawSsdInformation(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	CRect tabCardRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, headerRect.bottom + 56);
	CRect listRect(m_contentRect.left + 8, tabCardRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, tabCardRect, UiSurfaceAlt, 10, UiBorder);
	DrawRoundedCard(dc, listRect, UiSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 16, headerRect.top + 42);
	dc.DrawText(_T("SSD信息"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("参考 DiskInfo 的按盘枚举思路，展示每块 SSD 的 SMART 相关状态"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	const int labelX = listRect.left + 16;
	const int labelWidth = 310;
	const int valueX = labelX + labelWidth + 46;
	const int contentTop = listRect.top + 14;
	const int contentBottom = listRect.bottom - 12;
	const int contentWidth = (listRect.right - 16) - valueX;
	const int valueRight = valueX + contentWidth;

	TEXTMETRIC labelTm = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&labelTm);
	const int labelLineHeight = max(static_cast<int>(labelTm.tmHeight + labelTm.tmExternalLeading), 24);

	TEXTMETRIC valueTm = {};
	dc.SelectObject(&m_valueFont);
	dc.GetTextMetrics(&valueTm);
	const int valueLineHeight = max(static_cast<int>(valueTm.tmHeight + valueTm.tmExternalLeading), 24);
	const int rowPadding = 8;
	const int rowGap = 10;

	std::vector<int> rowHeights;
	rowHeights.reserve(m_ssdRows.size());
	int totalHeight = 14;
	for (const InfoRow& row : m_ssdRows)
	{
		CRect calcRect(0, 0, max(contentWidth, 80), 0);
		dc.SelectObject(&m_valueFont);
		dc.DrawText(row.value, calcRect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		const int valueHeight = max(valueLineHeight, calcRect.Height());
		const int rowHeight = max(labelLineHeight, valueHeight) + rowPadding;
		rowHeights.push_back(rowHeight);
		totalHeight += rowHeight + rowGap;
	}
	totalHeight += 10;
	UpdateVerticalScrollBar(totalHeight, max(1, listRect.Height() - 4));

	int y = contentTop - m_scrollPos;
	const int oldDc = dc.SaveDC();
	dc.IntersectClipRect(listRect.left + 6, listRect.top + 6, listRect.right - 6, listRect.bottom - 6);

	for (size_t i = 0; i < m_ssdRows.size(); ++i)
	{
		const InfoRow& row = m_ssdRows[i];
		const int rowHeight = i < rowHeights.size() ? rowHeights[i] : (labelLineHeight + rowPadding);
		dc.SelectObject(&m_labelFont);
		dc.SetTextColor(UiTertiaryText);
		CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
		dc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

		dc.SelectObject(&m_valueFont);
		dc.SetTextColor(UiText);
		CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
		dc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

		CPen linePen(PS_SOLID, 1, UiBorder);
		CPen* oldLinePen = dc.SelectObject(&linePen);
		const int separatorY = y + rowHeight + (rowGap / 2);
		dc.MoveTo(labelX, separatorY);
		dc.LineTo(valueRight, separatorY);
		dc.SelectObject(oldLinePen);

		y += rowHeight + rowGap;
		if (y > contentBottom + 20)
		{
			break;
		}
	}
	dc.RestoreDC(oldDc);
}

void CMFCApplication1Dlg::DrawScreenInformation(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	CRect listRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, listRect, UiSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 16, headerRect.top + 42);
	dc.DrawText(_T("屏幕详情"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("读取当前活动显示器的 EDID 并解析关键信息"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	const int labelX = listRect.left + 16;
	const int labelWidth = 178;
	const int valueX = labelX + labelWidth + 34;
	const int contentTop = listRect.top + 14;
	const int contentBottom = listRect.bottom - 12;
	const int contentWidth = (listRect.right - 16) - valueX;
	const int valueRight = valueX + contentWidth;

	TEXTMETRIC labelTm = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&labelTm);
	const int labelLineHeight = max(static_cast<int>(labelTm.tmHeight + labelTm.tmExternalLeading), 24);

	TEXTMETRIC valueTm = {};
	dc.SelectObject(&m_valueFont);
	dc.GetTextMetrics(&valueTm);
	const int valueLineHeight = max(static_cast<int>(valueTm.tmHeight + valueTm.tmExternalLeading), 24);
	const int rowPadding = 8;
	const int rowGap = 10;

	std::vector<int> rowHeights;
	rowHeights.reserve(m_screenRows.size());
	int totalHeight = 14;
	for (const InfoRow& row : m_screenRows)
	{
		CRect calcRect(0, 0, max(contentWidth, 80), 0);
		dc.SelectObject(&m_valueFont);
		dc.DrawText(row.value, calcRect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		const int valueHeight = max(valueLineHeight, calcRect.Height());
		const int rowHeight = max(labelLineHeight, valueHeight) + rowPadding;
		rowHeights.push_back(rowHeight);
		totalHeight += rowHeight + rowGap;
	}
	totalHeight += 10;
	UpdateVerticalScrollBar(totalHeight, max(1, listRect.Height() - 4));

	int y = contentTop - m_scrollPos;
	const int oldDc = dc.SaveDC();
	dc.IntersectClipRect(listRect.left + 6, listRect.top + 6, listRect.right - 6, listRect.bottom - 6);

	for (size_t i = 0; i < m_screenRows.size(); ++i)
	{
		const InfoRow& row = m_screenRows[i];
		const int rowHeight = i < rowHeights.size() ? rowHeights[i] : (labelLineHeight + rowPadding);
		dc.SelectObject(&m_labelFont);
		dc.SetTextColor(UiTertiaryText);
		CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
		dc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

		dc.SelectObject(&m_valueFont);
		dc.SetTextColor(UiText);
		CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
		dc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

		CPen linePen(PS_SOLID, 1, UiBorder);
		CPen* oldLinePen = dc.SelectObject(&linePen);
		const int separatorY = y + rowHeight + (rowGap / 2);
		dc.MoveTo(labelX, separatorY);
		dc.LineTo(valueRight, separatorY);
		dc.SelectObject(oldLinePen);

		y += rowHeight + rowGap;
		if (y > contentBottom + 20)
		{
			break;
		}
	}
	dc.RestoreDC(oldDc);
}

void CMFCApplication1Dlg::DrawPowerCfgReportPage(CDC& dc, const CRect& clientRect, const CString& title, const CString& subtitle, const std::vector<InfoRow>& rows)
{
	UNREFERENCED_PARAMETER(clientRect);

	CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	CRect listRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, listRect, UiSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 230, headerRect.top + 42);
	dc.DrawText(title, titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 230, headerRect.bottom - 8);
	dc.DrawText(subtitle, subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	const int labelX = listRect.left + 16;
	const int labelWidth = 178;
	const int valueX = labelX + labelWidth + 34;
	const int contentTop = listRect.top + 14;
	const int contentBottom = listRect.bottom - 12;
	const int contentWidth = (listRect.right - 16) - valueX;
	const int valueRight = valueX + contentWidth;

	TEXTMETRIC labelTm = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&labelTm);
	const int labelLineHeight = max(static_cast<int>(labelTm.tmHeight + labelTm.tmExternalLeading), 24);

	TEXTMETRIC valueTm = {};
	dc.SelectObject(&m_valueFont);
	dc.GetTextMetrics(&valueTm);
	const int valueLineHeight = max(static_cast<int>(valueTm.tmHeight + valueTm.tmExternalLeading), 24);
	const int rowPadding = 8;
	const int rowGap = 10;

	std::vector<int> rowHeights;
	rowHeights.reserve(rows.size());
	int totalHeight = 14;
	for (const InfoRow& row : rows)
	{
		CRect calcRect(0, 0, max(contentWidth, 80), 0);
		dc.SelectObject(&m_valueFont);
		dc.DrawText(row.value, calcRect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		const int valueHeight = max(valueLineHeight, calcRect.Height());
		const int rowHeight = max(labelLineHeight, valueHeight) + rowPadding;
		rowHeights.push_back(rowHeight);
		totalHeight += rowHeight + rowGap;
	}
	totalHeight += 10;
	UpdateVerticalScrollBar(totalHeight, max(1, listRect.Height() - 4));

	int y = contentTop - m_scrollPos;
	const int oldDc = dc.SaveDC();
	dc.IntersectClipRect(listRect.left + 6, listRect.top + 6, listRect.right - 6, listRect.bottom - 6);

	if (rows.empty())
	{
		dc.SelectObject(&m_valueFont);
		dc.SetTextColor(UiSecondaryText);
		dc.TextOutW(labelX, y, _T("正在生成日志，请稍候..."));
	}
	else
	{
		for (size_t i = 0; i < rows.size(); ++i)
		{
			const InfoRow& row = rows[i];
			const int rowHeight = i < rowHeights.size() ? rowHeights[i] : (labelLineHeight + rowPadding);
			dc.SelectObject(&m_labelFont);
			dc.SetTextColor(UiTertiaryText);
			CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
			dc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

			dc.SelectObject(&m_valueFont);
			dc.SetTextColor(UiText);
			CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
			dc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

			CPen linePen(PS_SOLID, 1, UiBorder);
			CPen* oldLinePen = dc.SelectObject(&linePen);
			const int separatorY = y + rowHeight + (rowGap / 2);
			dc.MoveTo(labelX, separatorY);
			dc.LineTo(valueRight, separatorY);
			dc.SelectObject(oldLinePen);

			y += rowHeight + rowGap;
			if (y > contentBottom + 20)
			{
				break;
			}
		}
	}
	dc.RestoreDC(oldDc);
}

void CMFCApplication1Dlg::DrawSystemStatus(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	CRect listRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, listRect, UiSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 16, headerRect.top + 42);
	dc.DrawText(_T("系统状态"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("查看 Windows 版本、激活、安全启动和磁盘加密状态"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	const int labelX = listRect.left + 16;
	const int labelWidth = 178;
	const int valueX = labelX + labelWidth + 34;
	const int contentTop = listRect.top + 14;
	const int contentBottom = listRect.bottom - 12;
	const int contentWidth = (listRect.right - 16) - valueX;
	const int valueRight = valueX + contentWidth;

	TEXTMETRIC labelTm = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&labelTm);
	const int labelLineHeight = max(static_cast<int>(labelTm.tmHeight + labelTm.tmExternalLeading), 24);

	TEXTMETRIC valueTm = {};
	dc.SelectObject(&m_valueFont);
	dc.GetTextMetrics(&valueTm);
	const int valueLineHeight = max(static_cast<int>(valueTm.tmHeight + valueTm.tmExternalLeading), 24);
	const int rowPadding = 8;
	const int rowGap = 10;

	std::vector<int> rowHeights;
	rowHeights.reserve(m_systemStatusRows.size());
	int totalHeight = 14;
	for (const InfoRow& row : m_systemStatusRows)
	{
		CRect calcRect(0, 0, max(contentWidth, 80), 0);
		dc.SelectObject(&m_valueFont);
		dc.DrawText(row.value, calcRect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		const int valueHeight = max(valueLineHeight, calcRect.Height());
		const int rowHeight = max(labelLineHeight, valueHeight) + rowPadding;
		rowHeights.push_back(rowHeight);
		totalHeight += rowHeight + rowGap;
	}
	totalHeight += 10;
	UpdateVerticalScrollBar(totalHeight, max(1, listRect.Height() - 4));

	int y = contentTop - m_scrollPos;
	const int oldDc = dc.SaveDC();
	dc.IntersectClipRect(listRect.left + 6, listRect.top + 6, listRect.right - 6, listRect.bottom - 6);

	if (m_systemStatusRows.empty())
	{
		dc.SelectObject(&m_valueFont);
		dc.SetTextColor(UiSecondaryText);
		dc.TextOutW(labelX, y, _T("正在加载系统状态，请稍候..."));
	}
	else
	{
		for (size_t i = 0; i < m_systemStatusRows.size(); ++i)
		{
			const InfoRow& row = m_systemStatusRows[i];
			const int rowHeight = i < rowHeights.size() ? rowHeights[i] : (labelLineHeight + rowPadding);
			dc.SelectObject(&m_labelFont);
			dc.SetTextColor(UiTertiaryText);
			CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
			dc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

			dc.SelectObject(&m_valueFont);
			dc.SetTextColor(UiText);
			CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
			dc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

			CPen linePen(PS_SOLID, 1, UiBorder);
			CPen* oldLinePen = dc.SelectObject(&linePen);
			const int separatorY = y + rowHeight + (rowGap / 2);
			dc.MoveTo(labelX, separatorY);
			dc.LineTo(valueRight, separatorY);
			dc.SelectObject(oldLinePen);

			y += rowHeight + rowGap;
			if (y > contentBottom + 20)
			{
				break;
			}
		}
	}
	dc.RestoreDC(oldDc);
}

void CMFCApplication1Dlg::DrawSystemExceptionInformation(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	CRect listRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, listRect, UiSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 16, headerRect.top + 42);
	dc.DrawText(_T("系统异常"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("查看异常驱动信息，以及 System 日志中的 WER/System Error/Kernel-Power 重大异常事件"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	const int labelX = listRect.left + 16;
	const int labelWidth = 178;
	const int valueX = labelX + labelWidth + 34;
	const int contentTop = listRect.top + 14;
	const int contentBottom = listRect.bottom - 12;
	const int contentWidth = (listRect.right - 16) - valueX;
	const int valueRight = valueX + contentWidth;

	TEXTMETRIC labelTm = {};
	dc.SelectObject(&m_labelFont);
	dc.GetTextMetrics(&labelTm);
	const int labelLineHeight = max(static_cast<int>(labelTm.tmHeight + labelTm.tmExternalLeading), 24);

	TEXTMETRIC valueTm = {};
	dc.SelectObject(&m_valueFont);
	dc.GetTextMetrics(&valueTm);
	const int valueLineHeight = max(static_cast<int>(valueTm.tmHeight + valueTm.tmExternalLeading), 24);
	const int rowPadding = 8;
	const int rowGap = 10;

	std::vector<int> rowHeights;
	rowHeights.reserve(m_systemExceptionRows.size());
	int totalHeight = 14;
	for (const InfoRow& row : m_systemExceptionRows)
	{
		CRect calcRect(0, 0, max(contentWidth, 80), 0);
		dc.SelectObject(&m_valueFont);
		dc.DrawText(row.value, calcRect, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
		const int valueHeight = max(valueLineHeight, calcRect.Height());
		const int rowHeight = max(labelLineHeight, valueHeight) + rowPadding;
		rowHeights.push_back(rowHeight);
		totalHeight += rowHeight + rowGap;
	}
	totalHeight += 10;
	UpdateVerticalScrollBar(totalHeight, max(1, listRect.Height() - 4));

	int y = contentTop - m_scrollPos;
	const int oldDc = dc.SaveDC();
	dc.IntersectClipRect(listRect.left + 6, listRect.top + 6, listRect.right - 6, listRect.bottom - 6);

	if (m_systemExceptionRows.empty())
	{
		dc.SelectObject(&m_valueFont);
		dc.SetTextColor(UiSecondaryText);
		dc.TextOutW(labelX, y, _T("正在加载系统异常信息，请稍候..."));
	}
	else
	{
		for (size_t i = 0; i < m_systemExceptionRows.size(); ++i)
		{
			const InfoRow& row = m_systemExceptionRows[i];
			const int rowHeight = i < rowHeights.size() ? rowHeights[i] : (labelLineHeight + rowPadding);
			dc.SelectObject(&m_labelFont);
			dc.SetTextColor(UiTertiaryText);
			CRect labelRect(labelX, y, labelX + labelWidth, y + rowHeight);
			dc.DrawText(row.item, labelRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

			dc.SelectObject(&m_valueFont);
			dc.SetTextColor(UiText);
			CRect valueRect(valueX, y + 1, valueRight, y + rowHeight);
			dc.DrawText(row.value, valueRect, DT_LEFT | DT_WORDBREAK);

			CPen linePen(PS_SOLID, 1, UiBorder);
			CPen* oldLinePen = dc.SelectObject(&linePen);
			const int separatorY = y + rowHeight + (rowGap / 2);
			dc.MoveTo(labelX, separatorY);
			dc.LineTo(valueRight, separatorY);
			dc.SelectObject(oldLinePen);

			y += rowHeight + rowGap;
			if (y > contentBottom + 20)
			{
				break;
			}
		}
	}
	dc.RestoreDC(oldDc);
}

void CMFCApplication1Dlg::DrawStartupItems(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	const int actionHeight = 156;
	const CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	const CRect listCardRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - actionHeight - 16);
	const CRect actionCardRect(m_contentRect.left + 8, listCardRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, listCardRect, UiSurface, 12, UiBorder);
	DrawRoundedCard(dc, actionCardRect, UiSurfaceAlt, 10, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 16, headerRect.top + 42);
	dc.DrawText(_T("启动项"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("查看开机自动启动项目，右键启动项可启用、禁用、删除或刷新"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

	UpdateVerticalScrollBar(max(1, m_contentRect.Height()), max(1, m_contentRect.Height()));
}

void CMFCApplication1Dlg::DrawDriverDetails(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 18, headerRect.top + 10, headerRect.right - 16, headerRect.top + 42);
	dc.DrawText(_T("驱动详情"), titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 18, headerRect.top + 42, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("树状展示：计算机名称 > 设备类型 > 驱动名称 / 版本 / 日期"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);
}

void CMFCApplication1Dlg::DrawAcpiInformation(CDC& dc, const CRect& clientRect)
{
	DrawDriverDetails(dc, clientRect);
}

// 绘制系统设置页背景卡片与分组标题。
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

	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);
	DrawRoundedCard(dc, optionsCardRect, UiSurface, 10, UiBorder);
	DrawRoundedCard(dc, buttonCardRect, UiSurfaceAlt, 8, UiBorder);
	DrawRoundedCard(dc, statusCardRect, UiSurfaceAlt, 8, UiBorder);

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

	DrawRoundedCard(dc, grpSecurity, UiSurfaceAlt, 8, UiBorder);
	DrawRoundedCard(dc, grpBehavior, UiSurfaceAlt, 8, UiBorder);
	DrawRoundedCard(dc, grpPower, UiSurfaceAlt, 8, UiBorder);
	DrawRoundedCard(dc, grpUpdate, UiSurfaceAlt, 8, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 16, headerRect.top + 10, headerRect.right - 16, headerRect.top + 40);
	dc.DrawText(_T("系统设置优化"), titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 16, headerRect.top + 40, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("勾选后可执行系统优化项"), subtitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	dc.SelectObject(&m_labelFont);
	dc.SetTextColor(UiSecondaryText);
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

// 根据内容高度与可视高度刷新滚动条参数。
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

// 计算侧边栏、内容区与菜单按钮矩形。
void CMFCApplication1Dlg::RecalcLayoutRects(const CRect& clientRect)
{
	const int margin = 12;
	const int availableWidth = max(320, clientRect.Width() - margin * 2);
	const int sideWidth = max(224, min(280, availableWidth / 4));
	m_sideRect = CRect(margin, margin, margin + sideWidth, clientRect.bottom - margin);
	m_contentRect = CRect(m_sideRect.right + 12, margin, clientRect.right - margin, clientRect.bottom - margin);
	m_infoMenuRect = CRect(m_sideRect.left + 10, m_sideRect.top + 54, m_sideRect.right - 10, m_sideRect.top + 102);
	m_statusMenuRect = CRect(m_sideRect.left + 10, m_infoMenuRect.bottom + 10, m_sideRect.right - 10, m_infoMenuRect.bottom + 62);
	m_exceptionMenuRect = CRect(m_sideRect.left + 10, m_statusMenuRect.bottom + 10, m_sideRect.right - 10, m_statusMenuRect.bottom + 62);
	m_startupMenuRect = CRect(m_sideRect.left + 10, m_exceptionMenuRect.bottom + 10, m_sideRect.right - 10, m_exceptionMenuRect.bottom + 62);
	m_acpiMenuRect = CRect(m_sideRect.left + 10, m_startupMenuRect.bottom + 10, m_sideRect.right - 10, m_startupMenuRect.bottom + 62);
	m_ssdMenuRect = CRect(m_sideRect.left + 10, m_acpiMenuRect.bottom + 10, m_sideRect.right - 10, m_acpiMenuRect.bottom + 62);
	m_screenMenuRect = CRect(m_sideRect.left + 10, m_ssdMenuRect.bottom + 10, m_sideRect.right - 10, m_ssdMenuRect.bottom + 62);
	m_batteryLogMenuRect = CRect(m_sideRect.left + 10, m_screenMenuRect.bottom + 10, m_sideRect.right - 10, m_screenMenuRect.bottom + 62);
	m_powerLogMenuRect = CRect(m_sideRect.left + 10, m_batteryLogMenuRect.bottom + 10, m_sideRect.right - 10, m_batteryLogMenuRect.bottom + 62);
	m_utilityMenuRect = CRect(m_sideRect.left + 10, m_powerLogMenuRect.bottom + 10, m_sideRect.right - 10, m_powerLogMenuRect.bottom + 62);
	m_settingsMenuRect = CRect(m_sideRect.left + 10, m_utilityMenuRect.bottom + 10, m_sideRect.right - 10, m_utilityMenuRect.bottom + 62);
}

// 动态创建设置页复选框、按钮与状态文本控件。
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
		SetWindowTheme(checkBox->GetSafeHwnd(), L"Explorer", nullptr);
	}

	m_btnApply.Create(_T("应用选中的设置"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BTN_APPLY_SETTINGS);
	m_btnReboot.Create(_T("重启系统"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BTN_REBOOT);
	m_btnToggleSelect.Create(_T("取消全选"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BTN_TOGGLE_SELECT);
	m_btnApply.SetFont(&m_settingsFont);
	m_btnReboot.SetFont(&m_settingsFont);
	m_btnToggleSelect.SetFont(&m_settingsFont);
	SetWindowTheme(m_btnApply.GetSafeHwnd(), L"Explorer", nullptr);
	SetWindowTheme(m_btnReboot.GetSafeHwnd(), L"Explorer", nullptr);
	SetWindowTheme(m_btnToggleSelect.GetSafeHwnd(), L"Explorer", nullptr);

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

// 动态创建 SSD 页签控件（用于切换多块 SSD）。
void CMFCApplication1Dlg::CreateSsdControls()
{
	m_ssdTab.Create(WS_CHILD | WS_TABSTOP | TCS_TABS | TCS_SINGLELINE, CRect(0, 0, 10, 10), this, IDC_SSD_TAB);
	m_ssdTab.SetFont(&m_settingsFont);
	SetWindowTheme(m_ssdTab.GetSafeHwnd(), L"Explorer", nullptr);
	RefreshSsdTabs();
	UpdateSsdControlLayout();
}

void CMFCApplication1Dlg::CreateDriverControls()
{
	m_driverTree.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
		CRect(0, 0, 10, 10), this, IDC_DRIVER_TREE);
	m_driverTree.SetFont(&m_valueFont);
	m_driverTree.SetItemHeight(40);
	m_driverImageList.Create(24, 24, ILC_COLOR32 | ILC_MASK, 16, 8);
	m_driverImageList.Add(m_hIcon != nullptr ? m_hIcon : AfxGetApp()->LoadStandardIcon(IDI_APPLICATION));
	m_driverTree.SetImageList(&m_driverImageList, TVSIL_NORMAL);
	SetWindowTheme(m_driverTree.GetSafeHwnd(), L"Explorer", nullptr);
	ShowDriverDetailsLoading();
}

void CMFCApplication1Dlg::UpdateDriverControlLayout()
{
	if (!::IsWindow(m_driverTree.GetSafeHwnd()))
	{
		return;
	}
	CRect treeRect(m_contentRect.left + 14, m_contentRect.top + 118, m_contentRect.right - 14, m_contentRect.bottom - 14);
	m_driverTree.MoveWindow(treeRect, TRUE);
}

void CMFCApplication1Dlg::LoadDriverDetails()
{
	if (m_driverDetailsLoaded)
	{
		return;
	}

	CollectDriverDetails(m_driverComputerName, m_driverClassGroups);
	m_driverDetailsLoaded = true;
	m_driverDetailsLoading = false;
	RefreshDriverTree();
}

void CMFCApplication1Dlg::ShowDriverDetailsLoading()
{
	if (!::IsWindow(m_driverTree.GetSafeHwnd()))
	{
		return;
	}
	m_driverTree.SetRedraw(FALSE);
	m_driverTree.DeleteAllItems();
	HTREEITEM hComputer = m_driverTree.InsertItem(HasValue(m_driverComputerName) ? m_driverComputerName : _T("本机"), 0, 0, TVI_ROOT, TVI_LAST);
	m_driverTree.InsertItem(_T("正在加载驱动信息..."), 0, 0, hComputer, TVI_LAST);
	m_driverTree.Expand(hComputer, TVE_EXPAND);
	m_driverTree.SetRedraw(TRUE);
}

void CMFCApplication1Dlg::CollectDriverDetails(CString& computerName, std::vector<DriverClassGroup>& classGroups)
{
	computerName.Empty();
	classGroups.clear();

	std::map<CString, DriverClassGroup, std::less<>> classes;
	std::set<CString, std::less<>> seenKeys;

	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES);
	if (deviceInfoSet != INVALID_HANDLE_VALUE)
	{
		for (DWORD index = 0;; ++index)
		{
			SP_DEVINFO_DATA deviceInfoData = {};
			deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			if (!SetupDiEnumDeviceInfo(deviceInfoSet, index, &deviceInfoData))
			{
				break;
			}

			CString name = ReadSetupDeviceProperty(deviceInfoSet, deviceInfoData, SPDRP_FRIENDLYNAME);
			if (!HasValue(name))
			{
				name = ReadSetupDeviceProperty(deviceInfoSet, deviceInfoData, SPDRP_DEVICEDESC);
			}
			if (!HasValue(name))
			{
				continue;
			}

			CString classGuid;
			TCHAR guidText[64] = {};
			if (StringFromGUID2(deviceInfoData.ClassGuid, guidText, _countof(guidText)) > 0)
			{
				classGuid = guidText;
			}

			CString deviceClass = ReadSetupDeviceProperty(deviceInfoSet, deviceInfoData, SPDRP_CLASS);
			if (!HasValue(deviceClass))
			{
				TCHAR className[128] = {};
				if (SetupDiClassNameFromGuid(&deviceInfoData.ClassGuid, className, _countof(className), nullptr))
				{
					deviceClass = className;
				}
			}

			const CString driverKey = ReadSetupDeviceProperty(deviceInfoSet, deviceInfoData, SPDRP_DRIVER);
			CString driverVersion = ReadDriverKeyValue(driverKey, _T("DriverVersion"));
			CString driverDate = FormatDriverDateText(ReadDriverKeyValue(driverKey, _T("DriverDate")));
			ULONG deviceStatus = 0;
			ULONG deviceProblem = 0;
			const bool present = (CM_Get_DevNode_Status(&deviceStatus, &deviceProblem, deviceInfoData.DevInst, 0) == CR_SUCCESS);
			if (!present)
			{
				continue;
			}
			const bool healthy = ((deviceStatus & DN_HAS_PROBLEM) == 0) && deviceProblem == 0;

			const CString className = NormalizeDeviceClassName(deviceClass, classGuid);

			name.Trim();
			if (name.IsEmpty())
			{
				continue;
			}

			CString key = ToLower(className);
			key += _T("|");
			key += ToLower(name);
			key += _T("|");
			key += ToLower(driverVersion);
			key += _T("|");
			key += driverDate;
			if (seenKeys.find(key) != seenKeys.end())
			{
				continue;
			}
			seenKeys.insert(key);

			DriverEntry entry;
			entry.name = name;
			entry.version = driverVersion;
			entry.date = driverDate;
			entry.present = present;
			entry.healthy = healthy;

			DriverClassGroup& group = classes[className];
			group.className = className;
			if (!HasValue(group.classGuid))
			{
				group.classGuid = classGuid;
			}
			group.drivers.push_back(entry);
		}
		SetupDiDestroyDeviceInfoList(deviceInfoSet);
	}

	computerName = FirstValue(_T("ROOT\\CIMV2"), _T("SELECT Name FROM Win32_ComputerSystem"), _T("Name"));
	if (!HasValue(computerName))
	{
		TCHAR nameBuffer[MAX_COMPUTERNAME_LENGTH + 1] = {};
		DWORD nameChars = _countof(nameBuffer);
		if (GetComputerName(nameBuffer, &nameChars))
		{
			computerName = nameBuffer;
		}
	}
	if (!HasValue(computerName))
	{
		computerName = _T("本机");
	}

	for (auto& classPair : classes)
	{
		std::sort(classPair.second.drivers.begin(), classPair.second.drivers.end(), [](const DriverEntry& a, const DriverEntry& b)
			{
				if (a.healthy != b.healthy)
				{
					return a.healthy && !b.healthy;
				}
				if (a.present != b.present)
				{
					return a.present && !b.present;
				}
				return a.name.CompareNoCase(b.name) < 0;
			});

		classGroups.push_back(classPair.second);
	}

	std::sort(classGroups.begin(), classGroups.end(), [](const DriverClassGroup& a, const DriverClassGroup& b)
		{
			return a.className.CompareNoCase(b.className) < 0;
		});
}

void CMFCApplication1Dlg::RefreshDriverTree()
{
	if (!::IsWindow(m_driverTree.GetSafeHwnd()))
	{
		return;
	}

	m_driverTree.SetRedraw(FALSE);
	m_driverTree.DeleteAllItems();

	if (m_driverImageList.GetSafeHandle() != nullptr)
	{
		m_driverImageList.DeleteImageList();
	}
	m_driverImageList.Create(24, 24, ILC_COLOR32 | ILC_MASK, max(16, static_cast<int>(m_driverClassGroups.size()) + 1), 8);
	const int rootIcon = m_driverImageList.Add(m_hIcon != nullptr ? m_hIcon : AfxGetApp()->LoadStandardIcon(IDI_APPLICATION));
	std::map<CString, int, std::less<>> iconByGuid;
	auto classIconIndex = [&](const CString& classGuid) -> int
		{
			if (!HasValue(classGuid))
			{
				return rootIcon;
			}
			const CString key = ToLower(classGuid);
			const auto found = iconByGuid.find(key);
			if (found != iconByGuid.end())
			{
				return found->second;
			}

			int iconIndex = rootIcon;
			GUID guid = {};
			if (SUCCEEDED(CLSIDFromString(const_cast<LPOLESTR>(static_cast<LPCOLESTR>(classGuid.GetString())), &guid)))
			{
				HICON hClassIcon = nullptr;
				if (SetupDiLoadClassIcon(&guid, &hClassIcon, nullptr) && hClassIcon != nullptr)
				{
					iconIndex = m_driverImageList.Add(hClassIcon);
					DestroyIcon(hClassIcon);
				}
			}
			iconByGuid[key] = iconIndex;
			return iconIndex;
		};
	m_driverTree.SetImageList(&m_driverImageList, TVSIL_NORMAL);

	HTREEITEM hComputer = m_driverTree.InsertItem(HasValue(m_driverComputerName) ? m_driverComputerName : _T("本机"), rootIcon, rootIcon, TVI_ROOT, TVI_LAST);
	if (hComputer == nullptr)
	{
		m_driverTree.SetRedraw(TRUE);
		return;
	}

	if (m_driverDetailsLoading && !m_driverDetailsLoaded)
	{
		m_driverTree.InsertItem(_T("正在加载驱动信息..."), rootIcon, rootIcon, hComputer, TVI_LAST);
		m_driverTree.Expand(hComputer, TVE_EXPAND);
		m_driverTree.SetRedraw(TRUE);
		return;
	}

	if (m_driverClassGroups.empty())
	{
		m_driverTree.InsertItem(_T("未获取到驱动信息"), rootIcon, rootIcon, hComputer, TVI_LAST);
		m_driverTree.Expand(hComputer, TVE_EXPAND);
		m_driverTree.SetRedraw(TRUE);
		return;
	}

	for (const auto& classGroup : m_driverClassGroups)
	{
		CString classLabel;
		classLabel.Format(_T("%s  (%d)"), static_cast<LPCTSTR>(classGroup.className), static_cast<int>(classGroup.drivers.size()));

		const int iconIndex = classIconIndex(classGroup.classGuid);
		HTREEITEM hClass = m_driverTree.InsertItem(classLabel, iconIndex, iconIndex, hComputer, TVI_LAST);
		if (hClass == nullptr)
		{
			continue;
		}

		for (const auto& dev : classGroup.drivers)
		{
			CString deviceName = dev.name;
			const CString statusText = dev.healthy ? _T("● 正常运行") : _T("● 异常");
			CString driverLine;
			driverLine.Format(_T("%s    |    版本: %s    |    日期: %s    |    状态: %s"),
				static_cast<LPCTSTR>(deviceName),
				HasValue(dev.version) ? static_cast<LPCTSTR>(dev.version) : _T("N/A"),
				HasValue(dev.date) ? static_cast<LPCTSTR>(dev.date) : _T("N/A"),
				static_cast<LPCTSTR>(statusText));
			HTREEITEM hDevice = m_driverTree.InsertItem(driverLine, iconIndex, iconIndex, hClass, TVI_LAST);
			if (hDevice != nullptr)
			{
				m_driverTree.SetItemData(hDevice, dev.healthy ? 1 : 2);
			}
		}
	}

	m_driverTree.Expand(hComputer, TVE_EXPAND);
	m_driverTree.SetRedraw(TRUE);
}

void CMFCApplication1Dlg::CreateStartupControls()
{
	m_startupList.Create(WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		CRect(0, 0, 10, 10), this, IDC_STARTUP_LIST);
	m_startupList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);
	m_startupList.SetFont(&m_startupListFont);
	if (m_startupRowImageList.GetSafeHandle() == nullptr)
	{
		m_startupRowImageList.Create(1, 34, ILC_COLOR32, 1, 1);
		m_startupList.SetImageList(&m_startupRowImageList, LVSIL_SMALL);
	}
	m_startupList.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 180);
	m_startupList.InsertColumn(1, _T("状态"), LVCFMT_LEFT, 104);
	m_startupList.InsertColumn(2, _T("来源"), LVCFMT_LEFT, 210);
	m_startupList.InsertColumn(3, _T("命令/路径"), LVCFMT_LEFT, 520);
	SetWindowTheme(m_startupList.GetSafeHwnd(), L"Explorer", nullptr);

	m_btnStartupEnable.Create(_T("启用"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_STARTUP_ENABLE);
	m_btnStartupDisable.Create(_T("禁用"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_STARTUP_DISABLE);
	m_btnStartupDelete.Create(_T("删除"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_STARTUP_DELETE);
	m_btnStartupRefresh.Create(_T("刷新"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_STARTUP_REFRESH);
	m_labelStartupName.Create(_T("名称"), WS_CHILD | SS_LEFT | SS_CENTERIMAGE, CRect(0, 0, 10, 10), this, IDC_STARTUP_NAME_LABEL);
	m_labelStartupPath.Create(_T("程序路径或命令"), WS_CHILD | SS_LEFT | SS_CENTERIMAGE, CRect(0, 0, 10, 10), this, IDC_STARTUP_PATH_LABEL);
	m_editStartupName.Create(WS_CHILD | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, CRect(0, 0, 10, 10), this, IDC_STARTUP_NAME);
	m_editStartupPath.Create(WS_CHILD | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL, CRect(0, 0, 10, 10), this, IDC_STARTUP_PATH);
	m_btnStartupBrowse.Create(_T("浏览"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_STARTUP_BROWSE);
	m_btnStartupAdd.Create(_T("添加启动项"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_STARTUP_ADD);
	m_startupStatusText.Create(_T("就绪"), WS_CHILD | SS_LEFT | SS_CENTERIMAGE, CRect(0, 0, 10, 10), this, IDC_STARTUP_STATUS);

	CWnd* controls[] = {
		&m_btnStartupEnable, &m_btnStartupDisable, &m_btnStartupDelete, &m_btnStartupRefresh,
		&m_labelStartupName, &m_labelStartupPath, &m_editStartupName, &m_editStartupPath, &m_btnStartupBrowse,
		&m_btnStartupAdd, &m_startupStatusText
	};
	for (CWnd* control : controls)
	{
		control->SetFont(&m_settingsFont);
		SetWindowTheme(control->GetSafeHwnd(), L"Explorer", nullptr);
	}

	LoadStartupItems();
	UpdateStartupControlLayout();
	UpdateStartupButtons();
}

void CMFCApplication1Dlg::CreatePowerLogControls()
{
	m_btnBatteryLogDetails.Create(_T("详情"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BATTERY_LOG_DETAILS);
	m_btnBatteryLogRefresh.Create(_T("刷新"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_BATTERY_LOG_REFRESH);
	m_btnPowerLogDetails.Create(_T("详情"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_POWER_LOG_DETAILS);
	m_btnPowerLogRefresh.Create(_T("刷新"), WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, CRect(0, 0, 10, 10), this, IDC_POWER_LOG_REFRESH);
	m_btnBatteryLogDetails.SetFont(&m_settingsFont);
	m_btnBatteryLogRefresh.SetFont(&m_settingsFont);
	m_btnPowerLogDetails.SetFont(&m_settingsFont);
	m_btnPowerLogRefresh.SetFont(&m_settingsFont);
	SetWindowTheme(m_btnBatteryLogDetails.GetSafeHwnd(), L"Explorer", nullptr);
	SetWindowTheme(m_btnBatteryLogRefresh.GetSafeHwnd(), L"Explorer", nullptr);
	SetWindowTheme(m_btnPowerLogDetails.GetSafeHwnd(), L"Explorer", nullptr);
	SetWindowTheme(m_btnPowerLogRefresh.GetSafeHwnd(), L"Explorer", nullptr);
	UpdatePowerLogControlLayout();
}

void CMFCApplication1Dlg::UpdatePowerLogControlLayout()
{
	if (!::IsWindow(m_btnBatteryLogDetails.GetSafeHwnd()) ||
		!::IsWindow(m_btnBatteryLogRefresh.GetSafeHwnd()) ||
		!::IsWindow(m_btnPowerLogDetails.GetSafeHwnd()) ||
		!::IsWindow(m_btnPowerLogRefresh.GetSafeHwnd()))
	{
		return;
	}

	const CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	const int buttonWidth = 92;
	const int buttonHeight = 36;
	const int buttonGap = 10;
	const int right = headerRect.right - 18;
	const int top = headerRect.top + 30;
	m_btnBatteryLogDetails.MoveWindow(right - buttonWidth, top, buttonWidth, buttonHeight, TRUE);
	m_btnBatteryLogRefresh.MoveWindow(right - buttonWidth * 2 - buttonGap, top, buttonWidth, buttonHeight, TRUE);
	m_btnPowerLogDetails.MoveWindow(right - buttonWidth, top, buttonWidth, buttonHeight, TRUE);
	m_btnPowerLogRefresh.MoveWindow(right - buttonWidth * 2 - buttonGap, top, buttonWidth, buttonHeight, TRUE);
}

// 按当前 SSD 页几何参数更新页签控件位置。
void CMFCApplication1Dlg::UpdateSsdControlLayout()
{
	if (!::IsWindow(m_ssdTab.GetSafeHwnd()))
	{
		return;
	}

	const CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	const CRect tabCardRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, headerRect.bottom + 56);
	m_ssdTabRect = tabCardRect;
	m_ssdTabRect.DeflateRect(10, 8);
	m_ssdTab.MoveWindow(m_ssdTabRect, TRUE);
}

// 按当前 SSD 数据刷新页签，并同步选中盘信息。
void CMFCApplication1Dlg::RefreshSsdTabs()
{
	if (!::IsWindow(m_ssdTab.GetSafeHwnd()))
	{
		return;
	}

	m_ssdTab.DeleteAllItems();
	for (size_t i = 0; i < m_ssdTabTitles.size(); ++i)
	{
		TCITEM item = {};
		item.mask = TCIF_TEXT;
		item.pszText = const_cast<LPTSTR>(static_cast<LPCTSTR>(m_ssdTabTitles[i]));
		m_ssdTab.InsertItem(static_cast<int>(i), &item);
	}

	if (m_ssdTabTitles.empty())
	{
		TCITEM item = {};
		item.mask = TCIF_TEXT;
		item.pszText = const_cast<LPTSTR>(_T("无SSD"));
		m_ssdTab.InsertItem(0, &item);
		m_activeSsdIndex = 0;
		return;
	}

	m_activeSsdIndex = max(0, min(m_activeSsdIndex, static_cast<int>(m_ssdTabTitles.size()) - 1));
	m_ssdTab.SetCurSel(m_activeSsdIndex);
	if (m_activeSsdIndex >= 0 && m_activeSsdIndex < static_cast<int>(m_ssdDiskRows.size()))
	{
		m_ssdRows = m_ssdDiskRows[static_cast<size_t>(m_activeSsdIndex)];
	}
}

void CMFCApplication1Dlg::UpdateStartupControlLayout()
{
	if (!::IsWindow(m_startupList.GetSafeHwnd()))
	{
		return;
	}

	const int actionHeight = 156;
	const CRect headerRect(m_contentRect.left + 8, m_contentRect.top + 8, m_contentRect.right - 8, m_contentRect.top + 96);
	const CRect listCardRect(m_contentRect.left + 8, headerRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - actionHeight - 16);
	const CRect actionCardRect(m_contentRect.left + 8, listCardRect.bottom + 8, m_contentRect.right - 8, m_contentRect.bottom - 8);
	const int inner = 12;
	const int buttonHeight = 34;
	const int editHeight = 30;
	const int gap = 8;

	m_startupList.MoveWindow(listCardRect.left + inner, listCardRect.top + inner,
		max(80, listCardRect.Width() - inner * 2), max(80, listCardRect.Height() - inner * 2), TRUE);

	const int actionLeft = actionCardRect.left + inner;
	const int actionRight = actionCardRect.right - inner;
	const int firstRowTop = actionCardRect.top + 10;
	m_btnStartupEnable.MoveWindow(0, 0, 0, 0, TRUE);
	m_btnStartupDisable.MoveWindow(0, 0, 0, 0, TRUE);
	m_btnStartupDelete.MoveWindow(0, 0, 0, 0, TRUE);
	m_btnStartupRefresh.MoveWindow(0, 0, 0, 0, TRUE);
	m_startupStatusText.MoveWindow(actionLeft, firstRowTop, max(180, actionRight - actionLeft), buttonHeight, TRUE);

	const int labelTop = actionCardRect.top + 58;
	const int secondRowTop = actionCardRect.top + 92;
	const int addButtonWidth = 118;
	const int browseWidth = 72;
	const int nameWidth = max(220, min(320, (actionCardRect.Width() - inner * 2) / 4));
	const int pathLeft = actionLeft + nameWidth + gap;
	const int pathRight = actionRight - browseWidth - gap - addButtonWidth - gap;
	m_labelStartupName.MoveWindow(actionLeft, labelTop, nameWidth, 30, TRUE);
	m_labelStartupPath.MoveWindow(pathLeft, labelTop, max(160, pathRight - pathLeft), 30, TRUE);
	m_editStartupName.MoveWindow(actionLeft, secondRowTop, nameWidth, editHeight, TRUE);
	m_editStartupPath.MoveWindow(pathLeft, secondRowTop, max(140, pathRight - pathLeft), editHeight, TRUE);
	m_btnStartupBrowse.MoveWindow(pathRight + gap, secondRowTop - 2, browseWidth, buttonHeight, TRUE);
	m_btnStartupAdd.MoveWindow(actionRight - addButtonWidth, secondRowTop - 2, addButtonWidth, buttonHeight, TRUE);

	const int listWidth = max(300, listCardRect.Width() - inner * 2);
	m_startupList.SetColumnWidth(0, max(180, listWidth / 5));
	m_startupList.SetColumnWidth(1, 104);
	m_startupList.SetColumnWidth(2, max(210, listWidth / 5));
	m_startupList.SetColumnWidth(3, max(240, listWidth - m_startupList.GetColumnWidth(0) - m_startupList.GetColumnWidth(1) - m_startupList.GetColumnWidth(2) - 8));
}

void CMFCApplication1Dlg::LoadStartupItems()
{
	m_startupItems.clear();

	auto startupItemExists = [this](const StartupItem& item) -> bool
		{
			const CString itemName = ToLower(Trimmed(item.name));
			const CString itemCommand = ToLower(Trimmed(item.command));
			for (const StartupItem& existing : m_startupItems)
			{
				if (ToLower(Trimmed(existing.name)) == itemName &&
					ToLower(Trimmed(existing.command)) == itemCommand)
				{
					return true;
				}
			}
			return false;
		};

	auto startupApprovedRecordExists = [this](HKEY rootKey, const CString& approvedKey, const CString& valueName) -> bool
		{
			for (const StartupItem& existing : m_startupItems)
			{
				if (existing.approvedRoot == rootKey &&
					existing.approvedSubKey.CompareNoCase(approvedKey) == 0 &&
					existing.name.CompareNoCase(valueName) == 0)
				{
					return true;
				}
			}
			return false;
		};

	auto addRegistryItems = [this, &startupItemExists](HKEY rootKey, REGSAM viewFlag, bool disabled, const CString& approvedKey, const CString& label, StartupSource source)
		{
			HKEY key = nullptr;
			const CString subKey = StartupRunSubKey(disabled);
			if (RegOpenKeyEx(rootKey, subKey, 0, KEY_READ | viewFlag, &key) != ERROR_SUCCESS)
			{
				return;
			}

			DWORD index = 0;
			for (;;)
			{
				TCHAR valueName[512] = {};
				BYTE data[4096] = {};
				DWORD valueNameChars = _countof(valueName);
				DWORD dataBytes = sizeof(data);
				DWORD type = 0;
				const LSTATUS status = RegEnumValue(key, index, valueName, &valueNameChars, nullptr, &type, data, &dataBytes);
				if (status == ERROR_NO_MORE_ITEMS)
				{
					break;
				}
				++index;
				if (status != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ))
				{
					continue;
				}

				StartupItem item;
				item.name = valueName;
				item.command = reinterpret_cast<LPCTSTR>(data);
				item.sourceLabel = label;
				item.location = subKey;
				item.source = source;
				item.enabled = !disabled;
				item.approvedRoot = rootKey;
				item.approvedSubKey = approvedKey;
				if (!disabled)
				{
					bool approvedEnabled = true;
					if (ReadStartupApprovedEnabled(rootKey, approvedKey, item.name, approvedEnabled))
					{
						item.enabled = approvedEnabled;
						if (!approvedEnabled)
						{
							item.sourceLabel = label;
						}
					}
				}
				if (startupItemExists(item))
				{
					continue;
				}
				m_startupItems.push_back(item);
			}
			RegCloseKey(key);
		};

	addRegistryItems(HKEY_CURRENT_USER, KEY_WOW64_64KEY, false, _T("Run"), _T("当前用户 Run"), StartupSource::HkcuRun);
	addRegistryItems(HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY, false, _T("Run"), _T("所有用户 Run"), StartupSource::HklmRun);
	addRegistryItems(HKEY_CURRENT_USER, KEY_WOW64_32KEY, false, _T("Run32"), _T("当前用户 Run32"), StartupSource::HkcuRun32);
	addRegistryItems(HKEY_LOCAL_MACHINE, KEY_WOW64_32KEY, false, _T("Run32"), _T("所有用户 Run32"), StartupSource::HklmRun32);
	addRegistryItems(HKEY_CURRENT_USER, KEY_WOW64_64KEY, true, _T("Run"), _T("当前用户 Run（已禁用）"), StartupSource::HkcuRunDisabled);
	addRegistryItems(HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY, true, _T("Run"), _T("所有用户 Run（已禁用）"), StartupSource::HklmRunDisabled);

	auto addFolderItems = [this, &startupItemExists](const CString& folder, const CString& label, StartupSource enabledSource, StartupSource disabledSource)
		{
			if (folder.IsEmpty() || !PathFileExists(folder))
			{
				return;
			}

			const CString disabledFolder = CombinePath(folder, _T("SystemInspectorDisabledStartup"));
			CFileFind finder;
			BOOL working = finder.FindFile(CombinePath(folder, _T("*.*")));
			while (working)
			{
				working = finder.FindNextFile();
				if (finder.IsDots() || finder.IsDirectory())
				{
					continue;
				}

				StartupItem item;
				item.name = finder.GetFileName();
				item.command = finder.GetFilePath();
				item.sourceLabel = label;
				item.location = finder.GetFilePath();
				item.disabledLocation = EnsureUniqueFilePath(disabledFolder, item.name);
				item.source = enabledSource;
				item.enabled = true;
				if (startupItemExists(item))
				{
					continue;
				}
				m_startupItems.push_back(item);
			}
			finder.Close();

			if (!PathFileExists(disabledFolder))
			{
				return;
			}

			CFileFind disabledFinder;
			working = disabledFinder.FindFile(CombinePath(disabledFolder, _T("*.*")));
			while (working)
			{
				working = disabledFinder.FindNextFile();
				if (disabledFinder.IsDots() || disabledFinder.IsDirectory())
				{
					continue;
				}

				StartupItem item;
				item.name = disabledFinder.GetFileName();
				item.command = disabledFinder.GetFilePath();
				item.sourceLabel = label + _T("（已禁用）");
				item.location = disabledFinder.GetFilePath();
				item.disabledLocation = EnsureUniqueFilePath(folder, item.name);
				item.source = disabledSource;
				item.enabled = false;
				if (startupItemExists(item))
				{
					continue;
				}
				m_startupItems.push_back(item);
			}
			disabledFinder.Close();
		};

	addFolderItems(KnownFolderPath(FOLDERID_Startup), _T("当前用户启动文件夹"), StartupSource::UserStartupFolder, StartupSource::UserStartupFolderDisabled);
	addFolderItems(KnownFolderPath(FOLDERID_CommonStartup), _T("所有用户启动文件夹"), StartupSource::CommonStartupFolder, StartupSource::CommonStartupFolderDisabled);

	auto addApprovedOnlyItems = [this, &startupItemExists, &startupApprovedRecordExists](HKEY rootKey, const CString& approvedKey, const CString& label)
		{
			HKEY key = nullptr;
			if (RegOpenKeyEx(rootKey, StartupApprovedSubKey(approvedKey), 0, KEY_READ | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
			{
				return;
			}

			DWORD index = 0;
			for (;;)
			{
				TCHAR valueName[512] = {};
				BYTE data[32] = {};
				DWORD valueNameChars = _countof(valueName);
				DWORD dataBytes = sizeof(data);
				DWORD type = 0;
				const LSTATUS status = RegEnumValue(key, index, valueName, &valueNameChars, nullptr, &type, data, &dataBytes);
				if (status == ERROR_NO_MORE_ITEMS)
				{
					break;
				}
				++index;
				if (status != ERROR_SUCCESS || type != REG_BINARY || dataBytes == 0)
				{
					continue;
				}
				if (startupApprovedRecordExists(rootKey, approvedKey, valueName))
				{
					continue;
				}

				StartupItem item;
				item.name = valueName;
				item.command = _T("N/A");
				item.sourceLabel = label;
				item.source = StartupSource::ApprovedOnly;
				item.approvedOnly = true;
				item.approvedRoot = rootKey;
				item.approvedSubKey = approvedKey;
				item.enabled = data[0] != 0x03;
				if (!HasDisplayValue(item.command))
				{
					continue;
				}
				if (startupItemExists(item))
				{
					continue;
				}
				m_startupItems.push_back(item);
			}
			RegCloseKey(key);
		};

	addApprovedOnlyItems(HKEY_CURRENT_USER, _T("StartupFolder"), _T("当前用户启动文件夹"));
	addApprovedOnlyItems(HKEY_LOCAL_MACHINE, _T("StartupFolder"), _T("所有用户启动文件夹"));
	addApprovedOnlyItems(HKEY_CURRENT_USER, _T("StartupTasks"), _T("当前用户 StartupTasks"));
	addApprovedOnlyItems(HKEY_LOCAL_MACHINE, _T("StartupTasks"), _T("所有用户 StartupTasks"));
	RefreshStartupList();
}

void CMFCApplication1Dlg::RefreshStartupList()
{
	if (!::IsWindow(m_startupList.GetSafeHwnd()))
	{
		return;
	}

	m_startupList.DeleteAllItems();
	for (size_t i = 0; i < m_startupItems.size(); ++i)
	{
		const StartupItem& item = m_startupItems[i];
		const int row = m_startupList.InsertItem(static_cast<int>(i), item.name);
		m_startupList.SetItemText(row, 1, item.enabled ? _T("已启用") : _T("已禁用"));
		m_startupList.SetItemText(row, 2, item.sourceLabel);
		m_startupList.SetItemText(row, 3, item.command);
	}
	UpdateStartupButtons();
}

int CMFCApplication1Dlg::GetSelectedStartupIndex() const
{
	if (!::IsWindow(m_startupList.GetSafeHwnd()))
	{
		return -1;
	}

	POSITION pos = m_startupList.GetFirstSelectedItemPosition();
	if (pos == nullptr)
	{
		return -1;
	}
	return m_startupList.GetNextSelectedItem(pos);
}

void CMFCApplication1Dlg::UpdateStartupButtons()
{
	const int index = GetSelectedStartupIndex();
	const bool hasSelection = index >= 0 && index < static_cast<int>(m_startupItems.size());
	const bool enabled = hasSelection && m_startupItems[static_cast<size_t>(index)].enabled;
	if (::IsWindow(m_btnStartupEnable.GetSafeHwnd()))
	{
		m_btnStartupEnable.EnableWindow(hasSelection && !enabled);
		m_btnStartupDisable.EnableWindow(hasSelection && enabled);
		m_btnStartupDelete.EnableWindow(hasSelection);
	}
}

void CMFCApplication1Dlg::SetStartupStatusText(const CString& text)
{
	if (::IsWindow(m_startupStatusText.GetSafeHwnd()))
	{
		m_startupStatusText.SetWindowText(text);
	}
}

bool CMFCApplication1Dlg::SetSelectedStartupItemEnabled(bool enable)
{
	const int index = GetSelectedStartupIndex();
	if (index < 0 || index >= static_cast<int>(m_startupItems.size()))
	{
		SetStartupStatusText(_T("请先选择启动项。"));
		return false;
	}

	const StartupItem item = m_startupItems[static_cast<size_t>(index)];
	if (item.enabled == enable)
	{
		return true;
	}

	auto moveRegistryValue = [](HKEY rootKey, const CString& fromSubKey, const CString& toSubKey, const CString& valueName) -> bool
		{
			HKEY fromKey = nullptr;
			if (RegOpenKeyEx(rootKey, fromSubKey, 0, KEY_READ | KEY_SET_VALUE | KEY_WOW64_64KEY, &fromKey) != ERROR_SUCCESS)
			{
				return false;
			}

			DWORD type = 0;
			BYTE data[4096] = {};
			DWORD dataBytes = sizeof(data);
			const LSTATUS readStatus = RegQueryValueEx(fromKey, valueName, nullptr, &type, data, &dataBytes);
			if (readStatus != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ))
			{
				RegCloseKey(fromKey);
				return false;
			}

			HKEY toKey = nullptr;
			if (RegCreateKeyEx(rootKey, toSubKey, 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &toKey, nullptr) != ERROR_SUCCESS)
			{
				RegCloseKey(fromKey);
				return false;
			}

			const bool ok = RegSetValueEx(toKey, valueName, 0, type, data, dataBytes) == ERROR_SUCCESS &&
				RegDeleteValue(fromKey, valueName) == ERROR_SUCCESS;
			RegCloseKey(toKey);
			RegCloseKey(fromKey);
			return ok;
		};

	bool ok = false;
	switch (item.source)
	{
	case StartupSource::HkcuRun:
	case StartupSource::HklmRun:
	case StartupSource::HkcuRun32:
	case StartupSource::HklmRun32:
	case StartupSource::ApprovedOnly:
		if (item.approvedRoot != nullptr && HasValue(item.approvedSubKey))
		{
			ok = WriteStartupApprovedEnabled(item.approvedRoot, item.approvedSubKey, item.name, enable);
		}
		break;
	case StartupSource::HkcuRunDisabled:
		ok = moveRegistryValue(HKEY_CURRENT_USER, StartupRunSubKey(true), StartupRunSubKey(false), item.name);
		if (ok)
		{
			WriteStartupApprovedEnabled(HKEY_CURRENT_USER, _T("Run"), item.name, true);
		}
		break;
	case StartupSource::HklmRunDisabled:
		ok = moveRegistryValue(HKEY_LOCAL_MACHINE, StartupRunSubKey(true), StartupRunSubKey(false), item.name);
		if (ok)
		{
			WriteStartupApprovedEnabled(HKEY_LOCAL_MACHINE, _T("Run"), item.name, true);
		}
		break;
	case StartupSource::UserStartupFolder:
	case StartupSource::CommonStartupFolder:
		CreateDirectory(ParentDirectory(item.disabledLocation), nullptr);
		ok = MoveFileEx(item.location, item.disabledLocation, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) == TRUE;
		break;
	case StartupSource::UserStartupFolderDisabled:
	case StartupSource::CommonStartupFolderDisabled:
		ok = MoveFileEx(item.location, item.disabledLocation, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) == TRUE;
		break;
	default:
		break;
	}

	if (ok)
	{
		LoadStartupItems();
		SetStartupStatusText(enable ? _T("启动项已启用。") : _T("启动项已禁用。"));
	}
	else
	{
		SetStartupStatusText(_T("操作失败，请确认管理员权限或文件占用状态。"));
	}
	return ok;
}

bool CMFCApplication1Dlg::DeleteSelectedStartupItem()
{
	const int index = GetSelectedStartupIndex();
	if (index < 0 || index >= static_cast<int>(m_startupItems.size()))
	{
		SetStartupStatusText(_T("请先选择启动项。"));
		return false;
	}

	const StartupItem item = m_startupItems[static_cast<size_t>(index)];
	CString prompt;
	prompt.Format(_T("确认删除启动项“%s”？\r\n\r\n此操作会删除对应的注册表值或启动文件夹文件。"), item.name.GetString());
	if (AfxMessageBox(prompt, MB_ICONWARNING | MB_YESNO) != IDYES)
	{
		return false;
	}

	auto deleteRegistryValue = [](HKEY rootKey, REGSAM viewFlag, const CString& subKey, const CString& valueName) -> bool
		{
			HKEY key = nullptr;
			if (RegOpenKeyEx(rootKey, subKey, 0, KEY_SET_VALUE | viewFlag, &key) != ERROR_SUCCESS)
			{
				return false;
			}
			const bool ok = RegDeleteValue(key, valueName) == ERROR_SUCCESS;
			RegCloseKey(key);
			return ok;
		};

	bool ok = false;
	switch (item.source)
	{
	case StartupSource::HkcuRun:
		ok = deleteRegistryValue(HKEY_CURRENT_USER, KEY_WOW64_64KEY, StartupRunSubKey(false), item.name);
		DeleteStartupApprovedValue(HKEY_CURRENT_USER, _T("Run"), item.name);
		break;
	case StartupSource::HklmRun:
		ok = deleteRegistryValue(HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY, StartupRunSubKey(false), item.name);
		DeleteStartupApprovedValue(HKEY_LOCAL_MACHINE, _T("Run"), item.name);
		break;
	case StartupSource::HkcuRun32:
		ok = deleteRegistryValue(HKEY_CURRENT_USER, KEY_WOW64_32KEY, StartupRunSubKey(false), item.name);
		DeleteStartupApprovedValue(HKEY_CURRENT_USER, _T("Run32"), item.name);
		break;
	case StartupSource::HklmRun32:
		ok = deleteRegistryValue(HKEY_LOCAL_MACHINE, KEY_WOW64_32KEY, StartupRunSubKey(false), item.name);
		DeleteStartupApprovedValue(HKEY_LOCAL_MACHINE, _T("Run32"), item.name);
		break;
	case StartupSource::HkcuRunDisabled:
		ok = deleteRegistryValue(HKEY_CURRENT_USER, KEY_WOW64_64KEY, StartupRunSubKey(true), item.name);
		DeleteStartupApprovedValue(HKEY_CURRENT_USER, _T("Run"), item.name);
		break;
	case StartupSource::HklmRunDisabled:
		ok = deleteRegistryValue(HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY, StartupRunSubKey(true), item.name);
		DeleteStartupApprovedValue(HKEY_LOCAL_MACHINE, _T("Run"), item.name);
		break;
	case StartupSource::ApprovedOnly:
		if (item.approvedRoot != nullptr && HasValue(item.approvedSubKey))
		{
			DeleteStartupApprovedValue(item.approvedRoot, item.approvedSubKey, item.name);
			ok = true;
		}
		break;
	case StartupSource::UserStartupFolder:
	case StartupSource::CommonStartupFolder:
	case StartupSource::UserStartupFolderDisabled:
	case StartupSource::CommonStartupFolderDisabled:
		ok = DeleteFile(item.location) == TRUE;
		break;
	default:
		break;
	}

	if (ok)
	{
		LoadStartupItems();
		SetStartupStatusText(_T("启动项已删除。"));
	}
	else
	{
		SetStartupStatusText(_T("删除失败，请确认管理员权限或文件占用状态。"));
	}
	return ok;
}

bool CMFCApplication1Dlg::AddStartupItem(const CString& name, const CString& command, CString& errorMessage)
{
	errorMessage.Empty();
	CString itemName = Trimmed(name);
	CString itemCommand = Trimmed(command);
	if (itemName.IsEmpty())
	{
		errorMessage = _T("启动项名称不能为空。");
		return false;
	}
	if (itemCommand.IsEmpty())
	{
		errorMessage = _T("启动项路径不能为空。");
		return false;
	}

	if (PathFileExists(itemCommand))
	{
		itemCommand = QuoteCommandPath(itemCommand);
	}

	HKEY key = nullptr;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, StartupRunSubKey(false), 0, nullptr, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, nullptr, &key, nullptr) != ERROR_SUCCESS)
	{
		errorMessage = _T("无法打开当前用户 Run 注册表项。");
		return false;
	}

	const DWORD bytes = static_cast<DWORD>((itemCommand.GetLength() + 1) * sizeof(TCHAR));
	const bool ok = RegSetValueEx(key, itemName, 0, REG_SZ, reinterpret_cast<const BYTE*>(static_cast<LPCTSTR>(itemCommand)), bytes) == ERROR_SUCCESS;
	RegCloseKey(key);
	if (!ok)
	{
		errorMessage = _T("写入启动项失败。");
		return false;
	}
	WriteStartupApprovedEnabled(HKEY_CURRENT_USER, _T("Run"), itemName, true);
	return true;
}

// 按当前页面几何参数更新设置控件位置。
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

	// 与 DrawSystemSettings 保持同一套几何参数，保证绘制区域与控件命中区域一致。
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

	const int buttonLeft = buttonCardRect.left + 12;
	const int buttonGap = 8;
	const int buttonWidth = max(96, (buttonCardRect.Width() - 24 - buttonGap * 2) / 3);
	const int buttonTop = buttonCardRect.top + 7;
	m_btnApply.MoveWindow(buttonLeft, buttonTop, buttonWidth, 38, TRUE);
	m_btnReboot.MoveWindow(buttonLeft + buttonWidth + buttonGap, buttonTop, buttonWidth, 38, TRUE);
	m_btnToggleSelect.MoveWindow(buttonLeft + (buttonWidth + buttonGap) * 2, buttonTop, buttonWidth, 38, TRUE);

	const int statusLineHeight = max(20, static_cast<int>(tmSettings.tmHeight + tmSettings.tmExternalLeading + 2));
	m_statusText.MoveWindow(statusCardRect.left + 10, statusCardRect.top + 6, statusCardRect.Width() - 20, statusLineHeight, TRUE);
	m_adminHintText.MoveWindow(statusCardRect.left + 10, statusCardRect.top + 8 + statusLineHeight, statusCardRect.Width() - 20, statusLineHeight, TRUE);
}

// 根据当前页显示/隐藏设置控件，并重置设置页滚动位置。
void CMFCApplication1Dlg::UpdatePageVisibility()
{
	const bool showSettings = (m_activePage == PAGE_SYSTEM_SETTINGS);
	const int settingsCmd = showSettings ? SW_SHOW : SW_HIDE;
	const bool showSsd = (m_activePage == PAGE_SSD_INFO);
	const bool showScreen = (m_activePage == PAGE_SCREEN_INFO);
	const bool showStatus = (m_activePage == PAGE_SYSTEM_STATUS);
	const bool showException = (m_activePage == PAGE_SYSTEM_EXCEPTION);
	const bool showStartup = (m_activePage == PAGE_STARTUP_ITEMS);
	const bool showAcpi = (m_activePage == PAGE_ACPI_INFO);
	const bool showBatteryLog = (m_activePage == PAGE_BATTERY_LOG);
	const bool showPowerLog = (m_activePage == PAGE_POWER_LOG);
	const bool showUtility = (m_activePage == PAGE_UTILITY_TOOLS);
	const int startupCmd = showStartup ? SW_SHOW : SW_HIDE;
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

	if (::IsWindow(m_ssdTab.GetSafeHwnd()))
	{
		m_ssdTab.ShowWindow(showSsd ? SW_SHOW : SW_HIDE);
	}

	if (::IsWindow(m_driverTree.GetSafeHwnd()))
	{
		m_driverTree.ShowWindow(showAcpi ? SW_SHOW : SW_HIDE);
	}

	if (::IsWindow(m_startupList.GetSafeHwnd()))
	{
		m_startupList.ShowWindow(startupCmd);
		m_btnStartupEnable.ShowWindow(SW_HIDE);
		m_btnStartupDisable.ShowWindow(SW_HIDE);
		m_btnStartupDelete.ShowWindow(SW_HIDE);
		m_btnStartupRefresh.ShowWindow(SW_HIDE);
		m_labelStartupName.ShowWindow(startupCmd);
		m_labelStartupPath.ShowWindow(startupCmd);
		m_editStartupName.ShowWindow(startupCmd);
		m_editStartupPath.ShowWindow(startupCmd);
		m_btnStartupBrowse.ShowWindow(startupCmd);
		m_btnStartupAdd.ShowWindow(startupCmd);
		m_startupStatusText.ShowWindow(startupCmd);
	}

	if (::IsWindow(m_btnBatteryLogDetails.GetSafeHwnd()))
	{
		m_btnBatteryLogDetails.ShowWindow(showBatteryLog ? SW_SHOW : SW_HIDE);
		m_btnBatteryLogRefresh.ShowWindow(showBatteryLog ? SW_SHOW : SW_HIDE);
		m_btnPowerLogDetails.ShowWindow(showPowerLog ? SW_SHOW : SW_HIDE);
		m_btnPowerLogRefresh.ShowWindow(showPowerLog ? SW_SHOW : SW_HIDE);
	}

	if (showSettings)
	{
		m_scrollPos = 0;
		UpdateSettingsControlLayout();
	}
	else if (showSsd)
	{
		m_scrollPos = 0;
		UpdateSsdControlLayout();
		RefreshSsdTabs();
	}
	else if (showScreen)
	{
		m_scrollPos = 0;
	}
	else if (showStatus)
	{
		m_scrollPos = 0;
	}
	else if (showException)
	{
		m_scrollPos = 0;
		if (!m_systemExceptionLoaded && !m_systemExceptionLoading)
		{
			m_systemExceptionLoading = true;
			m_systemExceptionRows.clear();
			m_systemExceptionRows.push_back({ _T("状态"), _T("系统异常信息加载中...") });
			PostMessage(WM_APP_LOAD_SYSTEM_EXCEPTION_INFO, 0, 0);
		}
	}
	else if (showStartup)
	{
		m_scrollPos = 0;
		UpdateStartupControlLayout();
		UpdateStartupButtons();
	}
	else if (showAcpi)
	{
		m_scrollPos = 0;
		UpdateDriverControlLayout();
		if (!m_driverDetailsLoaded && !m_driverDetailsLoading)
		{
			m_driverDetailsLoading = true;
			ShowDriverDetailsLoading();
			PostMessage(WM_APP_LOAD_DRIVER_DETAILS, 0, 0);
		}
		else
		{
			RefreshDriverTree();
		}
	}
	else 	if (showBatteryLog || showPowerLog)
	{
		m_scrollPos = 0;
		UpdatePowerLogControlLayout();
		if (showBatteryLog && !m_batteryLogLoaded && !m_batteryLogLoading)
		{
			m_batteryLogLoading = true;
			m_batteryLogRows.clear();
			m_batteryLogRows.push_back({ _T("状态"), _T("电池日志生成中...") });
			PostMessage(WM_APP_LOAD_BATTERY_LOG_INFO, 0, 0);
		}
		if (showPowerLog && !m_powerLogLoaded && !m_powerLogLoading)
		{
			m_powerLogLoading = true;
			m_powerLogRows.clear();
			m_powerLogRows.push_back({ _T("状态"), _T("电源日志生成中...") });
			PostMessage(WM_APP_LOAD_POWER_LOG_INFO, 0, 0);
		}
	}
	else if (showUtility)
	{
		m_scrollPos = 0;
	}
}

// 处理 SSD 页签切换，展示对应磁盘信息。
void CMFCApplication1Dlg::OnTcnSelchangeSsdTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	const int index = m_ssdTab.GetCurSel();
	if (index >= 0 && index < static_cast<int>(m_ssdDiskRows.size()))
	{
		m_activeSsdIndex = index;
		m_ssdRows = m_ssdDiskRows[static_cast<size_t>(index)];
		m_scrollPos = 0;
		Invalidate();
	}

	*pResult = 0;
}

void CMFCApplication1Dlg::OnLvnItemchangedStartupList(NMHDR* pNMHDR, LRESULT* pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);
	UpdateStartupButtons();
	*pResult = 0;
}

void CMFCApplication1Dlg::OnNMRClickStartupList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE itemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int index = itemActivate != nullptr ? itemActivate->iItem : -1;
	if (index < 0)
	{
		CPoint clientPoint;
		GetCursorPos(&clientPoint);
		m_startupList.ScreenToClient(&clientPoint);
		LVHITTESTINFO hitTest = {};
		hitTest.pt = clientPoint;
		index = m_startupList.SubItemHitTest(&hitTest);
	}

	if (index >= 0 && index < static_cast<int>(m_startupItems.size()))
	{
		m_startupList.SetItemState(-1, 0, LVIS_SELECTED);
		m_startupList.SetItemState(index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_startupList.EnsureVisible(index, FALSE);
	}

	const int selectedIndex = GetSelectedStartupIndex();
	const bool hasSelection = selectedIndex >= 0 && selectedIndex < static_cast<int>(m_startupItems.size());
	const bool enabled = hasSelection && m_startupItems[static_cast<size_t>(selectedIndex)].enabled;

	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING | (hasSelection && !enabled ? MF_ENABLED : MF_GRAYED), ID_STARTUP_MENU_ENABLE, _T("启用"));
	menu.AppendMenu(MF_STRING | (hasSelection && enabled ? MF_ENABLED : MF_GRAYED), ID_STARTUP_MENU_DISABLE, _T("禁用"));
	menu.AppendMenu(MF_STRING | (hasSelection ? MF_ENABLED : MF_GRAYED), ID_STARTUP_MENU_DELETE, _T("删除"));
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, ID_STARTUP_MENU_REFRESH, _T("刷新"));

	CPoint screenPoint;
	GetCursorPos(&screenPoint);
	const UINT command = menu.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, screenPoint.x, screenPoint.y, this);
	switch (command)
	{
	case ID_STARTUP_MENU_ENABLE:
		SetSelectedStartupItemEnabled(true);
		break;
	case ID_STARTUP_MENU_DISABLE:
		SetSelectedStartupItemEnabled(false);
		break;
	case ID_STARTUP_MENU_DELETE:
		DeleteSelectedStartupItem();
		break;
	case ID_STARTUP_MENU_REFRESH:
		LoadStartupItems();
		SetStartupStatusText(_T("已刷新启动项列表。"));
		break;
	default:
		break;
	}

	*pResult = 0;
}

void CMFCApplication1Dlg::OnNMCustomdrawStartupList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW customDraw = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	switch (customDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
	{
		const int row = static_cast<int>(customDraw->nmcd.dwItemSpec);
		if (customDraw->iSubItem == 1 &&
			row >= 0 &&
			row < static_cast<int>(m_startupItems.size()))
		{
			const bool enabled = m_startupItems[static_cast<size_t>(row)].enabled;
			customDraw->clrText = enabled ? RGB(22, 128, 62) : RGB(185, 28, 28);
		}
		*pResult = CDRF_DODEFAULT;
		return;
	}
	default:
		break;
	}
	*pResult = CDRF_DODEFAULT;
}

void CMFCApplication1Dlg::OnNMCustomdrawDriverTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVCUSTOMDRAW customDraw = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);
	switch (customDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT:
	{
		const DWORD_PTR status = customDraw->nmcd.lItemlParam;
		if (status == 1)
		{
			customDraw->clrText = RGB(22, 128, 62);
		}
		else if (status == 2)
		{
			customDraw->clrText = RGB(185, 28, 28);
		}
		*pResult = CDRF_DODEFAULT;
		return;
	}
	default:
		break;
	}
	*pResult = CDRF_DODEFAULT;
}

void CMFCApplication1Dlg::OnBnClickedStartupEnable()
{
	SetSelectedStartupItemEnabled(true);
}

void CMFCApplication1Dlg::OnBnClickedStartupDisable()
{
	SetSelectedStartupItemEnabled(false);
}

void CMFCApplication1Dlg::OnBnClickedStartupDelete()
{
	DeleteSelectedStartupItem();
}

void CMFCApplication1Dlg::OnBnClickedStartupRefresh()
{
	LoadStartupItems();
	SetStartupStatusText(_T("启动项列表已刷新。"));
}

void CMFCApplication1Dlg::OnBnClickedStartupBrowse()
{
	CFileDialog dialog(TRUE, _T("exe"), nullptr,
		OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		_T("可执行文件 (*.exe;*.bat;*.cmd;*.lnk)|*.exe;*.bat;*.cmd;*.lnk|所有文件 (*.*)|*.*||"),
		this);
	if (dialog.DoModal() != IDOK)
	{
		return;
	}

	const CString path = dialog.GetPathName();
	m_editStartupPath.SetWindowText(path);
	CString name;
	m_editStartupName.GetWindowText(name);
	if (Trimmed(name).IsEmpty())
	{
		CString fileName = FileNameFromPath(path);
		const int dot = fileName.ReverseFind(_T('.'));
		if (dot > 0)
		{
			fileName = fileName.Left(dot);
		}
		m_editStartupName.SetWindowText(fileName);
	}
}

void CMFCApplication1Dlg::OnBnClickedStartupAdd()
{
	CString name;
	CString path;
	m_editStartupName.GetWindowText(name);
	m_editStartupPath.GetWindowText(path);

	CString error;
	if (!AddStartupItem(name, path, error))
	{
		SetStartupStatusText(error);
		return;
	}

	m_editStartupName.SetWindowText(_T(""));
	m_editStartupPath.SetWindowText(_T(""));
	LoadStartupItems();
	SetStartupStatusText(_T("启动项已添加到当前用户 Run。"));
}

void CMFCApplication1Dlg::OnBnClickedBatteryLogDetails()
{
	OpenPowerCfgReport(true);
}

void CMFCApplication1Dlg::OnBnClickedBatteryLogRefresh()
{
	RefreshPowerCfgReport(true);
}

void CMFCApplication1Dlg::OnBnClickedPowerLogDetails()
{
	OpenPowerCfgReport(false);
}

void CMFCApplication1Dlg::OnBnClickedPowerLogRefresh()
{
	RefreshPowerCfgReport(false);
}

void CMFCApplication1Dlg::EnsurePowerCfgReport(bool batteryReport)
{
	bool& loaded = batteryReport ? m_batteryLogLoaded : m_powerLogLoaded;
	CString& reportPath = batteryReport ? m_batteryLogPath : m_powerLogPath;
	if (loaded && !reportPath.IsEmpty() && PathFileExists(reportPath))
	{
		return;
	}

	LoadPowerCfgReportInformation(batteryReport);
	Invalidate();
}

void CMFCApplication1Dlg::LoadPowerCfgReportInformation(bool batteryReport)
{
	bool& loaded = batteryReport ? m_batteryLogLoaded : m_powerLogLoaded;
	CString& reportPath = batteryReport ? m_batteryLogPath : m_powerLogPath;
	std::vector<InfoRow>& rows = batteryReport ? m_batteryLogRows : m_powerLogRows;
	reportPath = GetPowerCfgReportPath(batteryReport ? _T("SystemInspector-BatteryReport.html") : _T("SystemInspector-SleepStudy.html"));
	rows.clear();
	rows.push_back({ _T("状态"), _T("正在生成日志...") });

	CString args;
	if (batteryReport)
	{
		args.Format(_T("/batteryreport /output \"%s\""), static_cast<LPCTSTR>(reportPath));
	}
	else
	{
		args.Format(_T("/sleepstudy /output \"%s\""), static_cast<LPCTSTR>(reportPath));
	}

	const bool ok = RunProcessAndWait(_T("powercfg.exe"), args, nullptr);
	rows.clear();
	loaded = true;

	CFileStatus status = {};
	if (ok && CFile::GetStatus(reportPath, status))
	{
		rows.push_back({ _T("状态"), _T("已生成") });
		rows.push_back({ _T("导出工具"), batteryReport ? _T("powercfg /batteryreport") : _T("powercfg /sleepstudy") });
		rows.push_back({ _T("日志文件"), reportPath });
		rows.push_back({ _T("文件大小"), FormatFileSize(static_cast<ULONGLONG>(status.m_size)) });
		rows.push_back({ _T("生成时间"), FormatFileTime(status.m_mtime) });
		rows.push_back({ _T("操作"), _T("点击“详情”在浏览器中打开报告；点击“刷新”会覆盖当前目录下的同名 HTML 报告") });
	}
	else
	{
		rows.push_back({ _T("状态"), _T("生成失败") });
		rows.push_back({ _T("导出工具"), batteryReport ? _T("powercfg /batteryreport") : _T("powercfg /sleepstudy") });
		rows.push_back({ _T("日志文件"), reportPath });
		rows.push_back({ _T("说明"), batteryReport ? _T("当前系统可能未提供电池，或 powercfg 未能生成电池报告。") : _T("当前系统可能不支持 Sleep Study，或 powercfg 未能生成睡眠研究报告。") });
	}
}

void CMFCApplication1Dlg::RefreshPowerCfgReport(bool batteryReport)
{
	bool& loaded = batteryReport ? m_batteryLogLoaded : m_powerLogLoaded;
	bool& loading = batteryReport ? m_batteryLogLoading : m_powerLogLoading;
	std::vector<InfoRow>& rows = batteryReport ? m_batteryLogRows : m_powerLogRows;
	loaded = false;
	if (loading)
	{
		return;
	}
	loading = true;
	rows.clear();
	rows.push_back({ _T("状态"), _T("正在刷新日志...") });
	PostMessage(batteryReport ? WM_APP_LOAD_BATTERY_LOG_INFO : WM_APP_LOAD_POWER_LOG_INFO, 0, 0);
	Invalidate();
}

void CMFCApplication1Dlg::StartSilentPreload()
{
	if (m_silentPreloadStarted)
	{
		return;
	}
	m_silentPreloadStarted = true;

	if (!m_ssdLoaded && !m_ssdLoading)
	{
		m_ssdLoading = true;
		m_ssdDiskRows.clear();
		m_ssdTabTitles.clear();
		m_ssdDiskRows.push_back({ { _T("状态"), _T("SSD信息后台加载中...") } });
		m_ssdTabTitles.push_back(_T("加载中"));
		m_ssdRows = m_ssdDiskRows.front();
		PostMessage(WM_APP_LOAD_SSD_INFO, 0, 0);
	}
	if (!m_screenLoaded && !m_screenLoading)
	{
		m_screenLoading = true;
		m_screenRows.clear();
		m_screenRows.push_back({ _T("状态"), _T("屏幕详情后台加载中...") });
		PostMessage(WM_APP_LOAD_SCREEN_INFO, 0, 0);
	}
	if (!m_systemExceptionLoaded && !m_systemExceptionLoading)
	{
		m_systemExceptionLoading = true;
		m_systemExceptionRows.clear();
		m_systemExceptionRows.push_back({ _T("状态"), _T("系统异常信息后台加载中...") });
		PostMessage(WM_APP_LOAD_SYSTEM_EXCEPTION_INFO, 0, 0);
	}
	if (!m_batteryLogLoaded && !m_batteryLogLoading)
	{
		m_batteryLogLoading = true;
		m_batteryLogRows.clear();
		m_batteryLogRows.push_back({ _T("状态"), _T("电池日志后台生成中...") });
		PostMessage(WM_APP_LOAD_BATTERY_LOG_INFO, 0, 0);
	}
	if (!m_powerLogLoaded && !m_powerLogLoading)
	{
		m_powerLogLoading = true;
		m_powerLogRows.clear();
		m_powerLogRows.push_back({ _T("状态"), _T("电源日志后台生成中...") });
		PostMessage(WM_APP_LOAD_POWER_LOG_INFO, 0, 0);
	}
	if (!m_driverDetailsLoaded && !m_driverDetailsLoading)
	{
		m_driverDetailsLoading = true;
		ShowDriverDetailsLoading();
		PostMessage(WM_APP_LOAD_DRIVER_DETAILS, 0, 0);
	}
}

void CMFCApplication1Dlg::OpenPowerCfgReport(bool batteryReport)
{
	const bool loading = batteryReport ? m_batteryLogLoading : m_powerLogLoading;
	if (loading)
	{
		AfxMessageBox(batteryReport ? _T("电池日志正在生成，请稍后再打开。") : _T("电源日志正在生成，请稍后再打开。"), MB_ICONINFORMATION | MB_OK);
		return;
	}

	EnsurePowerCfgReport(batteryReport);
	const CString& reportPath = batteryReport ? m_batteryLogPath : m_powerLogPath;
	if (reportPath.IsEmpty() || !PathFileExists(reportPath))
	{
		AfxMessageBox(batteryReport ? _T("电池日志文件尚未生成。") : _T("电源日志文件尚未生成。"), MB_ICONWARNING | MB_OK);
		return;
	}

	HINSTANCE result = ShellExecute(GetSafeHwnd(), _T("open"), reportPath, nullptr, nullptr, SW_SHOWNORMAL);
	if (reinterpret_cast<INT_PTR>(result) <= 32)
	{
		AfxMessageBox(_T("无法打开日志文件，请检查默认浏览器或文件关联。"), MB_ICONERROR | MB_OK);
	}
}

// 更新状态提示文字（颜色参数保留给后续扩展）。
void CMFCApplication1Dlg::SetStatusText(const CString& text, COLORREF color)
{
	UNREFERENCED_PARAMETER(color);
	if (::IsWindow(m_statusText.GetSafeHwnd()))
	{
		m_statusText.SetWindowText(text);
	}
}

// 为设置页控件统一配置文本与背景颜色。
HBRUSH CMFCApplication1Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	if ((m_activePage != PAGE_SYSTEM_SETTINGS && m_activePage != PAGE_STARTUP_ITEMS) || pWnd == nullptr)
	{
		return hbr;
	}

	const UINT id = static_cast<UINT>(pWnd->GetDlgCtrlID());
	if (m_activePage == PAGE_STARTUP_ITEMS)
	{
		if (id != IDC_STARTUP_STATUS &&
			id != IDC_STARTUP_NAME_LABEL &&
			id != IDC_STARTUP_PATH_LABEL &&
			id != IDC_STARTUP_NAME &&
			id != IDC_STARTUP_PATH)
		{
			return hbr;
		}
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetBkColor(UiSurfaceAlt);
		pDC->SetTextColor((id == IDC_STARTUP_NAME_LABEL || id == IDC_STARTUP_PATH_LABEL) ? UiSecondaryText : UiText);
		return (id == IDC_STARTUP_STATUS || id == IDC_STARTUP_NAME_LABEL || id == IDC_STARTUP_PATH_LABEL)
			? static_cast<HBRUSH>(m_uiBackgroundBrush.GetSafeHandle())
			: hbr;
	}

	const bool isSettingsCtrl =
		(id >= IDC_CHK_UAC && id <= IDC_CHK_WINDOWS_UPDATE) ||
		id == IDC_STATUS_TEXT ||
		id == IDC_ADMIN_HINT_TEXT;

	if (!isSettingsCtrl)
	{
		return hbr;
	}

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetBkColor(UiSurfaceAlt);
	pDC->SetTextColor(id == IDC_ADMIN_HINT_TEXT ? UiWarningText : UiText);
	return static_cast<HBRUSH>(m_uiBackgroundBrush.GetSafeHandle());
}

// 返回所有设置项复选框，便于批量遍历处理。
std::vector<CButton*> CMFCApplication1Dlg::GetOptionCheckBoxes()
{
	return {
		&m_chkUAC, &m_chkFirewall, &m_chkSecCenter, &m_chkAutoReboot,
		&m_chkCrashDump, &m_chkScreenSaver, &m_chkPower, &m_chkWindowsUpdate
	};
}

// 判断设置项是否全部处于选中状态。
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

// 批量设置全部选项勾选状态。
void CMFCApplication1Dlg::SetAllOptions(bool isChecked)
{
	for (CButton* option : GetOptionCheckBoxes())
	{
		option->SetCheck(isChecked ? BST_CHECKED : BST_UNCHECKED);
	}
}

// 刷新“全选/取消全选”按钮标题。
void CMFCApplication1Dlg::UpdateToggleSelectButton()
{
	if (::IsWindow(m_btnToggleSelect.GetSafeHwnd()))
	{
		m_btnToggleSelect.SetWindowText(AreAllOptionsSelected() ? _T("取消全选") : _T("全选"));
	}
}

// 检测当前进程是否属于管理员组。
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

// 选项变更后同步刷新全选按钮文案。
void CMFCApplication1Dlg::OnSettingsOptionChanged(UINT nID)
{
	UNREFERENCED_PARAMETER(nID);
	UpdateToggleSelectButton();
}

// 在“全选”和“取消全选”间切换当前设置项状态。
void CMFCApplication1Dlg::OnBnClickedToggleSelect()
{
	const bool selectAll = !AreAllOptionsSelected();
	SetAllOptions(selectAll);
	UpdateToggleSelectButton();
	SetStatusText(selectAll ? _T("已全选所有设置项。") : _T("已取消所有选择。"), RGB(60, 80, 110));
}

// 执行用户勾选的系统设置项并汇总结果提示。
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
	// 仅执行用户勾选项；allSuccess 汇总所有操作结果。
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

	if (AfxMessageBox(_T("设置已应用完成。部分设置需要重启系统才能生效，是否立即重启？"), MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		RunProcessAndWait(_T("shutdown.exe"), _T("-r -t 5 -c \"系统将在5秒后重启\""), nullptr);
	}
}

// 二次确认后发起系统重启命令。
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

// 启动命令行进程并同步等待结束，返回退出码是否为 0。
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
	// 以子进程退出码判断执行是否成功（0 表示成功）。
	DWORD exitCode = 1;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return exitCode == 0;
}

// 通过注册表开关启用/禁用 UAC。
bool CMFCApplication1Dlg::ApplyUAC(bool disable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	return key.SetDWORDValue(_T("EnableLUA"), disable ? 0 : 1) == ERROR_SUCCESS;
}

// 通过 netsh 同时设置公有/私有配置文件防火墙状态。
bool CMFCApplication1Dlg::ApplyFirewall(bool disable)
{
	const CString state = disable ? _T("off") : _T("on");
	return RunProcessAndWait(_T("netsh.exe"), _T("advfirewall set privateprofile state ") + state, nullptr) &&
		RunProcessAndWait(_T("netsh.exe"), _T("advfirewall set publicprofile state ") + state, nullptr);
}

// 设置安全中心通知项（杀毒/防火墙/更新）开关。
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

// 设置系统崩溃后是否自动重启。
bool CMFCApplication1Dlg::ApplyAutoReboot(bool disable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	return key.SetDWORDValue(_T("AutoReboot"), disable ? 0 : 1) == ERROR_SUCCESS;
}

// 设置崩溃时是否启用内存转储。
bool CMFCApplication1Dlg::ApplyCrashDump(bool enable)
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\CrashControl"), KEY_SET_VALUE) != ERROR_SUCCESS)
	{
		return false;
	}
	return key.SetDWORDValue(_T("CrashDumpEnabled"), enable ? 1 : 0) == ERROR_SUCCESS;
}

// 设置屏幕保护程序策略与当前用户配置。
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

// 批量调整电源超时参数（交流/电池）。
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

// 调用内置工具执行 Windows Update 禁用流程。
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

	// 释放内置工具到临时目录执行，结束后清理临时文件。
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

// 将资源释放为带指定文件名后缀的临时文件。
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

// 将 RCDATA 资源写入目标路径文件。
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

// 删除临时文件路径（空路径时忽略）。
void CMFCApplication1Dlg::DeleteTempFile(const CString& filePath)
{
	if (!filePath.IsEmpty())
	{
		DeleteFile(filePath);
	}
}

void CMFCApplication1Dlg::DrawUtilityTools(CDC& dc, const CRect& clientRect)
{
	UNREFERENCED_PARAMETER(clientRect);

	const int top = m_contentRect.top + 10;
	const int headerHeight = 84;
	const int headerGap = 14;
	const int cardGap = 14;
	const int iconSize = 64;
	const int colCount = (m_contentRect.Width() >= 680) ? 3 : 2;
	const int cardWidth = (m_contentRect.Width() - 32 - (colCount - 1) * cardGap) / colCount;
	const int textAreaHeight = 52;
	const int cardHeight = 20 + iconSize + 14 + textAreaHeight + 14;

	CRect headerRect(m_contentRect.left + 12, top, m_contentRect.right - 12, top + headerHeight);
	DrawRoundedCard(dc, headerRect, UiSubtleSurface, 12, UiBorder);

	dc.SelectObject(&m_titleFont);
	dc.SetTextColor(UiText);
	CRect titleRect(headerRect.left + 16, headerRect.top + 10, headerRect.right - 16, headerRect.top + 40);
	dc.DrawText(_T("实用工具"), titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	dc.SelectObject(&m_subtitleFont);
	dc.SetTextColor(UiSecondaryText);
	CRect subtitleRect(headerRect.left + 16, headerRect.top + 40, headerRect.right - 16, headerRect.bottom - 8);
	dc.DrawText(_T("常用硬件检测与驱动下载工具 · 点击卡片即可启动"), subtitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	const int totalWidth = colCount * cardWidth + (colCount - 1) * cardGap;
	const int startX = m_contentRect.left + (m_contentRect.Width() - totalWidth) / 2;
	int curY = headerRect.bottom + headerGap;
	int curX = startX;
	int cardIndex = 0;

	struct ToolInfo { HICON hIcon; const TCHAR* name; CRect* outRect; };
	const ToolInfo tools[] = {
		{ m_hIconCpuz,             _T("CPU-Z"),             &m_toolCpuzRect },
		{ m_hIconBattery,          _T("BatteryInfoView"),   &m_toolBatteryRect },
		{ m_hIconCoreTemp,         _T("Core Temp"),         &m_toolCoreTempRect },
		{ m_hIconHwinfo,           _T("HWiNFO64"),          &m_toolHwinfoRect },
		{ m_hIconCrystalDiskMark,  _T("CrystalDiskMark"),   &m_toolCrystalDiskMarkRect },
		{ m_hIconUsbTreeView,      _T("USBTreeView"),       &m_toolUsbTreeViewRect },
	};

	dc.SelectObject(&m_subtitleFont);

	for (const ToolInfo& tool : tools)
	{
		CRect cardRect(curX, curY, curX + cardWidth, curY + cardHeight);
		*(tool.outRect) = cardRect;
		DrawRoundedCard(dc, cardRect, UiSurfaceAlt, 10, UiBorder);

		if (tool.hIcon != nullptr)
		{
			const int iconX = cardRect.left + (cardWidth - iconSize) / 2;
			const int iconY = cardRect.top + 20;
			::DrawIconEx(dc.GetSafeHdc(), iconX, iconY, tool.hIcon, iconSize, iconSize, 0, nullptr, DI_NORMAL);
		}

		dc.SetTextColor(UiText);
		const int nameTop = cardRect.top + 20 + iconSize + 14;
		CRect nameRect(cardRect.left + 8, nameTop, cardRect.right - 8, nameTop + textAreaHeight);
		dc.DrawText(tool.name, nameRect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);

		cardIndex++;
		if (cardIndex % colCount == 0)
		{
			curX = startX;
			curY += cardHeight + cardGap;
		}
		else
		{
			curX += cardWidth + cardGap;
		}
	}

	if (cardIndex % colCount != 0)
	{
		curY += cardHeight + cardGap;
	}
	curY += 10;

	const int urlColCount = (m_contentRect.Width() >= 680) ? 2 : 1;
	const int urlCardWidth = (m_contentRect.Width() - 32 - (urlColCount - 1) * cardGap) / urlColCount;
	const int urlTotalWidth = urlColCount * urlCardWidth + (urlColCount - 1) * cardGap;
	const int urlStartX = m_contentRect.left + (m_contentRect.Width() - urlTotalWidth) / 2;

	struct UrlInfo { const TCHAR* name; CRect* outRect; };
	const UrlInfo urls[] = {
		{ _T("AMD 驱动下载"),    &m_toolAmdDriverRect },
		{ _T("Nvidia 驱动下载"), &m_toolNvidiaDriverRect },
	};

	for (int i = 0; i < urlColCount; ++i)
	{
		const int ux = urlStartX + i * (urlCardWidth + cardGap);
		CRect& r = *(urls[i].outRect);
		r = CRect(ux, curY, ux + urlCardWidth, curY + cardHeight);
		DrawRoundedCard(dc, r, UiSurfaceAlt, 10, UiBorder);

		if (m_hIconGlobe != nullptr)
		{
			const int iconX = r.left + (urlCardWidth - iconSize) / 2;
			const int iconY = r.top + 20;
			::DrawIconEx(dc.GetSafeHdc(), iconX, iconY, m_hIconGlobe, iconSize, iconSize, 0, nullptr, DI_NORMAL);
		}

		dc.SetTextColor(UiText);
		const int nameTop = r.top + 20 + iconSize + 14;
		CRect nameRect(r.left + 8, nameTop, r.right - 8, nameTop + textAreaHeight);
		dc.DrawText(urls[i].name, nameRect, DT_CENTER | DT_WORDBREAK | DT_END_ELLIPSIS);
	}
}

bool CMFCApplication1Dlg::ExtractArchiveToFolder(UINT resourceId, const CString& targetDir, const CString& expectedExe)
{
	const CString exeCheckPath = targetDir + _T("\\") + expectedExe;
	if (PathFileExists(exeCheckPath))
	{
		return true;
	}

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
	const BYTE* data = static_cast<const BYTE*>(LockResource(hLoaded));
	if (data == nullptr || size < 4)
	{
		return false;
	}

	DWORD offset = 0;
	const DWORD fileCount = *reinterpret_cast<const DWORD*>(data + offset);
	offset += sizeof(DWORD);

	SHCreateDirectoryEx(nullptr, targetDir, nullptr);

	for (DWORD i = 0; i < fileCount; ++i)
	{
		if (offset + sizeof(UINT16) > size)
		{
			return false;
		}

		const UINT16 pathLen = *reinterpret_cast<const UINT16*>(data + offset);
		offset += sizeof(UINT16);

		if (offset + pathLen * sizeof(WCHAR) + sizeof(DWORD) > size)
		{
			return false;
		}

		const CString relativePath(reinterpret_cast<const WCHAR*>(data + offset), pathLen);
		offset += pathLen * sizeof(WCHAR);

		const DWORD fileSize = *reinterpret_cast<const DWORD*>(data + offset);
		offset += sizeof(DWORD);

		if (offset + fileSize > size)
		{
			return false;
		}

		const CString fullPath = targetDir + _T("\\") + relativePath;

		const int lastSlash = fullPath.ReverseFind(_T('\\'));
		if (lastSlash >= 0)
		{
			const CString parentDir = fullPath.Left(lastSlash);
			SHCreateDirectoryEx(nullptr, parentDir, nullptr);
		}

		CFile file;
		if (!file.Open(fullPath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		{
			continue;
		}

		file.Write(data + offset, fileSize);
		file.Close();
		offset += fileSize;
	}

	return PathFileExists(exeCheckPath);
}

void CMFCApplication1Dlg::LaunchUtilityTool(UINT resourceId, const CString& folderName, const CString& exeName)
{
	TCHAR modulePath[MAX_PATH] = {};
	if (GetModuleFileName(nullptr, modulePath, MAX_PATH) == 0)
	{
		AfxMessageBox(_T("无法获取当前程序路径。"), MB_ICONERROR | MB_OK);
		return;
	}

	CString exeDir(modulePath);
	const int slash = exeDir.ReverseFind(_T('\\'));
	if (slash >= 0)
	{
		exeDir = exeDir.Left(slash);
	}

	const CString targetDir = exeDir + _T("\\") + folderName;
	const CString exePath = targetDir + _T("\\") + exeName;

	if (!ExtractArchiveToFolder(resourceId, targetDir, exeName))
	{
		CString msg;
		msg.Format(_T("无法释放 %s 所需文件。"), static_cast<LPCTSTR>(folderName));
		AfxMessageBox(msg, MB_ICONERROR | MB_OK);
		return;
	}

	HINSTANCE result = ShellExecute(nullptr, _T("open"), exePath, nullptr, targetDir, SW_SHOWNORMAL);
	if (reinterpret_cast<INT_PTR>(result) <= 32)
	{
		CString msg;
		msg.Format(_T("无法启动 %s，请确认文件是否存在。"), static_cast<LPCTSTR>(folderName));
		AfxMessageBox(msg, MB_ICONERROR | MB_OK);
	}
}
