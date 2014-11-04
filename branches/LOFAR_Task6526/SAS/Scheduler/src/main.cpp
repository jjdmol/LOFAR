/*
 * main.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jan 29, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/main.cpp $
 *
 */

#include <QtGui>
#include <QApplication>
#include "lofar_scheduler.h"
#include "Controller.h"
#include "schedulergui.h"

#undef main

QString currentUser;

int main(int argc, char *argv[])
{
	// remember the user running the scheduler, to know if certain functionality like publish and cleaning data should be enabled
	QStringList env(QProcess::systemEnvironment());
	env = env.filter("USER");
	if (!env.isEmpty()) {
		currentUser = env.front().split("=").back();
	}

	QApplication app(argc, argv);
    Controller c(app); // The controller is the intermediary between the SchedulerGUI (the view) and the Scheduler
    try {
    c.start(); // controller starts the GUI
    }
    catch (...) {
    	c.quit();
    }
    return app.exec();
}

