#include "motor_gui.h"

#include "stdafx.h"
#include "windows.h"
#include <cstdlib>
#include <string>
#include <list>
#include <direct.h>

#include <cstring>
#include <QThread>
using namespace std;
#include <conio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <QAction>
#include <Qmenu>
#include<QSettings>
#include <QTextCodec>
#include <QSerialPort>
#include<QTimer.h>
static HANDLE s_hEnventAquire = INVALID_HANDLE_VALUE;
LPCTSTR lpPath = L"C://motor_control.ini";
QString setting_Path_str = "C://motor_control.ini";
#define FLIPW(h) ((((uint16_t)h << 8)&0xFF00) | ((uint16_t)h >> 8))
#ifdef WIN32

#pragma execution_character_set("utf-8")

#endif

bool stop = false;
bool close_window = true;
bool only_search_north = true;
bool search_N_stop = false;
int run_speed = 100000;//常规速度120000，默认100000
int noraml_speed= 400000;//常规速度120000 ，1200000会抖
int low_speed = 120000;
int fast_speed = 120000;
int accelerate = 120000; //常规速度120000
int sleep_15 = 960000;
float scan_time = 3.0;
bool start_process_bar = false;
static QByteArray recvData;
bool ready_to_read = false;
bool close_check = false;


typedef enum enLogLevel {
	LOG_DEBUG = 1,
	LOG_INFO,
	LOG_ERROR,
	LOG_WARN,
	LOG_FATAL
}LogLevel;
const char LogTip[][8] = { "","Debug","Info","Error","Warn","Fatal" };
//获取日志名
void getLogFileName(string &logFileName)
{
	time_t t = time(0);
	char tmp[32] = { NULL };
	strftime(tmp, sizeof(tmp), "%Y-%m-%d", localtime(&t));
	strcat(tmp, ".log");

	string folderPath = "D:/LogFile";

	if (0 != folderPath.c_str())
	{
		mkdir(folderPath.c_str());   // 返回 0 表示创建成功，-1 表示失败
	}

	char path[50] = "D:/LogFile/";
	strcat(path, tmp);
	logFileName = path;
}
//写日志
void writeLog(int logLevel, const char *pLogFormat)
{
	string logFileName;
	getLogFileName(logFileName);
	std::ofstream    OsWrite(logFileName, std::ofstream::app);
	char logTxt[2048];
	memset(logTxt, 0, sizeof(logTxt));
	time_t t = time(0);
	char date[32] = { NULL };
	strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&t));
	sprintf(logTxt, "%s : [%s] %s", date, LogTip[logLevel], pLogFormat);
	OsWrite << logTxt;
	OsWrite << std::endl;
	OsWrite.close();
}
//string转QString
QString str2qstr(const string str)
{
	return QString::fromLocal8Bit(str.data());
}


