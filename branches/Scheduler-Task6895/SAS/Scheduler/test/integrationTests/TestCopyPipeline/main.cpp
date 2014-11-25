
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

#include <lofar_config.h>

#include "Scheduler/schedulerLib.h"

#include <time.h>
#include <string>
#include <iostream>
#include <exception>
#include <QtCore>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

LOFAR::Exception::TerminateHandler th(LOFAR::Exception::terminate);

class TestThread : public QThread
{
    // Very shallow wrapper around the run function
private:
    // Todo, rename to test or forward to function test?
    void run()
    {
        // TODO: We use fragile wait here, if we have signal back we could
        // make the next step conditional
        sleep(3);


        // Step 1: Press download button
        signalForward("DownloadSASSchedule","");
        sleep(5);


        // Step 2: Press close button
        signalForward("DownloadSASScheduleClose","");
        sleep(3);

        // Step 3: checkSASStatus
        signalForward("checkSASStatus","");
        sleep(5);

        // validate sas status
        bool result = getStatusSASDialogFeedbackResult();
        if (!result)
            THROW(LOFAR::Exception, "Test of SAS status return an problem") ;

        // Close the status window
        signalForward("closeCheckSASStatusDialog", "");
        sleep(1);

        // step 4: Press close application button
        signalForward("MainWindowClose","");
        //sleep(2);
    }
};

int main(int argc, char *argv[])
{
    // Test thread
    TestThread test1;
    test1.start();

    // The actual scheduler main function
    // TODO: Should we validate the exit state?
    int exit_value  = main_function(argc, argv);

    test1.quit();
    test1.wait();  // It takes about 3 seconds for the thread to be destructed

    return exit_value;
}


