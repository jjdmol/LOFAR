/*
 * main.cpp
 *
 * Author         : Wouter Klijn
 * e-mail         : klijn@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 28-oct-2014
 * URL            : $URL: https://svn.astron.nl/ROD/branches/LOFAR_Scheduler-Task6767/main.cpp $
 *
 */

#include "signalhandler.h"
#include "Controller.h"

#include <iostream>

//************** receiving signal back: **********************************
// In this case a queued connection is used, therefore you’re required to run
// an event loop in the thread the Thread object is living in.
// http://qt-project.org/wiki/ThreadsEventsQObjects

SignalHandler::SignalHandler(QApplication *app, Controller *c)
{
    itsApplication = app;
    itsController = c;
    // Create the connection between the SignalHandler class and the possible
    // items to action upon.

    connectSignals();
    connect(itsController->gui->getSchedulerGUIClass().action_DownloadSASSchedule,
            SIGNAL(triggered()), itsController, SLOT(downloadSASSchedule()));

}

void SignalHandler::connectSignals(void)
{
    connect(this,                          // This QObject signal
            SIGNAL(mainWindowClose()),     // Function in this object
            itsController,                 // The target object
            SLOT(quit()));                 // The slot to 'call'

    connect(this,          SIGNAL(downloadSASSchedule()),
            itsController, SLOT(downloadSASSchedule()));

    connect(this,          SIGNAL(closeSASScheduleDownloadDialog()),
            &(itsController->itsSASConnection->progressDialog()), SLOT(close()));

    connect(this,          SIGNAL(doNotSaveSchedule()),
            itsController, SLOT(setDoNotSaveSchedule()));

    connect(this,          SIGNAL(checkSASStatus()),
            itsController, SLOT(checkSASStatus()));

    connect(this,          SIGNAL(closeCheckSASStatusDialog()),
            &(itsController->itsSASConnection->getSASStatusDialog()), SLOT(close()));

}

int SignalHandler::signalForward(std::string action, std::string /*parameter*/)
{
    // IF then tree to select specific signals to send.
    // TODO: This feels clutchy might be a better solution. Might have to wait
    // untill next refactor step.

    std::cerr << "Received command: " << action << std::endl;

    if (action == "DownloadSASSchedule")
        emit downloadSASSchedule();
    else if (action == "DownloadSASScheduleClose")
        emit closeSASScheduleDownloadDialog();
    else if (action == "MainWindowClose")
    {   // The save dialog does not play nice with the with the signal system
        // Set a noSave flag and then close
        emit doNotSaveSchedule();
        emit mainWindowClose();
    }
    else if (action == "checkSASStatus")
        emit checkSASStatus();
    else if (action == "closeCheckSASStatusDialog")
        emit closeCheckSASStatusDialog();


    else{ // If an unknown action string is received return 1
        // TODO: Add to logfile line that unknown signal was received.
        throw 1;
    }
    // Incorrect code should result in exceptions. to be caught by test framework

    return 0;
}