// 在 motor_gui.cpp 中实现
QString motor_gui::parseFaultCode(const QByteArray& recvData)
{
	// 检查数据长度和格式
	if (recvData.size() < 5) {
		return QString::fromUtf8("响应数据长度不足");
	}
	if (recvData.at(1) != 0x03 || recvData.at(2) != 0x02) {
		return QString::fromUtf8("非故障码查询响应");
	}

	uint16_t faultCode = (static_cast<uint8_t>(recvData.at(3)) << 8) |
		static_cast<uint8_t>(recvData.at(4));

	// 静态映射表，使用 QString::fromUtf8 构建
	static QMap<uint16_t, QString> faultMap = []() {
		QMap<uint16_t, QString> map;
		map[0x2230] = QString::fromUtf8("母线过流");
		map[0x2310] = QString::fromUtf8("U相过流");
		map[0x2311] = QString::fromUtf8("V相过流");
		map[0x2320] = QString::fromUtf8("硬件短路");
		map[0x3220] = QString::fromUtf8("伺服欠压");
		map[0x3210] = QString::fromUtf8("伺服过压");
		map[0x4110] = QString::fromUtf8("环境过温");
		map[0x4120] = QString::fromUtf8("环境温度过低");
		map[0x4310] = QString::fromUtf8("功率模块过温");
		map[0x8482] = QString::fromUtf8("超过最大转速");
		map[0x8483] = QString::fromUtf8("速度跟踪误差过大");
		map[0x8611] = QString::fromUtf8("位置偏差过大");
		map[0x7380] = QString::fromUtf8("编码器连接错误");
		map[0x7383] = QString::fromUtf8("编码器多圈信息错误");
		map[0x7385] = QString::fromUtf8("编码器计数错误");
		map[0x7389] = QString::fromUtf8("编码器计数溢出错误");
		map[0x738A] = QString::fromUtf8("编码器通信CRC错误");
		map[0x738B] = QString::fromUtf8("编码器Delimiter错误");
		map[0x3221] = QString::fromUtf8("PWM驱动异常");
		map[0x8612] = QString::fromUtf8("超出位置极限");
		map[0x7384] = QString::fromUtf8("编码器过温");
		map[0x6280] = QString::fromUtf8("轮廓值设定错误");
		map[0x6281] = QString::fromUtf8("终止速度设定错误");
		map[0x6282] = QString::fromUtf8("终止速度设定错误(目标位置太近)");
		map[0x6283] = QString::fromUtf8("软件限位设置错误");
		map[0x6284] = QString::fromUtf8("位置范围限制设置错误");
		map[0x6285] = QString::fromUtf8("规划曲线类型设定错误(不支持)");
		map[0x6286] = QString::fromUtf8("规划曲线类型设定错误(不支持)");
		map[0x6287] = QString::fromUtf8("扭矩规划曲线类型设定错误");
		map[0x6288] = QString::fromUtf8("寻零模式设置错误(极限开关意外触发)");
		map[0x6289] = QString::fromUtf8("寻零模式设置错误(不支持)");
		map[0x628B] = QString::fromUtf8("寻零过程超时");
		map[0x628C] = QString::fromUtf8("规划Jerk-limited ramp时初始速度不为零");
		map[0x6180] = QString::fromUtf8("规划曲线执行时间小于0");
		map[0x6181] = QString::fromUtf8("设置停止速度大于初始速度");
		map[0x6182] = QString::fromUtf8("多点持续运动未设定位置、速度、加减速度值");
		map[0x6184] = QString::fromUtf8("回零方式内部状态转换错误");
		map[0x7124] = QString::fromUtf8("电机过温");
		map[0x3130] = QString::fromUtf8("缺相错误");
		map[0x8700] = QString::fromUtf8("同步错误");
		map[0x738C] = QString::fromUtf8("霍尔错误");
		map[0x6551] = QString::fromUtf8("目标速度值设置错误(位置控制时)");
		map[0x6552] = QString::fromUtf8("位置控制时加减速设定错误");
		map[0x6553] = QString::fromUtf8("位置轨迹规划周期值设定错误");
		map[0x7320] = QString::fromUtf8("Z脉冲重复定位位置错误");
		map[0x8620] = QString::fromUtf8("使能自动校准失败");   // 注意：0x8620有两个，这里使用第一个
		map[0x6542] = QString::fromUtf8("位置模式规划的减速度或快停减速度为0");
		map[0x6572] = QString::fromUtf8("速度模式规划的减速度或快停减速度为0");
		map[0x9100] = QString::fromUtf8("DI外部设定输入报警");
		map[0x8900] = QString::fromUtf8("I2T保护告警");
		map[0x8901] = QString::fromUtf8("未校准使能运行告警");
		map[0x8010] = QString::fromUtf8("角度辨识时位置反馈抖动");
		map[0x8020] = QString::fromUtf8("角度辨识时转子不动作(负载过大等)");
		map[0x8030] = QString::fromUtf8("角度辨识动作过大电流");
		map[0x8040] = QString::fromUtf8("角度辨识超时");
		map[0x8102] = QString::fromUtf8("相序检测时电机几乎没有转动");
		map[0x8104] = QString::fromUtf8("霍尔状态反馈异常");
		map[0x8105] = QString::fromUtf8("惯量辨识转速上升超时");
		map[0x8106] = QString::fromUtf8("负载惯量辨识结果为负值");
		map[0x8107] = QString::fromUtf8("负载惯量辨识未收敛");
		map[0x8201] = QString::fromUtf8("电机缺相");
		map[0x6562] = QString::fromUtf8("轮廓扭矩模式斜率设置错误");
		map[0x6189] = QString::fromUtf8("分频输出配置FPGA反馈错误");
		map[0x8610] = QString::fromUtf8("软着陆力控超压");
		// 0x8620 已存在，此处跳过重复
		map[0xF004] = QString::fromUtf8("碰撞检测告警");
		map[0x6190] = QString::fromUtf8("位置比较输出配置FPGA反馈错误");
		map[0x810C] = QString::fromUtf8("极对数辨识异常");
		map[0x8109] = QString::fromUtf8("参数辨识超时");
		map[0x810D] = QString::fromUtf8("反电势转矩系数辨识异常");
		map[0xDC02] = QString::fromUtf8("编码器断线报警");
		map[0x810E] = QString::fromUtf8("惯量辨识过程或结果异常");
		map[0x6501] = QString::fromUtf8("回零高低速设置异常");
		map[0x6502] = QString::fromUtf8("回零加速度设置异常");
		map[0x6191] = QString::fromUtf8("配置2026开启脉冲功能后FPGA通信握手失败");
		map[0x6192] = QString::fromUtf8("脉冲指令接收数据异常");
		map[0x6193] = QString::fromUtf8("脉冲指令接收数据校验错误");
		map[0x8401] = QString::fromUtf8("电机堵转保护");
		map[0x8301] = QString::fromUtf8("输入正余弦信号异常");
		map[0x8306] = QString::fromUtf8("零漂计算异常");
		map[0x8203] = QString::fromUtf8("断线故障(动力线)");
		map[0x738F] = QString::fromUtf8("编码器电池错误");
		map[0x7386] = QString::fromUtf8("编码器电池欠压");
		map[0x8441] = QString::fromUtf8("电机线断线报警");
		map[0x0000] = QString::fromUtf8("无故障");
		return map;
	}();

	if (faultMap.contains(faultCode)) {
		return faultMap[faultCode];
	}
	else {
		return QString::fromUtf8("未知故障码: 0x%1").arg(faultCode, 4, 16, QChar('0'));
	}
}
//读取所有图标路径
QString read_ico_path() {
	string strPath;
	char buffer[MAX_PATH];
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH在WINDEF.h中定义了，等于260  
	memset(exeFullPath, 0, MAX_PATH);
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, exeFullPath, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, exeFullPath, -1, buffer, iLength, NULL, NULL);
	strPath = (string)buffer;
	int pos = strPath.find_last_of('\\', strPath.length());
	strPath = strPath.substr(0, pos);
	QString ico_path_Qstr = str2qstr(strPath);
	return ico_path_Qstr;
}
//读取设置float格式
float readSettingFloat(LPCTSTR lpcName, int defalut_v,int decimal) {
	float saved_value;
	int saved_value_int;
	saved_value_int = GetPrivateProfileInt(L"Setting", lpcName, defalut_v, lpPath);
	saved_value = float(saved_value_int) / float(decimal);
	return saved_value;
}
//读取设置float格式
float readSettingFloat_new(LPCTSTR lpcName, string defalut_v) {
	float saved_value;
	//string value_str;
	//char value[16];
	//value_str = GetPrivateProfileString("Setting", lpcName, value, sizeof(value), lpPath);
	
	return saved_value;
}
//写入设置float格式
void writeSettingFloat(LPCTSTR lpcName, float sValue, int decimal) {
	int sValue_f;
	sValue_f = int(sValue * float(decimal));
	WCHAR strVal[128] = { 0 };
	swprintf(strVal, L"%d", sValue_f);
	WritePrivateProfileString(L"Setting", lpcName, strVal, lpPath);
}
//写入设置String格式
void writeSettingString(QString lpcName, QString save_string) {

	QSettings* settings = new QSettings(setting_Path_str, QSettings::IniFormat);
	settings->setIniCodec(QTextCodec::codecForName("UTF8"));
	settings->beginGroup("Setting");
	settings->setValue(lpcName, save_string);
	settings->endGroup();

}
//写入给采集软件读取的设置int格式
void writeSettingInt(LPCTSTR lpszName, int nValue)
{
	WCHAR strVal[128] = { 0 };
	swprintf(strVal, L"%d", nValue);
	WritePrivateProfileString(L"Setting", lpszName, strVal, lpPath);
}
QString readSettingString(QString lpcName) {
	QSettings* settings = new QSettings(setting_Path_str, QSettings::IniFormat);
	settings->setIniCodec(QTextCodec::codecForName("UTF8"));
	settings->beginGroup("Setting");
	QString read_str = settings->value(lpcName).toString();
	settings->endGroup();
	return read_str;
}
//保存浮点
void SetFloat(LPCTSTR lpszName, float fValue)
{
	WCHAR strVal[128] = { 0 };
	swprintf(strVal, L"%.1f", fValue);

	WritePrivateProfileString(L"WindSetting", lpszName, strVal, L"C:\\WindSetting.ini");
}
//保存整数
void SetInt(LPCTSTR lpszName, int nValue)
{
	WCHAR strVal[128] = { 0 };
	swprintf(strVal, L"%d", nValue);

	WritePrivateProfileString(L"WindSetting", lpszName, strVal, L"C:\\WindSetting.ini");
}
//读取速度和加点速度参数
void motor_gui::load_setting() {
	float speed_set,acc_set;
	speed_set=readSettingFloat(L"电机速度(100是10.0度/秒)",100 ,10);
	acc_set = readSettingFloat(L"电机加减速度(100是10.0度/平方秒)",100 ,10);
	ui.doubleSpinBox_speed->setValue(speed_set);
	ui.doubleSpinBox_acc->setValue(acc_set);

	int curr_angle, com_port;
	curr_angle = GetPrivateProfileInt(L"Setting", L"manual_turn", 0, lpPath);
	ui.spinBox->setValue(curr_angle);
	com_port = GetPrivateProfileInt(L"Setting", L"Com_Port", 1, lpPath);
	ui.comboBox->setCurrentIndex(com_port - 1);


}
void motor_gui::write_setting() {

	float speed_set, acc_set;
	speed_set = ui.doubleSpinBox_speed->value();
	acc_set = ui.doubleSpinBox_acc->value();
	SetFloat(L"VADASPEED", speed_set);
	int com_int;
	com_int = ui.comboBox->currentIndex();
	com_int = com_int + 1;
	//读取设置参数
	WCHAR strVal[3] = { 0 };
	swprintf(strVal, L"%d", com_int);
	WritePrivateProfileString(L"Setting", L"Com_Port", strVal, lpPath);

	double scan_time;
	int scan_time_int;
	scan_time = ui.doubleSpinBox_wait->value();
	scan_time_int = int(scan_time * 10);
	WCHAR strtime[8] = { 0 };
	swprintf(strtime, L"%d", scan_time_int);
	WritePrivateProfileString(L"Setting", L"Scan_time", strtime, lpPath);
	writeSettingFloat(L"电机速度(100是10.0度/秒)", speed_set, 10);
	writeSettingFloat(L"电机加减速度(100是10.0度/平方秒)", acc_set, 10);
	/*
	
	if (speed_set >= 360) {
		speed_set = 360;
		writeSettingFloat(L"电机速度(100是10.0度/秒)", speed_set, 10);
	}
	if (acc_set >= 360) {
		acc_set = 360;
		writeSettingFloat(L"电机加减速度(100是10.0度/平方秒)", acc_set, 10);
	}
	run_speed = int(speed_set * 89600 / 360);
	accelerate = int(acc_set * 89600 / 360);
	*/

}

