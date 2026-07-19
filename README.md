# SystemInspector

Windows 系统检测与系统设置优化工具（MFC / C++）。

## 功能概览

- **系统信息**
  - 主板型号、CPU 型号、GPU 型号、显示器信息
  - 内存、SSD、BIOS、EC、TPM 信息
  - 主板 SN、系统 SN、UUID
  - 有线/无线网卡地址、蓝牙地址

- **系统状态**
  - Windows 版本、Build、系统架构、安装日期、最近启动时间和运行时长
  - Windows 激活状态、系统语言、时区、Windows 目录
  - Secure Boot 和 BitLocker 状态

- **SSD 信息**
  - 基于 CAtaSmart 的多盘枚举与 SMART 状态展示
  - 支持按磁盘 Tab 切换
  - 展示型号、固件、序列号、接口类型、传输模式、温度、通电时长、通电次数、寿命、主机读写总量和设备标识
  - SSD 信息在进入页面后后台加载，避免阻塞界面

- **屏幕详情**
  - 读取当前活动显示器并解析 EDID
  - 展示显示器名称、ID、EDID 日期、版本、序列号、刷新率、校验和
  - 支持展示完整 EDID 原始十六进制数据
  - 屏幕详情后台加载，避免阻塞界面

- **系统设置**
  - UAC、Windows 防火墙、安全中心提示
  - 系统崩溃自动重启、完整内存转储、屏幕保护程序
  - 电源策略（睡眠/关屏/硬盘）
  - Windows 自动更新（基于内置 `Wub_x64.exe` + `Wub.ini`）

## 界面特性

- Win11 风格浅色界面、圆角窗口和卡片式布局
- 启动窗口自动居中，并按当前屏幕工作区的三分之二显示
- 支持最大化、最小化和窗口缩放
- 系统信息、SSD 信息、屏幕详情均使用后台线程加载，加载期间界面保持响应

## 命令行导出报告

支持通过命令行直接导出信息到文件：

```powershell
SystemInspector.exe -Report SSD "D:\Temp\ssd-report.txt"
SystemInspector.exe -Report SYSTEM "D:\Temp\system-report.txt"
SystemInspector.exe -Report STATUS "D:\Temp\status-report.txt"
SystemInspector.exe -Report EDID "D:\Temp\edid-report.txt"
```

- `SSD`：导出 SSD 信息（多盘分段）
- `SYSTEM`：导出系统信息
- `STATUS`：导出系统状态（Windows 版本、激活状态、Secure Boot、BitLocker 等）
- `EDID`：导出屏幕详情
- 导出完成后程序才会退出；如在 `cmd` 脚本里串行调用 GUI 程序，请使用 `start "" /wait SystemInspector.exe -Report ...`

## 运行要求

- Windows 10/11
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

- `x64\Release\SystemInspector.exe`

## 项目信息

- Product Name: `SystemInspector`
- Company: `Sixunited`
- Author: `Sclliang`
