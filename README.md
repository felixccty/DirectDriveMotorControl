# DirectDriveMotorControl

> 基于 Qt 5 / C++ 的直驱电机测风扫描运动控制上位机。

[![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue.svg)](https://www.microsoft.com/windows)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Framework](https://img.shields.io/badge/framework-Qt%205-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

`DirectDriveMotorControl` 是一套面向测风雷达/激光雷达转台的**直驱电机运动控制上位机**。软件通过 `QSerialPort` 以 **Modbus RTU**（9600-8-N-1）与直驱伺服驱动器通信，完成**归零 → 纠偏 → 四方位步进扫描 → 方位角通知采集软件**的闭环控制；具备故障码中文解析、运行日志、扫描进度条与无边框交互界面。

控制参数持久化在 `C:/motor_control.ini`，并通过 `C:/WindSetting.ini` 与测风采集软件交换方位角与工作模式；方位到位时同时用 Windows 命名事件通知采集程序取数。

---

## 核心特性

- **Modbus RTU 连接**：`initial_modbus_connection()` 配置 9600/8-N-1，读取归零状态判断握手成功。
- **四方位步进扫描**：`scan()` 按 北→东→南→西 循环，`set_position()` / `set_velocity()` 下发位置与速度。
- **归零与纠偏**：`back_to_zero()` / `reset_zero()` / `correct_move()` 完成寻零、重置原点与手动脉冲补偿。
- **故障码解析**：`parseFaultCode()` 将 0x2230 等寄存器故障码映射为中文描述（约 80 项）。
- **联机通知**：`send_direction()` 写 `WHAngle` 并触发命名事件；`send_model_id()` 写 `WorkMode`。
- **配置持久化**：`load_setting()` / `write_setting()` 读写速度与加速度；`SetFloat()` / `SetInt()` 写采集共享 INI。
- **进度条线程**：`thread_obj::process_bar_move()` 经 `bar_signal(int)` 驱动 `display_bar()` 实时显示扫描进度。
- **运行日志**：`writeLog()` / `log_record()` 按日期写入 `D:/LogFile/YYYY-MM-DD.log` 并在界面显示。
- **无边框交互**：`mousePressEvent()` 等实现标题栏拖动，`set_mod_show()` 展开/收起调试面板。
- **单实例运行**：`checkOneRunning()` 通过互斥量防止程序重复启动。

---

## 技术栈

| 类别 | 选型 |
|------|------|
| 语言 | C++11（Debug 为 `v142` 工具集，Release 为 `v140`） |
| UI 框架 | Qt 5.12（Core / Gui / Widgets / SerialPort / SerialBus） |
| 开发工具 | Visual Studio 2019 + Qt VS Tools |
| 通信协议 | Modbus RTU over RS-485 串口（QSerialPort，CRC16） |
| 配置存储 | Win32 INI API + QSettings → `C:/motor_control.ini`、`C:/WindSetting.ini` |
| 操作系统 | Windows 7 / 10 x64 |
| 构建产物 | motor_gui.exe |

---

## 快速开始

```bash
git clone https://github.com/felixccty/DirectDriveMotorControl.git
cd DirectDriveMotorControl
```

1. 安装 Visual Studio 2019（C++ 桌面开发负载）与 Qt 5.12 + Qt VS Tools。
2. 用 VS 打开 `src/motor_gui.sln`，选择 `Debug | x64` 编译，输出 `motor_gui.exe`。
3. 将 `resources/ico/` 目录拷贝到可执行文件所在目录，保持 `ico/` 子目录名不变。
4. 在 `C:/motor_control.ini` 配置串口号、速度与扫描停留时间（见 [使用示例](docs/05-usage.md)）。
5. 连接直驱电机控制器，运行 `motor_gui.exe`，点击扫描按钮开始四方位扫描。

完整步骤见 [运行与部署](docs/03-build-and-run.md) 与 [依赖配置](docs/04-dependencies.md)。

---

## 目录结构

```
DirectDriveMotorControl/
├── README.md / LICENSE / CHANGELOG.md / CONTRIBUTING.md / .gitignore
├── src/                           # 核心源码（保持原编码）
│   ├── main.cpp                   # 入口：单实例检查、创建窗口、启动连接
│   ├── motor_gui.h / .cpp         # 主窗口类与业务实现、故障码映射表
│   ├── motor_gui.ui / closeBox.ui # Qt Designer 界面定义
│   ├── motor_gui.qrc / logo.qrc   # Qt 资源描述（已脱敏为相对路径）
│   ├── motor_gui.sln / .vcxproj / .filters / motor_gui5.rc
│   └── StdAfx.h / .cpp / resource*.h
├── resources/                     # 图标与 logo 资源
│   ├── ico/                       # 界面按钮图标（运行时按 ico/ 目录读取）
│   ├── logo.ico / felix_logo.jpg
├── docs/                          # 本文档体系 + images/ 示意图
└── (本地按需) motor_control.ini / WindSetting.ini / Qt 运行库
```

---

## 文档索引

| 文档 | 内容 |
|------|------|
| [功能说明](docs/01-features.md) | 串口连接、四方位扫描、归零纠偏、故障码解析与日志 |
| [目录结构](docs/02-structure.md) | 源文件职责、关键类成员与数据流 |
| [运行与部署](docs/03-build-and-run.md) | VS2019 + Qt 5.12 编译、资源配置与 windeployqt |
| [依赖配置](docs/04-dependencies.md) | Qt 模块、编译器、硬件与串口要求 |
| [使用示例](docs/05-usage.md) | INI 配置、Modbus 帧与典型操作流程 |
| [注意事项](docs/06-notes.md) | 编码、固定路径、联机约定与已知限制 |
| [架构与流程](docs/07-architecture.md) | 分层架构与 4 张流程图解 |

## 硬件与联机说明

本项目是**控制框架**，不包含电机驱动固件与测风反演算法。运行时需自备支持 Modbus RTU
寄存器映射的直驱伺服驱动器，以及按约定读取 `C:/WindSetting.ini` 与命名事件的测风采集软件。
配置示例与寄存器约定见 [使用示例](docs/05-usage.md) 与 [注意事项](docs/06-notes.md)。

---

## 许可与作者

- **许可**：[MIT License](LICENSE)
- **作者 / 项目归属**：Felix
- **仓库**：https://github.com/felixccty/DirectDriveMotorControl

本项目以学习和工程交流为目的开源，欢迎 Issue 与 Pull Request。使用前请自行评估其在目标场景下的适用性与安全性。