void motor_gui::sleep_timer(int ms) {
	for (int i = 0; i < ms; i++) {
		Sleep(1);
		qApp->processEvents();
	}
}
//开机自动启动
void AutoPowerOn(){
	HKEY hKey;
	//std::string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	//1、找到系统的启动项  
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) ///打开启动项       
{
		//2、得到本程序自身的全路径
		TCHAR strExeFullDir[MAX_PATH];
		GetModuleFileName(NULL, strExeFullDir, MAX_PATH);
		//3、判断注册表项是否已经存在
		TCHAR strDir[MAX_PATH] = {};
		DWORD nLength = MAX_PATH;
		long result = RegGetValue(hKey, nullptr, L"Motor_start", RRF_RT_REG_SZ, 0, strDir, &nLength);
		//4、已经存在
		if (result != ERROR_SUCCESS || wcscmp(strExeFullDir, strDir) != 0)
		{

			//5、添加一个子Key,并设置值，"GISRestart"是应用程序名字（不加后缀.exe） 

			RegSetValueEx(hKey, L"Motor_start", 0, REG_SZ, (LPBYTE)strExeFullDir, (lstrlen(strExeFullDir) + 1) * sizeof(TCHAR));

			//6、关闭注册表

			RegCloseKey(hKey);
		}
	}
}
//取消当前程序开机启动
void CanclePowerOn(){
	HKEY hKey;

	//std::string strRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	//1、找到系统的启动项  
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS){
		//2、删除值
		RegDeleteValue(hKey, L"Motor_start");
		//3、关闭注册表
		RegCloseKey(hKey);
	}
}
//发送给采集度数
void send_direction(int curr_angle)
{
	//char dir_char[20];
	//sprintf(dir_char, "扫描度数:%d", curr_angle);
	//writeLog(LOG_INFO, dir_char);
	if (s_hEnventAquire == INVALID_HANDLE_VALUE) {
		s_hEnventAquire = ::CreateEvent(NULL, FALSE, FALSE, L"FELIX_WIND_TRACER_20180818");
	}
	SetInt(L"WHAngle", curr_angle);
	if (s_hEnventAquire != INVALID_HANDLE_VALUE)
		::SetEvent(s_hEnventAquire);

}
typedef unsigned short uint_16;
typedef unsigned char uint_8;
//uint8_t 转 uint16_t
uint16_t MODBUS_crc16(uint8_t* ptr, uint16_t len)
{
	unsigned char i;
	unsigned short crc = 0xFFFF;
	if (len == 0) {
		len = 1;
	}
	while (len--) {
		crc ^= *ptr;
		for (i = 0; i < 8; i++) {
			if (crc & 1) {
				crc >>= 1;
				crc ^= 0xA001;
			}
			else {
				crc >>= 1;
			}
		}
		ptr++;
	}
	return(crc);
}
//发送扫描模式id给采集软件
//model 1.ppi 2.cappi 3.rhi 4.vad 5.dbs 6.LOS
void send_model_id(int model) {
	SetInt(L"WorkMode", model);
	
}
//长睡眠但不卡
void motor_gui::long_sleep(int ms) {

	for (int i = 0; i < ms / 10;i++) {
		qApp->processEvents();
		Sleep(10);
		if (stop) {
			break;
		}
	}
}
//记录日志
void motor_gui::log_record(QString log_data) {
	std::string str = log_data.toStdString();
	const char* log_char = str.c_str();
	writeLog(LOG_INFO, log_char);
	ui.textBrowser->append(log_data);
	
	qApp->processEvents();
}
//CRC验证码     迷你测风直驱电机：131072一圈 /4=32768 
QByteArray add_CRC_code(QByteArray data_code) {
	QByteArray result_code;
	QByteArray ba = data_code;
	result_code = ba;
	uint8_t data[6];
	memcpy(&data, ba.mid(0, 6), 6);
	uint16_t crc_res = MODBUS_crc16(data, 6);
	//crc_res=FLIPW(crc_res);
	uint8_t low = (uint8_t)((crc_res & 0xFF00) >> 8);
	uint8_t high = (uint8_t)(crc_res & 0x00FF);
	result_code.append(high);
	result_code.append(low);
	return result_code;
}
//设置模式，使能开关，开始运动信号
QByteArray hx_mod_set = QByteArray::fromHex("01 06 60 60 00 01 56 14");
QByteArray hx_enable_before = QByteArray::fromHex("01 06 60 40 00 07 D7 DC");

