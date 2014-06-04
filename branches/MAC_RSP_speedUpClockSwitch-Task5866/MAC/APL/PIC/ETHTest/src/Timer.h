#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <math.h>

class Timer {
    struct timeval tstart,tstop;
 public:
    void start() { gettimeofday(&tstart, NULL); };
    void stop()  { gettimeofday(&tstop, NULL); };
    double time_elapsed() 
	{
 
	    return tstop.tv_sec-tstart.tv_sec 
		+ (tstop.tv_usec-tstart.tv_usec)/(double)1e6; 
	}
    unsigned long startTime() 
	{ 
	    struct tm* t = localtime(&(tstart.tv_sec));
	    unsigned long stamp =((((((1900+t->tm_year)*1000000+
				      t->tm_mon+1)*100
				     +t->tm_mday)*100)+t->tm_hour)*100
				  +t->tm_min)*100+t->tm_sec;
	    return stamp;
	}
    double stopTime() 
	{ 
	    return tstop.tv_sec
		+ tstop.tv_usec/(double)1e6; 
	}

    friend ostream& operator<<(ostream& strm, Timer& T);
};

inline ostream& operator<<(ostream& strm, Timer& T)
{
    
//    ios_base::fmtflags flags = strm.flags();
    char fc=strm.fill('0');
    struct tm* t = localtime(&(T.tstart.tv_sec));
    strm << 1900+t->tm_year
	 << setw(2) << t->tm_mon+1
	 << setw(2) << t->tm_mday
	 << setw(2) << t->tm_hour
	 << setw(2) << t->tm_min
	 << setw(2) << t->tm_sec;
//    strm.flags(flags);
    strm.fill(fc);
    return strm;
}


inline time_t Seconds(const double& t)
{
    return static_cast<time_t>(floor(t));
}

inline long NanoSeconds(const double& t)
{
    return static_cast<long>(floor((t - floor(t))*1e9));
}

#endif
