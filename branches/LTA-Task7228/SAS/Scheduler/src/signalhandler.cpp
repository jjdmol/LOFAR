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



SignalHandler::SignalHandler(QApplication *app, Controller *c)
{
    itsApplication = app;
    itsController = c;
    // Create the connection between the SignalHandler class and the possible
    // items to action upon.
    //connect(itsController->gui->getSchedulerGUIClass().action_DownloadSASSchedule,
    //       SIGNAL(triggered()), itsController, SLOT(downloadSASSchedule()));
}

int SignalHandler::signalForward(std::string action, std::string /*parameter*/)
{
    // IF then tree to select specific signals to send.
    // TODO: This feels clutchy might be a better solution. Might have to wait
    // untill next refactor step.

    std::cerr << "Received command: " << action << std::endl;

    if (action == "DownloadSASSchedule")
        emit itsController->gui->getSchedulerGUIClass(
                ).action_DownloadSASSchedule->trigger();
    else if (action == "DownloadSASScheduleClose")
    {

        QApplication::sendEvent(
            &itsController->itsSASConnection->progressDialog(),
                    new QCloseEvent());
        // TODO: Might cause an xevent que mixup
        // Xlib: sequence lost (0x1037e > 0x381) in reply type 0x9!
        //(google gives comparable/same erros
        // and the possible cause
    }
    else if (action == "MainWindowClose")
    {
        QApplication::sendEvent(
                    itsApplication,
                    new QCloseEvent());

    }
    // This action does not work. Saving this as a starting point for a next
    // atempt.
    else if (action == "PresNoInSaveDialog")
    {
        QMessageBox* box = itsController->possiblySaveMessageBox;
        std::cout << "debug 1" << std::endl;
        if (!box)  // If null pointer we cannot press a button in the dialog
            return 0;
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


