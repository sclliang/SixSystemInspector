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
#include <dwmapi.h>
#include <uxtheme.h>
#include <cwctype>
#include <cmath>
#include <map>
#include <set>
#include <vector>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "UxTheme.lib")

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
	constexpr int PAGE_SYSTEM_INFO = 0;
	constexpr int PAGE_SYSTEM_SETTINGS = 1;
	constexpr int PAGE_SSD_INFO = 2;
	constexpr int PAGE_SCREEN_INFO = 3;
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
	bool RunProcessCaptureOutput(const CString& executablePath, CString& output);
	CString JoinUniqueValues(const std::set<CString, std::less<>>& values, const CString& separator);
	int ExtractPhysicalDriveId(const CString& devicePath);

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

	CString ResolveEdidToolPath()
	{
		TCHAR modulePath[MAX_PATH] = {};
		GetModuleFileName(nullptr, modulePath, MAX_PATH);
		CString executableDir(modulePath);
		const int slash = executableDir.ReverseFind(_T('\\'));
		if (slash >= 0)
		{
			executableDir = executableDir.Left(slash);
		}

		CString edidToolPath = executableDir + _T("\\res\\wEdid.exe");
		if (!PathFileExists(edidToolPath))
		{
			edidToolPath = _T("D:\\source\\repos\\MFCApplication1\\res\\wEdid.exe");
		}
		return edidToolPath;
	}

	CString ResolveMonitorNameFromEdidTool()
	{
		const CString edidToolPath = ResolveEdidToolPath();
		CString output;
		if (!RunProcessCaptureOutput(edidToolPath, output))
		{
			return _T("");
		}

		CStringArray lines;
		int start = 0;
		while (start <= output.GetLength())
		{
			const int end = output.Find(_T('\n'), start);
			CString line = end >= 0 ? output.Mid(start, end - start) : output.Mid(start);
			line.Trim();
			if (!line.IsEmpty())
			{
				lines.Add(line);
			}
			if (end < 0)
			{
				break;
			}
			start = end + 1;
		}

		std::set<CString, std::less<>> monitorNames;
		for (int i = 0; i < lines.GetCount(); ++i)
		{
			const CString& line = lines.GetAt(i);
			if (line.Find(_T("Monitor Name:")) == 0)
			{
				const CString monitorName = Trimmed(line.Mid(13));
				if (HasValue(monitorName) && !IsLikelyMonitorIdToken(monitorName))
				{
					monitorNames.insert(monitorName);
				}
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

	bool RunProcessCaptureOutput(const CString& executablePath, CString& output)
	{
		output.Empty();

		SECURITY_ATTRIBUTES sa = {};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = nullptr;

		HANDLE readPipe = nullptr;
		HANDLE writePipe = nullptr;
		if (!CreatePipe(&readPipe, &writePipe, &sa, 0))
		{
			return false;
		}
		if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0))
		{
			CloseHandle(writePipe);
			CloseHandle(readPipe);
			return false;
		}

		STARTUPINFO si = {};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = writePipe;
		si.hStdError = writePipe;

		PROCESS_INFORMATION pi = {};
		CString cmdLine;
		cmdLine.Format(_T("\"%s\""), static_cast<LPCTSTR>(executablePath));
		LPTSTR mutableCmdLine = cmdLine.GetBuffer();
		const BOOL created = CreateProcess(
			nullptr,
			mutableCmdLine,
			nullptr,
			nullptr,
			TRUE,
			CREATE_NO_WINDOW,
			nullptr,
			nullptr,
			&si,
			&pi);
		cmdLine.ReleaseBuffer();
		CloseHandle(writePipe);

		if (!created)
		{
			CloseHandle(readPipe);
			return false;
		}

		WaitForSingleObject(pi.hProcess, 10000);

		CStringA rawText;
		char buffer[4096] = {};
		DWORD bytesRead = 0;
		while (ReadFile(readPipe, buffer, static_cast<DWORD>(sizeof(buffer) - 1), &bytesRead, nullptr) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			rawText += buffer;
		}

		CloseHandle(readPipe);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		output = CA2T(rawText);
		return !output.IsEmpty();
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
		// 按需求优先使用 wEdid 解析出的 Monitor Name；拿不到再降级到 WMI。
		const CString edidMonitorName = ResolveMonitorNameFromEdidTool();
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
	ON_MESSAGE(WM_APP_APPLY_SYSTEM_INFO, &CMFCApplication1Dlg::OnApplyLoadedSystemInformation)
	ON_MESSAGE(WM_APP_APPLY_SSD_INFO, &CMFCApplication1Dlg::OnApplyLoadedSsdInformation)
	ON_MESSAGE(WM_APP_APPLY_SCREEN_INFO, &CMFCApplication1Dlg::OnApplyLoadedScreenInformation)
	ON_BN_CLICKED(IDC_BTN_APPLY_SETTINGS, &CMFCApplication1Dlg::OnBnClickedApplySettings)
	ON_BN_CLICKED(IDC_BTN_REBOOT, &CMFCApplication1Dlg::OnBnClickedRebootSystem)
	ON_BN_CLICKED(IDC_BTN_TOGGLE_SELECT, &CMFCApplication1Dlg::OnBnClickedToggleSelect)
	ON_COMMAND_RANGE(IDC_OPTIONS_START, IDC_OPTIONS_END, &CMFCApplication1Dlg::OnSettingsOptionChanged)
	ON_NOTIFY(TCN_SELCHANGE, IDC_SSD_TAB, &CMFCApplication1Dlg::OnTcnSelchangeSsdTab)
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
	CenterWindowAtTwoThirdsOfScreen(*this);
	EnableWin11Chrome(GetSafeHwnd());

	BuildMainLayout();
	CreateSettingsControls();
	CreateSsdControls();
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

	CDialogEx::OnLButtonDown(nFlags, point);
}

// 处理垂直滚动条消息并更新当前滚动位置。
void CMFCApplication1Dlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_activePage != PAGE_SYSTEM_INFO &&
		m_activePage != PAGE_SYSTEM_SETTINGS &&
		m_activePage != PAGE_SSD_INFO &&
		m_activePage != PAGE_SCREEN_INFO)
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
		m_activePage != PAGE_SYSTEM_SETTINGS &&
		m_activePage != PAGE_SSD_INFO &&
		m_activePage != PAGE_SCREEN_INFO)
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

