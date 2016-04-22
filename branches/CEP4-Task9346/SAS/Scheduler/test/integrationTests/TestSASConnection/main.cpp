
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

#include "Scheduler/schedulerLib.h"

#include <time.h>
#include <string>
#include <iostream>
#include <QtCore>

// Example scheduler integration test.
// We start the scheduler in the main thread but also start a thread containing
// A number of actions to be performed by the scheduler.
// TODO: The majority of this source is boilerplate.
// refactor to remove duplicate code between tests.

// The exercising of the scheduler should happen in a thread. The gui runs in
// the main thread. TODO: This is the cause of errors: The event loop
// might get corrupted.
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

//        // Step 1: Press download button
//        signalForward("DownloadSASSchedule","");
//        sleep(5);

//        // Step 2: Press close button
//        signalForward("DownloadSASScheduleClose","");
//        sleep(3);

//        // Assert that the sas connection worked...?
//        // We need faulty state then

        // step 4: Press close application button
        signalForward("MainWindowClose","");
        sleep(2);
        // step 5: press no button, do not save
        signalForward("PresNoInSaveDialog", ",");

        // Assertain correct correct closure of program.
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
    test1.wait();  // It takes about 3 seconds for the thread to be distructed

    return exit_value;
}


