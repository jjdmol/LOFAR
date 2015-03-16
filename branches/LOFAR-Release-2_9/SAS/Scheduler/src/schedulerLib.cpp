/*
 * main.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jan 29, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/branches/LOFAR_Scheduler-Task6767/main.cpp $
 *
 */

#include <QtGui>
#include <QApplication>
#include "lofar_scheduler.h"
#include "Controller.h"
#include "schedulergui.h"
#include "signalhandler.h"

#include <exception>
#include <iostream>

#undef main

QString currentUser;

SignalHandler *handler;


int signalForward(std::string functionName, std::string parameter)
{
    // The signalForward is depending on handler* that is instantiated created in a different thread
    // Assure that it is assigned/created
    if (!handler)
        throw new exception();

    // Forward the signal and set return value
    return handler->signalForward(functionName, parameter);
}

int main_function(int argc, char *argv[])
{
    // remember the user running the scheduler, to know if
    //certain functionality like publish and cleaning data should be enabled
    // TODO: What is the function of this code? Description is sparce
    QStringList env(QProcess::systemEnvironment());
    env = env.filter("USER");    
    if (!env.isEmpty()) {
        currentUser = env.front().split("=").back();
    }
    // Create app and controller
    QApplication app(argc, argv);
    Controller c(app); // The controller is the intermediary between the SchedulerGUI (the view) and the Scheduler

    // Assign the handler with the adresses of the app and the controller
    // TODO: MVC seperation, where is the model? The M should be instantiated as a
    // specific object
    handler = new SignalHandler(&app, &c);

    // c.start() does not return it does this after closing gui window.
    try {
    c.start(); // controller starts the GUI
    }
    catch (...)
    {   // On all exception cough gracefully quit()
        // TODO: Might be problematic when memory exception. This feels like a desctructor
        c.quit();
    }
    return app.exec();
}
