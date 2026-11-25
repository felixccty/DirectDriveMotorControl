# 注意事项与已知限制

## 1. 源码编码

| 文件 | 编码 |
|------|------|
| `main.cpp`、`motor_gui.h` | GBK |
| `motor_gui.cpp` | UTF-8（带 BOM） |
| `motor_gui.ui`、`closeBox.ui`、`.qrc`、`.sln`、`.vcxproj`、`.filters` | UTF-8 |
| `motor_gui5.rc`、`resource6.h` | UTF-16 LE |
| `StdAfx.h/.cpp`、`resource.h` | GBK / UTF-8 混合（保留原样） |

请使用支持上述编码的编辑器修改源码；整体转码会导致中文注释与界面字符串乱码。

## 2. 固定路径常量

| 路径 | 用途 | 位置 |
|------|------|------|
| `C:/motor_control.ini` | 控制参数配置 | `motor_gui.cpp` 顶部 `lpPath` / `setting_Path_str` |
| `C:/WindSetting.ini` | 与采集软件共享的参数 | `SetFloat()` / `SetInt()` |
| `D:/LogFile/` | 日志目录（`mkdir` 自动创建） | `getLogFileName()` |
| `ico_path + "/ico/"` | 运行期图标目录 | `read_ico_path()` / `initial_ico()` |

这些是程序约定的运行期常量，已保留；如需改到其它盘符，请修改源码后重新编译。

## 3. 联机约定

- 命名事件 `FELIX_WIND_TRACER_20180818` 用于通知采集软件读取方位角，
  采集程序必须使用**完全相同**的事件名（源码中该常量已由私有名称统一脱敏为 Felix 前缀）。
- `WorkMode=4` 表示 VAD 扫描模式；`WHAngle` 取值 0/90/180/270 对应北/东/南/西。
- 串口参数为 9600-8-N-1；若驱动器参数不同，需在 `initial_modbus_connection()` 中调整。

## 4. 硬件与安全

- 本项目**不包含电机驱动固件与位置环算法**，仅按寄存器映射下发控制帧；
  实际运动行为取决于所接伺服驱动器。
- 扫描前请确认转台周围无人及障碍物，调试运行建议先断开机械负载。
- 归零超时（计数 > 200）会自动重试；仍失败时请检查接线、波特率与驱动器报警状态。
- 关闭窗口时若仍在扫描，程序会忽略关闭事件并提示先停止电机。

## 5. 已知限制

- 进度条线程与主循环通过全局 `start_process_bar` 标志耦合，扫描停留时间由界面参数控制；
- 日志按日期追加写入，长期运行需自行清理 `D:/LogFile/`；
- Release 配置使用较旧的 `v140` 工具集与 `msvc2015_64` Qt 版本，建议优先使用 Debug|x64 配置；
- 源码中的 `AutoPowerOn()` / `CanclePowerOn()` 为开机自启示例，当前在构造函数中已注释，默认不启用；
- 仓库不含第三方驱动库与硬件手册，使用前请自行获取并核对寄存器映射。