QByteArray hx_enable = QByteArray::fromHex("01 06 60 40 00 0F D6 1A");
QByteArray hx_disable = QByteArray::fromHex("01 06 60 40 00 06 16 1C");
QByteArray hx_move_low = QByteArray::fromHex("01 06 61 01 07 D0 C4 5A");
QByteArray hx_move_high = QByteArray::fromHex("01 06 60 7A 00 00 B6 13");
//4方位，位置设置信号 (测试用)
QByteArray hx_move_low_0 = QByteArray::fromHex("01 06 61 01 00 00 C7 F6");
QByteArray hx_move_high_0 = QByteArray::fromHex("01 06 60 7A 00 00 B6 13");
QByteArray hx_move_low_90 = QByteArray::fromHex("01 06 61 01 80 00 A6 36");
QByteArray hx_move_high_90 = QByteArray::fromHex("01 06 60 7A 00 00 B6 13");
QByteArray hx_move_low_180 = QByteArray::fromHex("01 06 61 01 00 00 C7 F6");
QByteArray hx_move_high_180 = QByteArray::fromHex("01 06 60 7A 00 01 77 D3");
QByteArray hx_move_low_270 = QByteArray::fromHex("01 06 61 01 80 00 A6 36");
QByteArray hx_move_high_270 = QByteArray::fromHex("01 06 60 7A 00 01 77 D3");



//速度和加减速设置信号
QByteArray hx_speed_low = QByteArray::fromHex("01 06 61 02 80 00 56 36");
QByteArray hx_speed_high = QByteArray::fromHex("01 06 60 81 00 02 46 23");
QByteArray hx_acc_low = QByteArray::fromHex("01 06 61 05 55 34 B8 B0");
QByteArray hx_acc_high = QByteArray::fromHex("01 06 60 83 00 21 A6 3A");
QByteArray hx_dec_low = QByteArray::fromHex("01 06 61 06 55 34 48 B0");
QByteArray hx_dec_high = QByteArray::fromHex("01 06 60 84 00 21 17 FB");
//开始，结束，零位，报警清除信号
QByteArray hx_start_move = QByteArray::fromHex("01 06 60 40 00 1F D7 D6");
QByteArray hx_stop = QByteArray::fromHex("01 06 60 40 01 0F D7 8A");


QSerialPort* m_serialPort;
//设置位置
void motor_gui::set_position(int next_position) {
	QByteArray position_low,position_high;
	  position_low= QByteArray::fromHex("01 06 61 01 00 00");
	position_high = QByteArray::fromHex("01 06 60 7A 00 00");
	//int a = next_position;
	uint32_t pos_32 = (uint32_t)next_position;
	uint8_t high_first = (uint8_t)((pos_32 & 0xFF000000) >> 24);
	uint8_t high_second = (uint8_t)((pos_32 & 0x00FF0000) >> 16);
	uint8_t low_first = (uint8_t)((pos_32 & 0x0000FF00) >> 8);
	uint8_t low_second = (uint8_t)(pos_32 & 0x000000FF);

	position_low[4] = low_first;
	position_low[5] = low_second;
	position_high[4] = high_first;
	position_high[5] = high_second;
	position_low = add_CRC_code(position_low);
	position_high = add_CRC_code(position_high);

	m_serialPort->write(position_low);
	qApp->processEvents();
	Sleep(200);
	m_serialPort->write(position_high);
	qApp->processEvents();
	Sleep(200);

	/*
	QByteArray hx_start_move_temp;
	hx_start_move_temp =add_CRC_code("01 06 60 40 00 1F");

	int a=-180149915;
	uint32_t b = (uint32_t)a;
	uint8_t high_first = (uint8_t)((b & 0xFF000000) >> 24);
	uint8_t high_second = (uint8_t)((b & 0x00FF0000) >> 16);
	uint8_t low_first = (uint8_t)((b & 0x0000FF00) >> 8);
	uint8_t low_second = (uint8_t)(b & 0x000000FF);

	hx_start_move_temp[4] = low_first;
	hx_start_move_temp[5] = low_second;
	*/
}
//设置位置
void motor_gui::set_velocity(int running_speed) {
	QByteArray velocity_low, velocity_high;
	velocity_low = QByteArray::fromHex("01 06 61 02 00 00");
	velocity_high = QByteArray::fromHex("01 06 60 81 00 00");
	uint32_t pos_32 = (uint32_t)running_speed;
	uint8_t high_first = (uint8_t)((pos_32 & 0xFF000000) >> 24);
	uint8_t high_second = (uint8_t)((pos_32 & 0x00FF0000) >> 16);
	uint8_t low_first = (uint8_t)((pos_32 & 0x0000FF00) >> 8);
	uint8_t low_second = (uint8_t)(pos_32 & 0x000000FF);

	velocity_low[4] = low_first;
	velocity_low[5] = low_second;
	velocity_high[4] = high_first;
	velocity_high[5] = high_second;
	velocity_low = add_CRC_code(velocity_low);
	velocity_high = add_CRC_code(velocity_high);
	m_serialPort->write(velocity_low);
	print_send_signal(velocity_low);
	qApp->processEvents();
	Sleep(200);
	m_serialPort->write(velocity_high);
	print_send_signal(velocity_high);
	qApp->processEvents();
	Sleep(200);
}

void motor_gui::enable_motor() {
	if (!m_serialPort->isOpen()) {
		if (!initial_modbus_connection()) {
			return;
		}
	}
	m_serialPort->write(hx_disable);
	qApp->processEvents();
	long_sleep(100);
	m_serialPort->write(hx_mod_set);
	qApp->processEvents();
	long_sleep(150);
	m_serialPort->write(hx_disable);
	qApp->processEvents();
	long_sleep(150);
	m_serialPort->write(hx_enable);
	qApp->processEvents();
	long_sleep(150);
	m_serialPort->write(hx_speed_low);
	qApp->processEvents();
	long_sleep(150);
	m_serialPort->write(hx_speed_high);
	qApp->processEvents();
	long_sleep(150);

	//m_serialPort->write(hx_acc_low);
	qApp->processEvents();
	long_sleep(150);
	//m_serialPort->write(hx_acc_high);
	qApp->processEvents();
	long_sleep(150);

}


