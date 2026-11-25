# 依赖配置

## 编译/运行依赖

| 类别 | 版本/组件 | 说明 |
|------|-----------|------|
| 语言 | C++11 | Debug `v142`，Release `v140` |
| IDE | Visual Studio 2019 | 桌面 C++ 开发负载 |
| Qt | 5.12.0 msvc2017_64 | 需 `QtCore` / `QtGui` / `QtWidgets` / `QtSerialPort` / `QtSerialBus` |
| Qt VS Tools | 与 VS2019 兼容版本 | 加载 `.vcxproj` 中的 Qt 设置 |
| Windows SDK | 10.0.19041.0 | 使用 INI API、注册表与命名事件 |
| 配置 | Win32 INI + QSettings | `C:/motor_control.ini`、`C:/WindSetting.ini` |
| 串口 | RS-485 / Modbus RTU | 9600-8-N-1，对应伺服驱动器寄存器映射 |

## 第三方依赖

- **Qt 官方模块**：核心逻辑只依赖 Qt 官方库，无闭源 SDK 调用。
- **VsmdLib / 伺服厂商 SDK**：源工程目录中曾附带厂商动态库与手册，
  当前控制实现直接通过 Modbus RTU 寄存器帧通信，编译不需要该库；
  如需使用厂商调试工具或私有协议，请向设备厂商获取。
- **测风采集软件**：通过 `C:/WindSetting.ini` 与命名事件
  `FELIX_WIND_TRACER_20180818` 对接，需使用相同约定。

## 系统要求

- Windows 7 / 10 x64；
- 可用串口（或 USB-RS485 转换器）与直驱电机控制器；
- 需对 `C:\` 根目录有写权限（配置文件固定路径）；
- 日志写入 `D:/LogFile`，需 D 盘可写（不存在时程序会尝试创建）。
