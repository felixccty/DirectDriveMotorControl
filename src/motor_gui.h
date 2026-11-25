#pragma once

#include <QtWidgets/QWidget>
#include "ui_motor_gui.h"

//#include"ui_closeBox.h"
#include "stdafx.h"
#include <QtGui/QCloseEvent>
#include <QSystemTrayIcon>
class motor_gui : public QWidget
{
	Q_OBJECT

public:
	motor_gui(QWidget *parent = Q_NULLPTR);
	QString perst="";
	QString process = "";
	QString persent_str = "";
	QThread* thread1;
	QString ico_path = "";
	QTimer* status_timer2;
	bool running = false;
	bool only_back_home = true;
	void sleep_timer(int ms);


public slots:
	void long_sleep(int);
	void log_record(QString);
	void scan();
	void stop_motor();
	void display_bar(int bar_process);
	void closeEvent(QCloseEvent *event);
	void load_setting();
	void write_setting();
	bool initial_modbus_connection();
	void set_position(int);
	void set_velocity(int);
	void back_to_zero();
	void reset_zero();
	void get_return_signal();
	void initial_ico();
	void onButtonMinClicked();
	void set_mod_show();
	void correct_move();
	void enable_motor();
	void long_wait_start();
	void strat_process_bar_thread();
	void strat_connect();
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	void clear_worning_click();
	void print_send_signal(QByteArray);
	QString parseFaultCode(const QByteArray& recvData);

private:
	Ui::motor_guiClass ui;
	//Ui::Dialog box;
};
//进度条线程
class thread_obj: public QObject
{
	Q_OBJECT
private slots:
	void process_bar_move();
	
	
signals:
	void bar_signal(int);
	

};