void motor_gui::correct_move() {


	log_record("-----纠偏-----");
	m_serialPort->write(hx_disable);
	print_send_signal(hx_disable);
	qApp->processEvents();
	long_sleep(100);
	m_serialPort->write(hx_mod_set);
	print_send_signal(hx_mod_set);
	qApp->processEvents();
	long_sleep(100);
	m_serialPort->write(hx_disable);
	print_send_signal(hx_disable);
	qApp->processEvents();
	long_sleep(100);


	m_serialPort->write(hx_enable);
	print_send_signal(hx_enable);
	qApp->processEvents();
	long_sleep(100);
	
	//m_serialPort->write(hx_speed_low);
	qApp->processEvents();
	long_sleep(100);
	//m_serialPort->write(hx_speed_high);
	qApp->processEvents();
	long_sleep(100);
	//1rpm对应279620cnt/s,2796200=10rpm=60度每秒
	//5592400=20rpm=120度每秒,11184800=24度/秒
	set_velocity(87381);
	//87381
	//m_serialPort->write(hx_acc_low);
	qApp->processEvents();
	long_sleep(100);
	//m_serialPort->write(hx_acc_high);
	qApp->processEvents();
	long_sleep(100);


	int pulse_correct;
	pulse_correct = ui.spinBox->value();
	//设置纠偏位置
	log_record("纠偏脉冲数：");
	log_record(QString::number(pulse_correct));

	set_position(pulse_correct);


	m_serialPort->write(hx_enable);
	print_send_signal(hx_enable);
	qApp->processEvents();
	Sleep(100);
	Sleep(200);

	m_serialPort->write(hx_start_move);
	print_send_signal(hx_start_move);
	Sleep(100);
	qApp->processEvents();
	long_sleep(1000);
	//重置原点位置
	reset_zero();
	log_record("-----纠偏结束-----");
}

