//#  LofarLoggerTest.cc: test program for the Lofar logger
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "../../src/LofarLogger.h"

#define EXAMPLE_LOGGER   (MAC_LOGGER_ROOT+string(".ExampleLogger"))
#define EXAMPLE_LOGGER2  (MAC_LOGGER_ROOT+string(".ExampleLogger2"))
#define PVSS_TEST_LOGGER (MAC_LOGGER_ROOT+string(".PVSS_Test"))

int main(int argc,char*[] argv)
{
    int i=1;
    LOFAR_LOG_TRACE(EXAMPLE_LOGGER,("example %s level message %d","trace",i++))
    LOFAR_LOG_TRACE(EXAMPLE_LOGGER2,("example2 %s level message %d","trace",i++))
    LOFAR_LOG_DEBUG(EXAMPLE_LOGGER,("example %s level message %d","debug",i++))
    LOFAR_LOG_DEBUG(EXAMPLE_LOGGER2,("example2 %s level message %d","debug",i++))
    LOFAR_LOG_INFO(EXAMPLE_LOGGER,("example %s level message %d","info",i++))
    LOFAR_LOG_INFO(EXAMPLE_LOGGER2,("example2 %s level message %d","info",i++))
    LOFAR_LOG_WARN(EXAMPLE_LOGGER,("example %s level message %d","warn",i++))
    LOFAR_LOG_WARN(EXAMPLE_LOGGER2,("example2 %s level message %d","warn",i++))
    LOFAR_LOG_ERROR(EXAMPLE_LOGGER,("example %s level message %d","error",i++))
    LOFAR_LOG_ERROR(EXAMPLE_LOGGER2,("example2 %s level message %d","error",i++))
    LOFAR_LOG_FATAL(EXAMPLE_LOGGER,("example %s level message %d","fatal",i++))
    LOFAR_LOG_FATAL(EXAMPLE_LOGGER2,("example2 %s level message %d","fatal",i++))

    while(1)
    {
        int sleepInterval(100); //ms
        struct timespec sleepIntervalStruct;
        if(argc > 1)
        {
            sleepInterval=atoi(argv[1]);
            if(sleepInterval==0)
            {
                sleepInterval=100;
            }
        }
        sleepIntervalStruct.tv_sec=sleepInterval/1000;
        sleepIntervalStruct.tv_nsec=(sleepInterval-(sleepIntervalStruct.tv_sec*1000))*1000000;

        struct timeval timeOfDay1;
        struct timeval timeOfDay2;
        struct timeval timeDiff;

        gettimeofday(&timeOfDay1,0);
        nanosleep(&sleepIntervalStruct,0);
        gettimeofday(&timeOfDay2,0);
        timersub(&timeOfDay2,&timeOfDay1,&timeDiff);

        LOFAR_LOG_INFO(PVSS_TEST_LOGGER,("expected,%ld.%08ld,actual,%ld.%06ld",
            sleepIntervalStruct.tv_sec,sleepIntervalStruct.tv_nsec,timeDiff.tv_sec,timeDiff.tv_usec))
    }
    return 0;
}


