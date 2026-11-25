#include "motor_gui.h"
#include <QtWidgets/QApplication>
#include <iostream> 
#include <windows.h>
using namespace std;


bool checkOneRunning()
{
	//  创建互斥量
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, L"motor_gui");
	//  检查错误代码
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		//  如果已有互斥量存在则释放句柄并复位互斥量
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		//  程序退出
		return  false;
	}
	else
		return true;
}

int main(int argc, char *argv[])
{

	QApplication a(argc, argv);

	if (!checkOneRunning()) {
		return 0;
	}
	motor_gui w;
	QString s1 = QStringLiteral("测风电机控制");
	w.setWindowTitle(s1);

	w.setWindowFlags(w.windowFlags()&~Qt::WindowMaximizeButtonHint);
	w.show();
	
	
	//w.long_wait_start();
	//Sleep(15000);
	//w.time_counter();
	w.strat_connect();
	
	return a.exec();



}

