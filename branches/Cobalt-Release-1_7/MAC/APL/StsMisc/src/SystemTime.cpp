#include "SystemTime.h"

//namespace LOFAR 
//{

SystemTime::SystemTime()
{ 
}

SystemTime::~SystemTime()
{

}

//
// Get systemdate and time  
//
void SystemTime::updateDateTime()
{
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );


	// update the date values from the system 
	itsYear = timeinfo->tm_year;
	itsMonth = timeinfo->tm_mon;
	itsDay = timeinfo->tm_mday;

	//update the time value from the system
	itsHours = timeinfo->tm_hour;
	itsMin = timeinfo->tm_min;
	itsSec = timeinfo->tm_sec;
}

int16 SystemTime::getYear()
{	
	return itsYear;
}

int16 SystemTime::getMonth()
{
	return itsMonth;
}

int16 SystemTime::getDay()
{
	return itsDay;
}

//
// Get time of the system, with the follow choises 
//
// 0 = get times as follow 154059. This stands for 15 hours, 40 minutes and 59 seconds
// 1 = get hours and minutes as follow 1349. This stands for 13 hours and 49 minutes
// 2 = get minutes and seconds as follow 1503. This stands for 15 minutes and 3 seconds 
// 3 = get only the hours
// 4 = get only the minutes
// 5 = get only the seconds
//
int32 SystemTime::getTime(int16 timePart)
{
	int32 time;

	switch(timePart)
	{
		case 0: time = (10000 * itsHours) + (100 * itsMin) + itsSec; 
			break;
		case 1: time = (100 * itsHours) + itsMin;  
			break;
		case 2: time = (100 * itsMin) + itsSec;		
			break;
		case 3: time = itsHours;						
			break;
		case 4: time = itsMin;							
			break;
		case 5: time = itsSec;							
	}
	
	return time;
}

//
// get time and date from system for the logfiles
//
char* SystemTime::getTimeDate()
{
	time_t rawtime;
	time ( &rawtime );
	
	return (ctime (&rawtime));  //(ctime (&rawtime));;
}


//} // close lofar namespace