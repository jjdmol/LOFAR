//# Timer.h: Accurate timer
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_COMMON_TIMER_H
#define LOFAR_COMMON_TIMER_H

// \file Timer.h
// Accurate timer

#include <cstdlib>
#include <cstring>
#include <iostream>

#if defined __ia64__ && defined __INTEL_COMPILER
#include <ia64regs.h>
#endif


namespace LOFAR {

  // Low-overhead and high-resolution interval timer for use on i386 and x86_64
  // platforms, using the processor's timestamp counter that is incremented each
  // cycle.  Put timer.start() and timer.stop() calls around the piece of
  // code to be timed; make sure that start() and stop() calls alternate.
  // A timer can be started and stopped multiple times; both the average and
  // total time, as well as the number of iterations are printed.
  // The measured time is real time (as opposed to user or system time).
  // The timer can be used to measure 10 nanosecond to a century time intervals.

  class NSTimer {
    public:
			   NSTimer(const char *name = 0, bool print_on_destruction = false);
			   ~NSTimer();

	void		   start();
	void		   stop();
	void		   reset();
	std::ostream   	   &print(std::ostream &);

    private:
	void		   print_time(std::ostream &, const char *which, double time) const;

	union {
	  long long	   total_time;
	  struct {
#if defined __PPC__
	      int	   high, low;
#else
	      int	   low, high;
#endif
	  };
	};

	unsigned long long count;
	char		   *const name;
	const bool	   print_on_destruction;

	static double	   CPU_speed_in_MHz, get_CPU_speed_in_MHz();
  };


  std::ostream &operator << (std::ostream &, class NSTimer &);


  inline void NSTimer::reset()
  {
    total_time = 0;
    count      = 0;
  }


  inline NSTimer::NSTimer(const char *name, bool print_on_destruction)
    :
    name(name != 0 ? strdup(name) : 0),
    print_on_destruction(print_on_destruction)
  {
    reset();
  }


  inline NSTimer::~NSTimer()
  {
    if (print_on_destruction)
      print(std::cerr);

    if (name != 0)
      free(name);
  }

  inline void NSTimer::start()
  {
#if (defined __i386__ || defined __x86_64__) && defined __PATHSCALE__
    unsigned eax, edx;

    asm volatile ("rdtsc" : "=a" (eax), "=d" (edx));

    total_time -= ((unsigned long long) edx << 32) + eax;
#elif (defined __i386__ || defined __x86_64__) && (defined __GNUC__ || defined __INTEL_COMPILER)
    asm volatile
    (
	"rdtsc\n\t"
	"subl %%eax, %0\n\t"
	"sbbl %%edx, %1"
    :
	"+m" (low), "+m" (high)
    :
    :
	"eax", "edx"
    );
#elif defined __ia64__ && defined __INTEL_COMPILER
    total_time -= __getReg(_IA64_REG_AR_ITC);
#elif defined __ia64__ && defined __GNUC__
    long long time;
    asm volatile ("mov %0=ar.itc" : "=r" (time));
    total_time -= time;
#elif defined __PPC__ && (defined __GNUC__ || defined __xlC__)
    int high_reg, low_reg, retry_reg;

    asm
    (
	"0:\n\t"
	"mftbu %0\n\t"
	"mftb %1\n\t"
	"mftbu %2\n\t"
	"cmpw %2,%0\n\t"
	"bne 0b\n\t"
	"subfc %3,%1,%3\n\t"
	"subfe %4,%0,%4"
    :
	"=r" (high_reg), "=r" (low_reg), "=r" (retry_reg), "=r" (low), "=r" (high)
    :
	"3" (low), "4" (high)
    );
#endif
  }


  inline void NSTimer::stop()
  {
#if (defined __i386__ || defined __x86_64__) && defined __PATHSCALE__
    unsigned eax, edx;

    asm volatile ("rdtsc\n\t" : "=a" (eax), "=d" (edx));
    total_time += ((unsigned long long) edx << 32) + eax;
#elif (defined __i386__ || defined __x86_64__) && (defined __GNUC__ || defined __INTEL_COMPILER)
    asm volatile
    (
	"rdtsc\n\t"
	"addl %%eax, %0\n\t"
	"adcl %%edx, %1"
    :
	"+m" (low), "+m" (high)
    :
    :
	"eax", "edx"
    );
#elif defined __ia64__ && defined __INTEL_COMPILER
    total_time += __getReg(_IA64_REG_AR_ITC);
#elif defined __ia64__ && defined __GNUC__
    long long time;
    asm volatile ("mov %0=ar.itc" : "=r" (time));
    total_time += time;
#elif defined __PPC__ && (defined __GNUC__ || defined __xlC__)
    int high_reg, low_reg, retry_reg;

    asm
    (
	"0:\n\t"
	"mftbu %0\n\t"
	"mftb %1\n\t"
	"mftbu %2\n\t"
	"cmpw %2,%0\n\t"
	"bne 0b\n\t"
	"addc %3,%3,%1\n\t"
	"adde %4,%4,%0"
    :
	"=r" (high_reg), "=r" (low_reg), "=r" (retry_reg), "=r" (low), "=r" (high)
    :
	"3" (low), "4" (high)
    );
#endif

    ++ count;
  }
}  // end namespace LOFAR


#endif
