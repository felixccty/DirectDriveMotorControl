# 使用示例

## 1. 配置文件 `C:/motor_control.ini`

程序使用 Win32 INI API 与 `QSettings`，`[Setting]` 段典型内容如下（键名与源码一致）：

```ini
[Setting]
; 串口号，1 = COM1
Com_Port=1
; 速度，100 表示 10.0 度/秒（按 10 倍整数存放）
电机速度(100是10.0度/秒)=100
; 加减速度，100 表示 10.0 度/平方秒
电机加减速度(100是10.0度/平方秒)=100
; 手动纠偏脉冲数
manual_turn=0
; 每个方位扫描停留时间，30 表示 3.0 秒（按 10 倍整数存放）
Scan_time=30
```

读取逻辑见 `load_setting()` / `readSettingFloat()`；保存见 `write_setting()`。
`QSettings` 写入的字符串项位于同一 `[Setting]` 段，编码为 UTF-8。

## 2. 采集共享配置 `C:/WindSetting.ini`

```ini
[WindSetting]
; 扫描速度（SetFloat 写入，保留 1 位小数）
VADASPEED=10.0
; 当前方位角：0 / 90 / 180 / 270
WHAngle=0
; 扫描模式：4 = VAD
WorkMode=4
; 视场角
WVAngle=75
```

说明：`send_direction(degree)` 写 `WHAngle` 并触发 Windows 事件
`FELIX_WIND_TRACER_20180818`；`send_model_id(4)` 写 `WorkMode`。

## 3. Modbus RTU 指令示例

| 用途 | 请求帧（hex） | 源码变量 |
|------|---------------|----------|
| 读归零状态 | `01 03 60 41 00 01 CRC` | `hx_get_home` |
| 模式设置 | `01 06 60 60 00 01 CRC` | `hx_mod_set` |
| 使能 | `01 06 60 40 00 0F CRC` | `hx_enable` |
| 禁用 | `01 06 60 40 00 06 CRC` | `hx_disable` |
| 开始运动 | `01 06 60 40 00 1F CRC` | `hx_start_move` |
| 停止 | `01 06 60 40 01 0F CRC` | `hx_stop` |
| 位置低 16 位 | `01 06 61 01 LL LL CRC` | `set_position()` |
| 位置高 16 位 | `01 06 60 7A HH HH CRC` | `set_position()` |
| 故障查询 | `01 03 60 3F 00 01 CRC` | `clear_worning_click()` |

`add_CRC_code()` 对前 6 字节计算 CRC16（初值 0xFFFF、多项式 0xA001），
按「低字节在前」追加 2 字节校验。

## 4. 典型操作流程

1. 启动程序，界面标题为「测风电机控制」，自动调用 `strat_connect()` 连接电机。
2. 状态显示「连接成功」后，点击扫描按钮进入 `scan()`。
3. 自动执行 `back_to_zero()` 归零与 `correct_move()` 纠偏。
4. 依次到达北/东/南/西四个方位，表盘与指示灯同步刷新，并通知采集软件取数。
5. 观察进度条（`扫描进度：xx%`）与日志区；故障帧会显示中文故障描述。
6. 点击停止按钮置 `stop=true` 并写 `hx_stop`；关闭窗口时会先要求停止电机。

## 5. 日志格式

`D:/LogFile/YYYY-MM-DD.log` 每行格式：

```
2026-11-25 10:15:30 : [Info] -----连接成功-----
2026-11-25 10:15:41 : [Info] 发送:01 06 60 40 00 1F ...
2026-11-25 10:16:02 : [Info] 故障描述: 母线过流
```
