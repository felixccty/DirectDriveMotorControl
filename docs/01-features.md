# 功能说明

本文档逐项说明 `DirectDriveMotorControl` 的实际功能，函数与变量名均与源码一致。

## 1. 串口与 Modbus 连接

| 功能 | 关键函数 | 说明 |
|------|----------|------|
| 串口初始化 | `motor_gui::initial_modbus_connection()` | 波特率 9600、8 数据位、无校验、1 停止位、无流控，读缓冲 40960 |
| 帧校验 | `MODBUS_crc16()` / `add_CRC_code()` | CRC16（初值 0xFFFF、多项式 0xA001），向 6 字节指令追加 2 字节校验 |
| 握手检测 | `initial_modbus_connection()` | 下发 `01 03 60 41 00 01` 读归零状态，收到从站 0x01 应答即判连接成功 |
| 连接状态 | `ui.label_5` | 显示「连接成功」/「连接失败」/「停止扫描」等状态文本 |

## 2. 四方位步进扫描

- `motor_gui::scan()`：置 `running=true`，先 `back_to_zero()` 归零，再进入主循环。
- 方位序号：`north_=0`、`east_=1`、`south_=2`、`west_=3`；每次 `degree_change` 在 0~3 间循环。
- `set_position(int)`：把 32 位位置拆成高/低 16 位，写入 `61 01`（低）与 `60 7A`（高）寄存器。
- `set_velocity(int)`：写入 `61 02` / `60 81` 速度寄存器；扫描循环中使用固定值 `87381`。
- 脉冲约定：一圈 2²⁴ = 16,777,216 脉冲，每步 `curr_pulse -= 32768`。
- 到位后更新 `ui.dial`、`ui.label_3`（`0° / 90° / 180° / 270°`）与方位按钮高亮，并调用 `send_direction()`。
- 等待 `doubleSpinBox_wait` 设定的扫描停留时间后写 `hx_stop`，进入下一方位。
- 每累计 20000 圈（约 7 天）触发一次 `back_to_zero()` + `reset_zero()` 重新标定原点。

## 3. 归零、纠偏与使能

- `back_to_zero()`：写模式寄存器 `60 60`/`60 98`，启动回零并轮询 `01 03 60 41 00 01`，
  超时计数达 200 时重试；完成后调用 `reset_zero()` 与 `correct_move()`。
- `reset_zero()`：写 `60 98 00 23` 重置当前位置为零点。
- `correct_move()`：读取 `ui.spinBox` 的手动纠偏脉冲数，经 `set_position()` 补偿后重新归零。
- `enable_motor()` / `hx_enable` / `hx_disable` / `hx_start_move` / `hx_stop`：
  分别对应使能、禁用、启动运动与停止指令。

## 4. 故障码解析与接收

- `get_return_signal()`：40ms 定时器（`status_timer2`）触发，读取串口响应。
- `parseFaultCode(const QByteArray&)`：校验 `at(1)==0x03 && at(2)==0x02`，
  以 `(at(3)<<8)|at(4)` 组合故障码，查 `QMap<uint16_t, QString>` 映射为中文描述。
- 映射覆盖母线过流 `0x2230`、U/V 相过流、伺服欠/过压、编码器错误、堵转保护 `0x8401` 等约 80 项；
  未知码返回 `未知故障码: 0xXXXX`，无故障时返回 `无故障`。
- `ByteArrayToHexString()` / `print_send_signal()`：收发帧以十六进制记录到日志与界面。

## 5. 联机数据交换

- `send_direction(int)`：设置 `WHAngle` 到 `C:/WindSetting.ini`，并 `SetEvent()` 通知命名事件
  `FELIX_WIND_TRACER_20180818`（采集软件需使用同名事件）。
- `send_model_id(int)`：设置 `WorkMode`（1=PPI、2=CAPPI、3=RHI、4=VAD、5=DBS、6=LOS），
  扫描循环固定写 4（VAD 模式）。
- `SetFloat()` / `SetInt()`：向 `[WindSetting]` 段写入 `VADASPEED`、`WVAngle` 等共享参数。

## 6. 配置与日志

- `load_setting()` / `write_setting()`：读写 `电机速度(100是10.0度/秒)`、
  `电机加减速度(100是10.0度/平方秒)`、`manual_turn`、`Com_Port`、`Scan_time`。
- `readSettingFloat()` / `writeSettingFloat()`：Win32 INI API，浮点按 10 倍整数存储。
- `writeSettingString()` / `readSettingString()`：`QSettings` 读写 `[Setting]` 段。
- `getLogFileName()` / `writeLog()` / `log_record()`：按天生成 `D:/LogFile/YYYY-MM-DD.log`，
  级别含 `Debug / Info / Error / Warn / Fatal`。

## 7. 界面与线程

- `thread_obj::process_bar_move()` 独立线程按 `scan_time` 周期 `emit bar_signal(i)`，
  经 `display_bar(int)` 更新 `ui.progressBar` 与 `ui.label_5`（`扫描进度：xx%`）。
- `initial_ico()` 从 `ico_path + "/ico/"` 加载最小化、关闭、扫描、logo、停止图标。
- `set_mod_show()` 在 629×439 与 958×439 两种宽度间切换调试面板。
- `mousePressEvent()` / `mouseMoveEvent()` / `mouseReleaseEvent()` 实现无边框窗口拖动；
  `onButtonMinClicked()` 最小化；`closeEvent()` 在运行中先提示停止电机。
- `checkOneRunning()` 以互斥量 `motor_gui` 保证单实例。
