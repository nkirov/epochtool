#include <QApplication>
#include "ut.h"

int main(int argc, char *argv[])
{
     	QApplication app(argc, argv);
	Ut ut;	
	ut.show();
	return app.exec();
}

