#include <QDebug>
#include <QFileInfo>
#include <QtCore/qmath.h>

#include "ut.h"
 
Ut::Ut(QWidget *parent) : QDialog(parent)
{
qDebug() << "start Ut";
	ui.setupUi(this);
	connect(ui.convert, SIGNAL(clicked()), this, SLOT(read_write()));
	connect(ui.file, SIGNAL(textChanged(QString)), this, SLOT(newfile()));
	connect(ui.convert, SIGNAL(pressed()), this, SLOT(processing()));

	red.setColor(QPalette::Base,QColor(Qt::red));
	white.setColor(QPalette::Base,QColor(Qt::white));

//	filename = "..\\data\\" + ui.file->text() + "maindata_lt.txt";

	newfile();
qDebug() << "stop Ut";
}

/************************************************************/

QString Ut::zero(int k)
{
	QString s = QString::number(k);	
	if (k < 10) return "0" + s;
	else return s;   
}  

double Ut::hours(int hh, double mm, double ss)
{
       return 1.0*hh + mm/60.0 + ss/3600.0;      
} 

QString Ut::h2hhmmss(double h)
{
	h = fabs(h);
	int hh = int(h);
	int mm = int((h - hh)*60);
	int ss = int(((h - hh)*60 - mm)*60);
	QString s(zero(hh) + zero(mm) + zero(ss));
qDebug() << "h2hhmmss" << s;
	return s;  	
}    
/*---------------------------------------------------*/

void precession00(double RA, double DEC, double T, 
			double& RA2000, double& DEC2000)
{
/* //http://www.stargazing.net/kepler/b1950.html
	RA2000 = RA + 0.640265 + 0.278369 * sin(RA/180*PI) * tan(DEC/180*PI);
	DEC2000 = DEC + 0.278369 * cos(RA/180*PI);

qDebug() << RA2000/15 << DEC2000;
*/

/* //The astronomical almanac 2012, p. B54
	double M = T*(1.2811556689 + 
			T*(0.00038655131 + 
				T*(0.000010079625 - 
					T*(9.60194E-9 - 
						T*1.68806E-10))));
	double N = T*(0.5567199731 - 
			T*(0.00011930372 - 
				T*(0.000011617400 - 
					T*(1.96917E-9 - 
						T*3.5389E-11))));

	double alpham = RA - 0.5*(M + N*sin(RA/180*PI)*tan(DEC/180*PI));
	double deltam = DEC - 0.5*N*cos(alpham/180*PI); 

qDebug() << "M=" << M << " N=" << N << " am=" << alpham << " dm=" << deltam;

	RA2000 = RA - M - N*sin(alpham/180*PI)*tan(deltam/180*PI);
	DEC2000 = DEC - N*cos(alpham/180*PI);

qDebug() << RA2000/15 << DEC2000;
*/
}

void precession01(double RA, double DEC, double T, 
			double& RA2000, double& DEC2000)
{
// Jean Meeus, Astronomishe algorithmen, 1991, p.136
// Peter Duffertt-Smith, Prac. astro. with your calc., 1988

//	double m = (3.07496 + 0.00186*T)*15; // sec
//	double n = (20.0431 - 0.0085*T);     // sec
	const double PI = 3.14159265; 
	double m = 3.07420*15; 
	double n = 20.0383; 

qDebug() << "m=" << m << "n=" << n;

qDebug() << RA << n*qCos(RA/180*PI);

	if (qFabs(RA - 90.0) > 1e-5)
		RA2000 = (RA*60*60 - (m + n*qSin(RA/180*PI)*qTan(DEC/180*PI))*T)/60/60; 
	else    
		RA2000 = RA;
	DEC2000 = (DEC*60*60 - (n*qCos(RA/180*PI))*T)/60/60;
}

void precession02(double ra0, double de0, double eq0, 
			double& ra1, double& de1)
{
// Rumen Bogdanovski, rumen.pl
//
	const double DEG2RAD = 3.1415926535897932384626433832795/180.0;
	const double RAD2DEG = 180.0/3.1415926535897932384626433832795;

	double ra = ra0*DEG2RAD;
	double dec = de0*DEG2RAD;

	double cosd = qCos(dec);

	double x0 = cosd*qCos(ra);
	double y0 = cosd*qSin(ra);
	double z0 = qSin(dec);

	eq0 = 2000 + eq0;
	const double eq1 = 2000.0;  // ???

	double ST = (eq0 - 2000.0)*0.001;
	double T = (eq1 - eq0)*0.001;

	double sec2rad = DEG2RAD/3600.0;
	double A = sec2rad*T*(23062.181 + ST*(139.656 + 0.0139*ST) + T*
				(30.188 - 0.344*ST + 17.998*T));
	double B = sec2rad*T*T*(79.280 + 0.410*ST + 0.205*T) + A;
	double C = sec2rad*T*(20043.109 - ST*(85.33 + 0.217*ST) + T*
				(-42.665 - 0.217*ST - 41.833*T));

	double sinA = qSin(A), sinB = qSin(B), sinC = qSin(C);
	double cosA = qCos(A), cosB = qCos(B), cosC = qCos(C);

	double rot[3][3];
	
	rot[0][0] = cosA*cosB*cosC - sinA*sinB;
	rot[0][1] = (-1)*sinA*cosB*cosC - cosA*sinB;
	rot[0][2] = (-1)*sinC*cosB;

	rot[1][0] = cosA*cosC*sinB + sinA*cosB;
	rot[1][1] = (-1)*sinA*cosC*sinB + cosA*cosB;
	rot[1][2] = (-1)*sinB*sinC;

	rot[2][0] = cosA*sinC;
	rot[2][1] = (-1)*sinA*sinC;
	rot[2][2] = cosC;

	double x1 = rot[0][0]*x0 + rot[0][1]*y0 + rot[0][2]*z0;
	double y1 = rot[1][0]*x0 + rot[1][1]*y0 + rot[1][2]*z0;
	double z1 = rot[2][0]*x0 + rot[2][1]*y0 + rot[2][2]*z0;

	if (x1 == 0) 
	{
		if (y1 > 0) ra1 = 90.0;
		else ra1 = 270.0;
	}
	else ra1 = qAtan2(y1, x1)*RAD2DEG;
	      
	if(ra1 < 0) ra1 += 360;

	de1 = qAtan2(z1, sqrt(1 - z1*z1))* RAD2DEG;
}

