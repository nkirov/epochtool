#ifndef MENU_H
#define MENU_H

#include <QString>
#include <QDateTime>
#include "ui_ut.h"

class Ut : public QDialog
{
	Q_OBJECT
	public:
		Ut(QWidget *parent = 0);
	private:
		Ui::ut ui;
		QPalette red, white;
		QString filename;	

		QString zero(int);
		double hours(int, double, double);
		QString h2hhmmss(double);
		QString conv(QString);
	
	public slots:
		void read_write();
		void newfile();
		void processing();
};
#endif
