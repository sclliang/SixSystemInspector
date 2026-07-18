# SixSystemInspector

Windows 系统检测与系统设置优化工具（MFC / C++）。

## 功能概览

- **系统信息**
  - 主板型号
  - CPU 型号
  - GPU 型号
  - 显示器型号
  - 内存信息
  - SSD 信息（多盘）
  - BIOS 版本
  - EC 版本
  - 主板 SN / 系统 SN / UUID
  - 有线/无线网卡地址
  - 蓝牙地址
  - TPM 型号

- **系统设置**
  - UAC、Windows 防火墙、安全中心提示
  - 系统崩溃自动重启、完整内存转储、屏幕保护程序
  - 电源策略（睡眠/关屏/硬盘）
  - Windows 自动更新（基于内置 `Wub_x64.exe` + `Wub.ini`）

## 运行要求

- Windows 10/11
- Visual Studio 2022（含 MFC）
- 程序使用 `requireAdministrator` 清单，双击运行会弹出 UAC

## 构建

### Visual Studio
1. 打开 `MFCApplication1.slnx`
2. 选择 `Release | x64`
3. 生成项目

### 命令行（MSBuild）
```powershell
MSBuild.exe .\MFCApplication1.vcxproj /p:Configuration=Release /p:Platform=x64
```

## 输出文件

- `x64\Release\SixSystemInspector.exe`

## 项目信息

- Product Name: `SixSystemInspector`
- Company: `Sixunited`
- Author: `Sclliang`