LRESULT CMFCApplication1Dlg::OnApplyLoadedSystemInformation(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	AsyncLoadResult* result = reinterpret_cast<AsyncLoadResult*>(wParam);
	if (result == nullptr)
	{
		return 0;
	}

	m_systemRows = result->systemRows;
	m_loading = false;
	if (m_activePage == PAGE_SYSTEM_INFO)
	{
		m_scrollPos = 0;
	}
	delete result;
	Invalidate();
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
	else if (type == _T("SYSTEM"))
	{
		LoadSystemInformation(false);
	}
	else if (type == _T("EDID"))
	{
		LoadScreenInformation();
	}
	else
	{
		errorMessage = _T("不支持的报告类型。仅支持：SSD、SYSTEM、EDID。");
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

void CMFCApplication1Dlg::LoadScreenInformation()
{
	const bool hasWindow = ::IsWindow(GetSafeHwnd());
	m_screenRows.clear();
	m_screenLoaded = false;

	TCHAR modulePath[MAX_PATH] = {};
	GetModuleFileName(nullptr, modulePath, MAX_PATH);
	CString executableDir(modulePath);
	const int slash = executableDir.ReverseFind(_T('\\'));
	if (slash >= 0)
	{
		executableDir = executableDir.Left(slash);
	}
	CString edidToolPath = executableDir + _T("\\res\\wEdid.exe");
	if (!PathFileExists(edidToolPath))
	{
		edidToolPath = _T("D:\\source\\repos\\MFCApplication1\\res\\wEdid.exe");
	}

	CString output;
	if (!RunProcessCaptureOutput(edidToolPath, output))
	{
		m_screenRows.push_back({ _T("状态"), _T("未能执行 wEdid.exe") });
		m_screenRows.push_back({ _T("路径"), edidToolPath });
		m_screenLoaded = true;
		m_screenLoading = false;
		if (hasWindow)
		{
			Invalidate();
		}
		return;
	}

	CStringArray lines;
	int start = 0;
	while (start <= output.GetLength())
	{
		int end = output.Find(_T('\n'), start);
		CString line = end >= 0 ? output.Mid(start, end - start) : output.Mid(start);
		line.Trim();
		if (!line.IsEmpty())
		{
			lines.Add(line);
		}
		if (end < 0)
		{
			break;
		}
		start = end + 1;
	}

	CString monitorId;
	CString edidDate;
	CString edidVersion;
	CString serialNumber;
	CString refreshRate;
	CString monitorName;
	CString checksum;
	CString fullEdidHex;
	bool inHexSection = false;

	for (int i = 0; i < lines.GetCount(); ++i)
	{
		const CString& line = lines.GetAt(i);
		if (line.Find(_T("Found value EDID")) == 0)
		{
			inHexSection = true;
			continue;
		}
		if (line.Find(_T("====== EDID Decode======")) == 0)
		{
			inHexSection = false;
			continue;
		}

		if (inHexSection)
		{
			bool looksLikeHexLine = true;
			for (int c = 0; c < line.GetLength(); ++c)
			{
				const wchar_t ch = line[c];
				const bool isHexChar = (ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F');
				if (!(isHexChar || ch == L' '))
				{
					looksLikeHexLine = false;
					break;
				}
			}
			if (looksLikeHexLine && !line.IsEmpty())
			{
				if (!fullEdidHex.IsEmpty())
				{
					fullEdidHex += _T("\r\n");
				}
				fullEdidHex += line;
			}
		}
		if (line.Find(_T("Monitor ID:")) == 0)
		{
			monitorId = Trimmed(line.Mid(11));
		}
		else if (line.Find(_T("EDID Date:")) == 0)
		{
			edidDate = Trimmed(line.Mid(10));
		}
		else if (line.Find(_T("EDID Version:")) == 0)
		{
			edidVersion = Trimmed(line.Mid(13));
		}
		else if (line.Find(_T("Serial Number:")) == 0)
		{
			serialNumber = Trimmed(line.Mid(14));
		}
		else if (line.Find(_T("Refresh Rate:")) == 0)
		{
			refreshRate = Trimmed(line.Mid(13));
		}
		else if (line.Find(_T("Monitor Name:")) == 0)
		{
			monitorName = Trimmed(line.Mid(13));
		}
		else if (line.Find(_T("EDID Checksum:")) == 0)
		{
			checksum = Trimmed(line.Mid(14));
		}
	}

	m_screenRows.push_back({ _T("状态"), _T("已读取 EDID") });
	m_screenRows.push_back({ _T("显示器名称"), HasValue(monitorName) ? monitorName : _T("N/A") });
	m_screenRows.push_back({ _T("显示器ID"), HasValue(monitorId) ? monitorId : _T("N/A") });
	m_screenRows.push_back({ _T("EDID日期"), HasValue(edidDate) ? edidDate : _T("N/A") });
	m_screenRows.push_back({ _T("EDID版本"), HasValue(edidVersion) ? edidVersion : _T("N/A") });
	m_screenRows.push_back({ _T("序列号"), HasValue(serialNumber) ? serialNumber : _T("N/A") });
	m_screenRows.push_back({ _T("刷新率"), HasValue(refreshRate) ? refreshRate : _T("N/A") });
	m_screenRows.push_back({ _T("校验和"), HasValue(checksum) ? checksum : _T("N/A") });
	if (HasValue(fullEdidHex))
	{
		m_screenRows.push_back({ _T("EDID原始数据(完整)"), fullEdidHex });
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
	AddSystemInfoRow(_T("有线网卡地址"), wiredMac);
	AddSystemInfoRow(_T("无线网卡地址"), wirelessMac);
	AddSystemInfoRow(_T("蓝牙地址"), bluetoothAddress);
	AddSystemInfoRow(_T("TPM型号"), tpmModel);

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
	DrawRoundedCard(memDc, m_settingsMenuRect, m_activePage == PAGE_SYSTEM_SETTINGS ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_ssdMenuRect, m_activePage == PAGE_SSD_INFO ? selectedColor : unselectedColor, 10);
	DrawRoundedCard(memDc, m_screenMenuRect, m_activePage == PAGE_SCREEN_INFO ? selectedColor : unselectedColor, 10);

	if (m_activePage == PAGE_SYSTEM_INFO) DrawAccentStrip(memDc, m_infoMenuRect, UiAccent);
	if (m_activePage == PAGE_SYSTEM_SETTINGS) DrawAccentStrip(memDc, m_settingsMenuRect, UiAccent);
	if (m_activePage == PAGE_SSD_INFO) DrawAccentStrip(memDc, m_ssdMenuRect, UiAccent);
	if (m_activePage == PAGE_SCREEN_INFO) DrawAccentStrip(memDc, m_screenMenuRect, UiAccent);

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

	if (m_activePage == PAGE_SYSTEM_SETTINGS)
	{
		DrawSystemSettings(memDc, clientRect);
	}
	else if (m_activePage == PAGE_SSD_INFO)
	{
		DrawSsdInformation(memDc, clientRect);
	}
	else if (m_activePage == PAGE_SCREEN_INFO)
	{
		DrawScreenInformation(memDc, clientRect);
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
	dc.DrawText(_T("基于 wEDID 接口读取 EDID 并解析显示器关键信息"), subtitleRect, DT_LEFT | DT_WORDBREAK | DT_END_ELLIPSIS);

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
	m_ssdMenuRect = CRect(m_sideRect.left + 10, m_infoMenuRect.bottom + 10, m_sideRect.right - 10, m_infoMenuRect.bottom + 62);
	m_screenMenuRect = CRect(m_sideRect.left + 10, m_ssdMenuRect.bottom + 10, m_sideRect.right - 10, m_ssdMenuRect.bottom + 62);
	m_settingsMenuRect = CRect(m_sideRect.left + 10, m_screenMenuRect.bottom + 10, m_sideRect.right - 10, m_screenMenuRect.bottom + 62);
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
