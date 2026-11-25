# 运行与部署

## 1. 环境准备

- Windows 7 / 10 x64；
- Visual Studio 2019（Desktop development with C++ 工作负载）；
- Qt 5.12.0（msvc2017_64）与 Qt VS Tools 插件；
- 串口（RS-485）连接的直驱伺服驱动器与其 Modbus 寄存器手册。

## 2. 编译

1. 用 Visual Studio 打开 `src/motor_gui.sln`。
2. 确认 Qt 版本：Debug 配置使用 `5.12.0_msvc2017_64`（`v142`），
   Release 配置使用 `msvc2015_64`（`v140`）；若本机版本不同，请在项目属性中调整。
3. `src/motor_gui.vcxproj` 中的头文件路径已改为 `$(QTDIR)\include;%(AdditionalIncludeDirectories)`
   可移植占位，需确保 `QTDIR` 指向本机 Qt 安装目录。
4. 选择 `Debug | x64` 生成解决方案，输出 `motor_gui.exe`。

> 源码 `main.cpp` / `motor_gui.h` 为 **GBK**，`motor_gui.cpp` 为 **UTF-8（带 BOM）**，
> `.ui/.qrc` 为 UTF-8，`.rc/.h`(资源) 为 UTF-16 LE。请勿整体转码，详见 [注意事项](06-notes.md)。

## 3. 资源部署

1. 将 `resources/ico/` 整个目录拷贝到 `motor_gui.exe` 同目录：
   程序通过 `read_ico_path()` 取得 exe 目录，再拼接 `/ico/xxx.png` 加载图标。
2. `resources/logo.ico` 用于窗口图标与 `.rc` 资源；`resources/felix_logo.jpg` 对应 `src/logo.qrc`。
3. 若自行修改 `.qrc` / `.rc` 中的资源路径，请重新用 rcc/rc 生成后编译。

## 4. 配置与运行

1. 新建 `C:/motor_control.ini`，至少配置 `Com_Port`（串口号）、
   `电机速度(100是10.0度/秒)`、`Scan_time`（见 [使用示例](05-usage.md)）。
2. 连接电机控制器上电，确认串口号与波特率一致。
3. 运行 `motor_gui.exe`：程序检测单实例后启动，
   `main()` 调用 `strat_connect()` → 连接串口 → 启动接收定时器 → 进入扫描循环。
4. 界面按钮：扫描（`pushButton1`）、归零（`pushButton2`）、停止（`pushButton3`）、
   清除报警、调试面板展开/收起与自绘最小化/关闭。

## 5. 打包发布（Windows）

```bat
windeployqt motor_gui.exe --release
```

将 `resources/ico/`、配置文件模板与必要的 Qt 运行库一并分发。
第三方驱动 SDK 与厂商手册不在本仓库内，由用户自行获取。
