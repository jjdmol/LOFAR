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
#include <lofar_config.h>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>


#include "schedulerLib.h"

LOFAR::Exception::TerminateHandler th(LOFAR::Exception::terminate);

int main(int argc, char *argv[])
{
    return main_function(argc, argv);
}
