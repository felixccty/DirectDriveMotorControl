# 目录结构

## 发布包目录

```
DirectDriveMotorControl/
├── README.md / LICENSE / CHANGELOG.md / CONTRIBUTING.md / .gitignore
├── src/
│   ├── main.cpp                   # 程序入口与单实例检查
│   ├── motor_gui.h                # motor_gui / thread_obj 类声明
│   ├── motor_gui.cpp              # 业务逻辑、Modbus 帧、故障码表
│   ├── motor_gui.ui / closeBox.ui # Qt Designer 界面
│   ├── motor_gui.qrc / logo.qrc   # 资源描述（相对路径）
│   ├── motor_gui.sln / .vcxproj / .filters / motor_gui5.rc
│   └── StdAfx.h / .cpp / resource*.h
├── resources/
│   ├── ico/                       # 运行时按钮图标
│   ├── logo.ico / felix_logo.jpg
├── docs/
│   └── images/                    # 架构图与流程图（4 张 SVG）
└── (本地按需) motor_control.ini / WindSetting.ini / Qt 运行库
```

## 关键类与成员

| 名称 | 类型 | 职责 |
|------|------|------|
| `motor_gui` | `QWidget` 子类 | 主窗口，聚合扫描、归零、配置、日志与界面交互 |
| `thread_obj` | `QObject` 子类 | 进度条线程，周期发出 `bar_signal(int)` |
| `Ui::motor_guiClass ui` | uic 生成 | 界面控件集合（表盘、进度条、日志框、按钮） |
| `m_serialPort` | `QSerialPort*` | Modbus RTU 串口通道 |
| `status_timer2` | `QTimer*` | 40ms 接收轮询定时器 |
| `running` / `stop` / `connected_aready` | `bool` | 扫描、停止与连接状态标志 |
| `ready_to_read` | `bool` | 是否处理串口应答 |
| `recvData` | `static QByteArray` | 最近一帧串口响应 |
| `ico_path` | `QString` | 由 `read_ico_path()` 取得的 exe 所在目录 |
| `scan_time` / `run_speed` / `low_speed` 等 | `float` / `int` | 扫描停留与速度参数 |

## 数据流

```
C:/motor_control.ini ──load_setting()──> UI 参数
                                        │
用户点击扫描 ──strat_connect()──> initial_modbus_connection()
                                        │
                              QSerialPort ──Modbus RTU 帧──> 直驱伺服驱动器
                                        │                       │
                              get_return_signal() <──── 应答帧 ────┘
                                        │
                    parseFaultCode() / log_record() / display_bar()
                                        │
                    send_direction() ──> C:/WindSetting.ini + Windows 事件
                                        │
                                  测风采集软件取数
```

固定路径与联机约定见 [注意事项](06-notes.md)。