//寻找原点位置归零
void motor_gui::back_to_zero() {
	if (!m_serialPort->isOpen()) {
		if (!initial_modbus_connection()) {
			return;
		}
	}
	write_setting();
	log_record("-----归零-----");
	int curr_manual_correct;
	curr_manual_correct=ui.spinBox->value();
	writeSettingInt( L"manual_turn", curr_manual_correct);
	QByteArray mod_set_pre = QByteArray::fromHex("01 06 60 40 00 06");
	QByteArray mod_set = QByteArray::fromHex("01 06 60 60 00 06");
	
	QByteArray home_mod_set_7 = QByteArray::fromHex("01 06 60 98 00 07");
	QByteArray home_mod_set_3 = QByteArray::fromHex("01 06 60 98 00 03");
	QByteArray home_mod_set_19 = QByteArray::fromHex("01 06 60 98 00 13");
	QByteArray home_mod_set_20 = QByteArray::fromHex("01 06 60 98 00 14");
	QByteArray back_home_start = QByteArray::fromHex("01 06 60 40 00 1F");
	QByteArray back_home_end = QByteArray::fromHex("01 06 60 40 01 0F");
	QByteArray hx_get_home = QByteArray::fromHex("01 03 60 41 00 01");
	mod_set_pre=add_CRC_code(mod_set_pre);
	mod_set =add_CRC_code(mod_set);
	home_mod_set_3 = add_CRC_code(home_mod_set_3);
	home_mod_set_20= add_CRC_code(home_mod_set_20);
	back_home_start = add_CRC_code(back_home_start);
	back_home_end = add_CRC_code(back_home_end);
	QByteArray check_worning = QByteArray::fromHex("01 03 60 3f 00 01");
	check_worning = add_CRC_code(check_worning);

	
	m_serialPort->write(mod_set_pre);
	print_send_signal(mod_set_pre);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(mod_set);
	print_send_signal(mod_set);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(home_mod_set_20);
	print_send_signal(home_mod_set_20);
	qApp->processEvents();
	Sleep(100);
	//m_serialPort->write(hx_disable);
	//qApp->processEvents();
	//Sleep(100);
	/*
	m_serialPort->write(hx_speed_low);
	qApp->processEvents();
	long_sleep(100);
	m_serialPort->write(hx_speed_high);
	qApp->processEvents();
	long_sleep(100);
	m_serialPort->write(hx_acc_low);
	qApp->processEvents();
	long_sleep(100);
	m_serialPort->write(hx_acc_high);
	qApp->processEvents();
	long_sleep(100);
	*/

	m_serialPort->write(hx_enable);
	print_send_signal(hx_enable);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(back_home_start);
	print_send_signal(back_home_start);
	qApp->processEvents();
	Sleep(100);
	hx_get_home = add_CRC_code(hx_get_home);
	ready_to_read = true;
	Sleep(100);
	int overtime_counter = 0;
	bool overtime_counter_bool = false;
	while (1) {
		if (stop) {
			if (only_back_home) {
				stop = false;
			}
			break;
		}
		m_serialPort->write(hx_get_home);
		qApp->processEvents();
		Sleep(100);
		qApp->processEvents();
		Sleep(100);
		qApp->processEvents();
		Sleep(100);
		if (close_check) {
			return;
		}
		if (recvData.size()<7) {

			continue;
		}
		if (recvData.at(3) == 0x16 ) {
			log_record("-----归零到位-----");
			recvData = NULL;
			break;
		}
		overtime_counter = overtime_counter + 1;
		if (overtime_counter%10 == 0) {
			log_record(QString::number(overtime_counter));
			if (overtime_counter > 200) {
				log_record("归零超时");
				overtime_counter = 0;
				overtime_counter_bool=true;
				
				
				
				QByteArray check_worning_new = QByteArray::fromHex("01 03 60 60 00 01");
				check_worning_new = add_CRC_code(check_worning_new);
				m_serialPort->write(check_worning_new);
				print_send_signal(check_worning_new);
				Sleep(100);
				qApp->processEvents();
				Sleep(100);
				qApp->processEvents();

				m_serialPort->write(check_worning);
				print_send_signal(check_worning);
				Sleep(100);
				qApp->processEvents();
				//break;
				m_serialPort->write(back_home_end);
				print_send_signal(back_home_end);
				qApp->processEvents();
				Sleep(200);

				m_serialPort->write(check_worning);
				print_send_signal(check_worning);
				Sleep(200);
				qApp->processEvents();

				m_serialPort->write(check_worning);
				print_send_signal(check_worning);
				Sleep(200);
				qApp->processEvents();

				


				m_serialPort->write(hx_enable);
				print_send_signal(hx_enable);
				qApp->processEvents();
				Sleep(200);
				m_serialPort->write(back_home_start);
				print_send_signal(back_home_start);
				qApp->processEvents();
				Sleep(200);
				
				//ready_to_read = true;

				overtime_counter = 0;
				overtime_counter_bool = false;


			}

		}
	}
	ready_to_read = false;
	/*
	if (overtime_counter_bool) {
		//running = false;
		//close();
		return;
	}
	*/

	reset_zero();
	Sleep(100);
	
	correct_move();
	
	//m_serialPort->write(back_home_end);
	//qApp->processEvents();
	Sleep(100);
}
//QByteArray 转 QString
QString ByteArrayToHexString(QByteArray& ba){
	QDataStream out(&ba, QIODevice::ReadWrite);   //将str的数据 读到out里面去
	QString buf;
	while (!out.atEnd())
	{
	qint8 outChar = 0;
	out >> outChar;   //每次一个字节的填充到 outchar
	QString str = QString("%1").arg(outChar & 0xFF, 2, 16, QLatin1Char('0')).toUpper() + QString(" ");   //2 字符宽度
	buf += str;
	 }
	return buf;
}
//重置当前位置为0点
void motor_gui::reset_zero() {
	
	QByteArray mod_set = QByteArray::fromHex("01 06 60 60 00 06");
	QByteArray mod_set_pre = QByteArray::fromHex("01 06 60 40 00 06");
	QByteArray home_mod_set_35 = QByteArray::fromHex("01 06 60 98 00 23");
	QByteArray back_home_start = QByteArray::fromHex("01 06 60 40 00 1F");
	QByteArray stop_home = QByteArray::fromHex("01 06 60 40 00 1F");
	mod_set_pre= add_CRC_code(mod_set_pre);
	mod_set = add_CRC_code(mod_set);
	home_mod_set_35 = add_CRC_code(home_mod_set_35);
	back_home_start = add_CRC_code(back_home_start);
	Sleep(100);
	m_serialPort->write(mod_set_pre);
	print_send_signal(mod_set_pre);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(mod_set);
	print_send_signal(mod_set);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(home_mod_set_35);
	print_send_signal(home_mod_set_35);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(hx_disable);
	print_send_signal(hx_disable);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(hx_enable);
	print_send_signal(hx_enable);
	qApp->processEvents();
	Sleep(100);
	m_serialPort->write(back_home_start);
	print_send_signal(back_home_start);
	qApp->processEvents();
	Sleep(500);
	qApp->processEvents();
	Sleep(500);
	m_serialPort->write(hx_stop);
	print_send_signal(hx_stop);
	qApp->processEvents();
	Sleep(300);
	
}
//获取返回信号
void motor_gui::get_return_signal() {
	if (close_check) {
		return;
	}
	if (!ready_to_read) {
		return;
	}
	QByteArray recvData_temp = m_serialPort->readAll();
	
	if (recvData_temp.isNull()) {
		return;
	}
	if (recvData_temp.isEmpty()) {
		return;
	}
	
	if (recvData_temp.size() > 20) {
		return;
	}
	if (recvData_temp.size() >= 5 && recvData_temp.at(1) == 0x03 && recvData_temp.at(2) == 0x02) {
		QString desc = parseFaultCode(recvData_temp);
		log_record("故障描述: " + desc);
	}
	recvData = recvData_temp;
	QString recvData_str;
	recvData_str = ByteArrayToHexString(recvData);
	log_record("接收:"+recvData_str);
	qApp->processEvents();
}
void motor_gui::print_send_signal(QByteArray sendData_byte) {
	QString sendData_str = ByteArrayToHexString(sendData_byte);
	log_record("发送:"+sendData_str);
	qApp->processEvents();
}
bool start_once = true;
bool connected_aready = false;
//初始化电机串口连接
bool motor_gui::initial_modbus_connection() {

	write_setting();

	QString com_number = ui.comboBox->currentText();
	m_serialPort->setPortName(com_number);
	m_serialPort->setBaudRate(QSerialPort::Baud9600);
	m_serialPort->setDataBits(QSerialPort::Data8);
	m_serialPort->setParity(QSerialPort::NoParity);
	m_serialPort->setStopBits(QSerialPort::OneStop);
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
	m_serialPort->setReadBufferSize(40960);
	m_serialPort->open(QIODevice::ReadWrite);
	
	if (m_serialPort->isOpen()) {
		QByteArray hx_get_home = QByteArray::fromHex("01 03 60 41 00 01");
		hx_get_home = add_CRC_code(hx_get_home);
		m_serialPort->write(hx_get_home);
		ready_to_read = true;
		bool connect_success = false;

		for(int over_time_count=0; over_time_count<30; over_time_count++){
			if (stop) {
				break;
			}
			
			//log_record("over_time_count:"+ QString::number(over_time_count));
			if ((recvData.size() >=4) && (recvData.at(0) == 0x01)) {
				connect_success = true;
				
				break;
			}
			qApp->processEvents();
			long_sleep(300);
			m_serialPort->write(hx_get_home);
		}
		ready_to_read = false;
		if (connect_success) {
			ui.label_5->setText("连接成功");
			connected_aready = true;
			log_record("-----连接成功-----");
			return true;
		}
		else {
			ui.label_5->setText("连接失败");
			connected_aready = false;
			m_serialPort->close();
			log_record("-----连接失败-----");
			return false;
		}
		
		
	}
	else {
		ui.label_5->setText("连接失败");
		connected_aready=false;
		m_serialPort->close();
		log_record("-----连接失败-----");
		return false;
	}

}
//开始扫描
void motor_gui::scan()
{
	if (running) {
		return;
	}
	only_back_home = false;
	running = true;
	bool first_pos=true;
	int step_counter = 0;
	//reset_zero();
	long_sleep(100);

	back_to_zero();
	if (!connected_aready) {
		
		return;
	}

	only_back_home = true;
	log_record("-----归零结束-----");

	int zero_position_counter = 0;
	while (!stop) {

		m_serialPort->write(hx_disable);
		print_send_signal(hx_disable);
		qApp->processEvents();
		long_sleep(100);
		m_serialPort->write(hx_mod_set);
		print_send_signal(hx_mod_set);
		qApp->processEvents();
		long_sleep(100);
		m_serialPort->write(hx_disable);
		print_send_signal(hx_disable);
		qApp->processEvents();
		long_sleep(100);
		m_serialPort->write(hx_enable);
		print_send_signal(hx_enable);
		qApp->processEvents();
		long_sleep(100);

		set_velocity(87381);
		
		//m_serialPort->write(hx_speed_low);
		qApp->processEvents();
		long_sleep(100);
		//m_serialPort->write(hx_speed_high);
		qApp->processEvents();
		long_sleep(100);
		

		//m_serialPort->write(hx_acc_low);
		qApp->processEvents();
		long_sleep(100);
		//m_serialPort->write(hx_acc_high);
		qApp->processEvents();
		long_sleep(100);

		int north_ = 0;
		int east_ = 1;
		int south_ = 2;
		int west_ = 3;

		//设置vad扫描模式
		send_model_id(4);
		int degree_change = 0;
		int curr_pulse = 0;
		while (1) {

			if (degree_change == 0) {
				set_position(curr_pulse);
			}
			if (degree_change == 1) {
				set_position(curr_pulse);
			}
			if (degree_change == 2) {
				set_position(curr_pulse);
			}
			if (degree_change == 3) {
				set_position(curr_pulse);
			}
			//一圈16,777,216脉冲，2的24次方
			curr_pulse = curr_pulse - 32768;//旧脉冲数32768*4=131072
			
			if (stop) {
				
				//ui.pushButton3->setStyleSheet("background-color: rgb(255, 70, 73)");
				//ui.pushButton3->setEnabled(true);
				break;
			}
			ready_to_read = true;
			m_serialPort->write(hx_enable);
			print_send_signal(hx_enable);
			qApp->processEvents();
			long_sleep(100);


			print_send_signal(hx_start_move);
			m_serialPort->write(hx_start_move);
			
			qApp->processEvents();
			Sleep(100);
			
			step_counter = 0;
			while (1) {
				if (first_pos) {
					first_pos = false;
					break;
				}
				step_counter = step_counter + 1;
				qApp->processEvents();
				Sleep(20);
				if (recvData.size() >= 15) {
					continue;
				}
				if (recvData.at(4) != 0x00) {
					continue;
				}
				if ((recvData.at(2) == 0x60) && (recvData.at(3) == 0x40) && (recvData.at(5) == 0x1F)) {
					break;
				}
				if (step_counter >= 10) {
					break;
				}
			}
			//ui.textBrowser->append("-----到位开始扫描-----");
			ready_to_read = false;

			if (((degree_change == north_) || (degree_change == east_) || (degree_change == south_) || (degree_change == west_))) {
				//指向北 并且回零计时器加一
				if (degree_change == north_) {
					ui.pushButton_East->setStyleSheet("QPushButton{background-color: rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_West->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_South->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_North->setStyleSheet("QPushButton{background-color:rgb(25, 102, 255);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.dial->setValue(180);
					ui.label_3->setText("0°");
					zero_position_counter = zero_position_counter + 1;
					QString p_counter = QString::number(zero_position_counter);
					log_record("第" + p_counter+"圈");
					send_direction(0);
				}
				//指向东
				else if (degree_change == east_) {
					ui.pushButton_East->setStyleSheet("QPushButton{background-color: rgb(25, 102, 255);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_West->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_South->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_North->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.dial->setValue(270);
					ui.label_3->setText("90°");
					//log_record("到位90度");
					send_direction(90);
				}
				//指向南
				else if (degree_change == south_) {
					ui.pushButton_East->setStyleSheet("QPushButton{background-color: rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_West->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_South->setStyleSheet("QPushButton{background-color:rgb(25, 102, 255);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_North->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.dial->setValue(0);
					ui.label_3->setText("180°");
					//log_record("到位180度");
					send_direction(180);
				}
				//指向西
				else if (degree_change == west_) {
					ui.pushButton_East->setStyleSheet("QPushButton{background-color: rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_West->setStyleSheet("QPushButton{background-color:rgb(25, 102, 255);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_South->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.pushButton_North->setStyleSheet("QPushButton{background-color:rgb(200, 200, 200);border:0.1px solid gray;border-radius:6px;color: rgb(255, 255, 255);}");
					ui.dial->setValue(90);
					ui.label_3->setText("270°");
					//log_record("到位270度");
					send_direction(270);
				}
			}
			scan_time = ui.doubleSpinBox_wait->value();
			start_process_bar = true;
			//等待扫描进度条
			while (1) {
				if (!start_process_bar) {
					break;
				}
				if (close_check) {
					return;
				}
				Sleep(10);
				qApp->processEvents();
			}
			m_serialPort->write(hx_stop);
			print_send_signal(hx_stop);
			qApp->processEvents();
			Sleep(100);
			
			//重置原点
			
			if (zero_position_counter >= 20000 && degree_change == north_) {
				zero_position_counter = 0;

				log_record("第20000圈重置原点(约7天)");
				back_to_zero();
				reset_zero();
				long_sleep(100);
				m_serialPort->write(hx_disable);
				print_send_signal(hx_disable);
				qApp->processEvents();
				long_sleep(100);
				m_serialPort->write(hx_mod_set);
				print_send_signal(hx_mod_set);
				qApp->processEvents();
				Sleep(100);
				m_serialPort->write(hx_disable);
				print_send_signal(hx_disable);
				qApp->processEvents();
				long_sleep(100);
				m_serialPort->write(hx_enable);
				print_send_signal(hx_enable);
				qApp->processEvents();
				Sleep(100);
				break;
			}
			

			if (degree_change < 3) {
				degree_change = degree_change + 1;
			}
			else {
				degree_change = 0;
			}
			
		}
		
	}
	stop = false;
	//m_serialPort->close();

}


void motor_gui::long_wait_start() {
	long_sleep(2000);
	if (initial_modbus_connection()==false) {
		return;
	}
	
	
	for (int k = 1; k < 10; k++) {
		long_sleep(1000);
		QString k_str;
		k_str = QString::number(k);
		log_record(k_str);
		if (stop) {
			return;
		}
	}
	scan();
}

//停止扫描
void motor_gui::stop_motor()
{
	ui.label_5->setText(tr("停止扫描"));
	stop = true;
	
	if (running) {
		m_serialPort->write(hx_stop);
	}
	running = false;

}

void motor_gui::closeEvent(QCloseEvent *event) {
	close_check = true;

	if (running) {
		ui.label_5->setText(tr("关闭前停止电机"));
		qApp->processEvents();
		Sleep(200);
		event->ignore();
	}
	else {
		event->accept();
	}
	status_timer2->stop();
	//thread->exit();

}

//进度条变化信号
void thread_obj::process_bar_move() {
	//等待采集期间的界面进度
	while (1) {
		if (start_process_bar) {
			for (int i = 0; i < 101; i=i+10) {
				emit bar_signal(i);
				Sleep(int(scan_time*100));
			}
			start_process_bar = false;
		}
		Sleep(10);
		if (close_check) {
			return;
		}
	}
}
//显示进度条
void motor_gui::display_bar(int bar_process) {
	ui.progressBar->setValue(bar_process);
	perst = "%";
	process = "扫描进度：";
	persent_str = QString::number(bar_process);
	persent_str.append(perst);
	process.append(persent_str);
	ui.label_5->setText(process);
}
//初始化图标
void motor_gui::initial_ico() {
	QIcon icon_min;
	icon_min.addFile(ico_path + "/ico/最小化.png");
	ui.pushButton_min_w->setIconSize(QSize(40, 40));
	ui.pushButton_min_w->setIcon(icon_min);
	QIcon icon_close;
	icon_close.addFile(ico_path + "/ico/关闭.png");
	ui.pushButton_close_w->setIconSize(QSize(40, 40));
	ui.pushButton_close_w->setIcon(icon_close);
	QIcon icon_scan;
	icon_scan.addFile(ico_path + "/ico/扫描.png");
	ui.pushButton1->setIconSize(QSize(50, 50));
	ui.pushButton1->setIcon(icon_scan);
	QIcon icon_logo;
	icon_logo.addFile(ico_path + "/ico/软件logo.png");
	ui.pushButton_logo->setIconSize(QSize(40, 40));
	ui.pushButton_logo->setIcon(icon_logo);
	QIcon icon_stop;
	icon_stop.addFile(ico_path + "/ico/停止.png");
	ui.pushButton3->setIconSize(QSize(50, 50));
	ui.pushButton3->setIcon(icon_stop);
}
//主窗口最小化
void motor_gui::onButtonMinClicked()
{
	if (Qt::Tool == (windowFlags() & Qt::Tool))
	{
		hide();
	}
	else
	{
		showMinimized();
	}
}
bool set_mod_check = true;
//右侧调试菜单开关
void motor_gui::set_mod_show() {
	if (set_mod_check) {
		set_mod_check = false;
		setFixedSize(629, 439);
		ui.pushButton_min_w->setGeometry(549, 0, 40, 40);
		ui.pushButton_close_w->setGeometry(589, 0, 40, 40);
		ui.pushButton_setMod->setText("▶");
	}
	else {
		set_mod_check = true;
		ui.pushButton_min_w->setGeometry(878, 0, 40, 40);
		ui.pushButton_close_w->setGeometry(918, 0, 40, 40);
		setFixedSize(958, 439);
		ui.pushButton_setMod->setText("◀");
	}
	
}
bool m_bDragging;			// 是否正在拖动
QPoint m_poStartPosition;	// 拖动开始前的鼠标位置
QPoint m_poFramePosition;	// 窗体的原始位置
//窗口可拖动点击
void motor_gui::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		QRect rect = ui.layer_title->rect();  //rect是鼠标实现可拖动的区域
		rect.setBottom(rect.top() + 50);
		if (rect.contains(event->pos()))
		{
			m_bDragging = true;
			m_poStartPosition = event->globalPos();
			m_poFramePosition = frameGeometry().topLeft();
		}
	}
	QWidget::mousePressEvent(event);
}
//窗口可拖动移动
void motor_gui::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton)	//只响应鼠标所见
	{
		if (m_bDragging)
		{
			// delta 相对偏移量, 
			QPoint delta = event->globalPos() - m_poStartPosition;
			// 新位置：窗体原始位置  + 偏移量
			move(m_poFramePosition + delta);
		}
	}
	QWidget::mouseMoveEvent(event);
}
//窗口释放
void motor_gui::mouseReleaseEvent(QMouseEvent* event)
{
	m_bDragging = false;
	QWidget::mouseReleaseEvent(event);
}

void motor_gui::clear_worning_click() {

	if (!m_serialPort->isOpen()) {
		if (!initial_modbus_connection()) {
			return;
		}
	}

	log_record("查看报警");
	//查看报错信息
	QByteArray check_worning = QByteArray::fromHex("01 03 60 3f 00 01");
	check_worning = add_CRC_code(check_worning);
	m_serialPort->write(check_worning);
	ready_to_read = true;
	
	for (int k = 0; k < 100; k++) {
		qApp->processEvents();
		Sleep(30);
	}
	ready_to_read = false;
	

	//QByteArray clear_worning = QByteArray::fromHex("01 06 60 40 00 8f");
	//clear_worning = add_CRC_code(clear_worning);
	//m_serialPort->write(clear_worning);
	//qApp->processEvents();
	//Sleep(100);
	//log_record("清除报警");
}

void motor_gui::strat_process_bar_thread() {

	thread_obj* thread_show;
	QThread* thread;
	thread_show = new thread_obj();
	thread = new QThread;
	connect(thread, SIGNAL(started()), thread_show, SLOT(process_bar_move()));
	connect(thread_show, SIGNAL(bar_signal(int)), this, SLOT(display_bar(int)));
	thread_show->moveToThread(thread);
	thread->start();//开始线程

}

void motor_gui::strat_connect() {
	if (initial_modbus_connection()) {
		log_record("连接成功");
		strat_process_bar_thread();
		scan();
	}
	else {
		log_record("连接失败");
	}
}
//主程序
motor_gui::motor_gui(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	//AutoPowerOn();
	//CanclePowerOn();
	ico_path = read_ico_path();
	initial_ico();
	QIcon icon = QIcon(ico_path + "/ico/logo.ico");
	//窗口logo
	setWindowIcon(QIcon(ico_path + "/ico/logo.ico"));
	//初始化参数
	load_setting();
	
	int scan_time_int;
	scan_time_int = GetPrivateProfileInt(L"Setting", L"Scan_time",30, lpPath);
	scan_time= float(scan_time_int) / float(10);
	ui.doubleSpinBox_wait->setValue(scan_time);
	SetInt(L"WVAngle", 75);

	ui.textBrowser->document()->setMaximumBlockCount(10000);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	m_serialPort = new QSerialPort();

	/*
	long_sleep(2000);
	for (int i = 0; i < 10; i++) {
		if (initial_modbus_connection() == true) {
			enable_motor();
			break;
		}

		long_sleep(2000);
		log_record("连接错误,尝试重连");
	}
	*/

	if (start_once) {
		start_once = false;
		status_timer2 = new QTimer(this);
		connect(status_timer2, SIGNAL(timeout()), this, SLOT(get_return_signal()));
		status_timer2->start(40);
	}
	//进度条线程
	connect(ui.pushButton1, SIGNAL(clicked()), this, SLOT(scan()));
	connect(ui.pushButton2, SIGNAL(clicked()), this, SLOT(back_to_zero()));
	connect(ui.pushButton3, SIGNAL(clicked()), this, SLOT(stop_motor()));

	//connect(ui.doubleSpinBox_speed, SIGNAL(valueChanged(float)), ui.doubleSpinBox_speed, SLOT(write_speed()));
	connect(ui.pushButton_close_w, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui.pushButton_min_w, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
	connect(ui.pushButton_setMod, SIGNAL(clicked()), this, SLOT(set_mod_show()));
	connect(ui.pushButton_setMod, SIGNAL(hoverd()), this, SLOT(set_mod_show()));
	connect(ui.pushButton_clear_worning, SIGNAL(clicked()), this, SLOT(clear_worning_click()));
	

	//scan();
	
}

