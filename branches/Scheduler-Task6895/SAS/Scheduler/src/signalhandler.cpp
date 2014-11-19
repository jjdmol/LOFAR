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
            &itsController->itsSASConnection->progressDialog(), SLOT(close()));

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
        emit mainWindowClose();
    else if (action == "PresNoInSaveDialog")
    {
        QMessageBox* box = itsController->getPossiblySaveMessageBox();
        std::cout << "debug 1" << std::endl;
        if (box)
        {
            // wait with connecting untill we know the box exists
            connect(this,          SIGNAL(pressNoInSaveScheduleMsgBox()),
                    box, SLOT(reject()));
            emit pressNoInSaveScheduleMsgBox();
            std::cout << "debug !box" << std::endl;
            return 0;
        }
        std::cout << "debug 2" << std::endl;

        //TODO: Emit a "do not save button clicked event"

        //QAbstractButton* noButton = box->button(QMessageBox::No);
        //QPushButton* noButton = dynamic_cast<QPushButton*>(box->button(QMessageBox::No));

        //std::cout << "result: " << box->result() << std::endl;
        //box->setResult(QMessageBox::No);
        //std::cout << "result: " << box->result() << std::endl;

        //QPushButton* pushButton = box->findChild<QPushButton*>();
        //std::cout << "debug 3" << std::endl;
        //std::cout << "Found buttons: " << pushButton << std::cout;


    }

    else{ // If an unknown action string is received return 1
        // TODO: Add to logfile line that unknown signal was received.
        throw 1;
    }
    // Incorrect code should result in exceptions. to be caught by test framework

    return 0;
}


