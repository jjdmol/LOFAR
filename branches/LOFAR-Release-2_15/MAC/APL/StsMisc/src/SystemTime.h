#ifndef SYSTEMTIME_H
#define SYSTEMTIME_H

#include <time.h>
#include "import/LofarTypes.h"
#include "import/lofar_string.h"
#include <ctime>



//namespace LOFAR 
//{
class SystemTime
{
	
	public:
		SystemTime();
		~SystemTime();
		
		//
		// set the time for system for all output files
		//
		char* getTimeDate();

		//
		//Get the time and date of the system
		//
		void updateDateTime();

		//
		// select year 
		//
		int16 getYear();
		
		//
		// select month 
		//
		int16 getMonth();
		
		//
		// select day
		//
		int16 getDay();
		
		//
		// select time
		//
		int32 getTime(int16 timePart);



	private:
		int16 itsYear;
		int16 itsMonth;
		int16 itsDay;
		int16 itsHours;
		int16 itsMin;
		int16 itsSec;

		


};
//} // close namespace lofar

#endif