QString Ut::conv(QString s)
{
	const int TIME = 28;  

      	int year = s.mid(TIME+0,4).toInt();
	int month = s.mid(TIME+4, 2).toInt();
	int day = s.mid(TIME+6, 2).toInt();
	QDate qdate(year, month, day);

	int hour = s.mid(TIME+8, 2).toInt();
	int minute = s.mid(TIME+10, 2).toInt(); 
	int second = s.mid(TIME+12, 2).toInt(); 
	QTime qtime(hour, minute, second);

	QDateTime qdt(qdate, qtime);
qDebug() << qdate << qtime << qdt;

	QDateTime from;
	
	if (ui.obst->isChecked()) from = qdt;
	else if (ui.b1975->isChecked()) from = QDateTime(QDate(1975, 1, 1), QTime(0,0,0));
	else if (ui.b1950->isChecked()) from = QDateTime(QDate(1950, 1, 1), QTime(0,0,0));
	else if (ui.b1900->isChecked()) from = QDateTime(QDate(1900, 1, 1), QTime(0,0,0));
	else if (ui.b1875->isChecked()) from = QDateTime(QDate(1875, 1, 1), QTime(0,0,0));
	else if (ui.b1855->isChecked()) from = QDateTime(QDate(1855, 1, 1), QTime(0,0,0));

	int Tsec = from.secsTo(QDateTime(QDate(2000, 1, 1), QTime(0,0,0)));

	double Tyear = 365.242190*24*60*60;
	double T = -Tsec/Tyear;
qDebug() << Tsec << Tyear << T;

	const int COORD = 14;

	int RAh = s.mid(COORD+0, 2).toInt();
	int RAm = s.mid(COORD+2, 2).toInt();
	int RAs = s.mid(COORD+4, 2).toInt();

	int DECg = s.mid(COORD+6, 3).toInt();
	int DECm = s.mid(COORD+9, 2).toInt();
	int DECs = s.mid(COORD+11, 2).toInt();
	
	double RA = hours(RAh*15, RAm*15, RAs*15);
	double DEC = hours(DECg, DECm, DECs);
qDebug() << RA << DEC;

	double RA2000, DEC2000;
	
//   formulae
	precession02(RA, DEC, T, RA2000, DEC2000);

	QString sout = s.left(COORD) + h2hhmmss(RA2000/15) + ((DEC2000>=0)?"+":"-") +
		h2hhmmss(DEC2000) + s.mid(COORD+13);            
qDebug() << s;
qDebug() << sout;
	return sout;
}

void Ut::read_write()
{
qDebug() << "begin read_write";
	QFile inp(filename);
	if (!inp.open(QFile::ReadOnly))
	{
qDebug() << "ERROR1";
		return;
	}
	QTextStream text1(&inp);
	
	QString filename2 = filename.left(filename.size() - 6) + "ok.txt";
qDebug() << filename << "-->" << filename2;
	QFile out(filename2);
	if (!out.open(QFile::WriteOnly))
	{
qDebug() << "ERROR2";
		return;
	}
	QTextStream text2(&out);

	while (!text1.atEnd())
	{
		QString line(text1.readLine());
		if (line.mid(27,1) != " ") text2 << line << endl;
		else text2 << conv(line) << endl;
	}
	inp.close();
	out.close();

	ui.convert->setText("DONE!");
	ui.convert->setDisabled(true);
qDebug() << "end read_write";	
}
/********************** slots *****************************************/
void Ut::newfile()
{
	QString s = "..\\data\\" + ui.file->text() + ui.label4->text();
qDebug() << "newfile:" << s;
	QFileInfo f(s);
	if (f.isFile()) 
	{
		ui.file->setPalette(white);
		ui.convert->setDisabled(false);
		filename = s;
	}
	else 
	{
		ui.file->setPalette(red);
		ui.convert->setDisabled(true);
	}
}

void Ut::processing()
{
	ui.convert->setText("Processing...");
	ui.convert->update();
}

