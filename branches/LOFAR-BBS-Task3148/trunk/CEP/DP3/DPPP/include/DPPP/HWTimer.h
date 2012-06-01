//# HWTimer.h: Accurate timer (based on NSTimer).
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef DPPP_HWTIMER_H
#define DPPP_HWTIMER_H

// \file
// Accurate timer (based on NSTimer).

#include <cstdlib>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>

#if defined __ia64__ && defined __INTEL_COMPILER
#include <ia64regs.h>
#endif

namespace LOFAR
{
namespace DPPP
{

// Low-overhead and high-resolution interval timer for use on i386, x86_64,
// ia64, and powerpc platforms, using the processor's timestamp counter that
// is incremented each cycle.

class HWTimer
{
public:
    HWTimer();

    // Start the timer.
    void start();
    // Stop the timer
    void stop();
    // Reset the timer to zero.
    void reset();

    // Get the elapsed time in clock ticks.
    unsigned long long ticks() const;

    // Get the total number of times start() and stop() were called.
    unsigned long long count() const;

private:
    union
    {
        long long   m_total_time;
        struct
        {
#if defined __PPC__
            int m_total_time_high, m_total_time_low;
#else
            int m_total_time_low, m_total_time_high;
#endif
        };
    };

#if defined __i386__ && defined __INTEL_COMPILER && defined _OPENMP
    union
    {
        unsigned long long  m_count;
        struct
        {
            int m_count_low, m_count_high;
        };
    };
#else
    unsigned long long m_count;
#endif
};

inline ostream &operator<<(ostream &os, const HWTimer &obj)
{
    os << "timer: " << setprecision(17) << (obj.ticks() / 1e9) << "/"
        << obj.count() << "/" << (obj.ticks() / (obj.count() * 1e9))
        << " tot/cnt/avg";
    return os;
}

inline void HWTimer::reset()
{
    m_total_time = 0;
    m_count      = 0;
}

inline unsigned long long HWTimer::ticks() const
{
    return m_total_time;
}

inline unsigned long long HWTimer::count() const
{
    return m_count;
}

inline HWTimer::HWTimer()
{
    reset();
}

inline void HWTimer::start()
{
#if defined __x86_64__ && defined __INTEL_COMPILER && defined _OPENMP
    asm volatile
    (
        "rdtsc\n\t"
        "shlq $32,%%rdx\n\t"
        "leaq (%%rax,%%rdx),%%rax\n\t"
        "lock;subq %%rax,%0"
    :
        "+m" (m_total_time)
    :
    :
        "rax", "rdx"
    );
#elif defined __i386__ && defined __INTEL_COMPILER && defined _OPENMP
    asm volatile
    (
        "rdtsc\n\t"
        "lock;subl %%eax,%0\n\t"
        "lock;sbbl %%edx,%1"
    :
        "+m" (m_total_time_low), "+m" (m_total_time_high)
    :
    :
        "eax", "edx"
    );
#elif (defined __i386__ || defined __x86_64__) && (defined __GNUC__ || defined __INTEL_COMPILER)
    asm volatile
    (
        "rdtsc\n\t"
        "subl %%eax, %0\n\t"
        "sbbl %%edx, %1"
    :
        "+m" (m_total_time_low), "+m" (m_total_time_high)
    :
    :
        "eax", "edx"
    );
#elif (defined __i386__ || defined __x86_64__) && defined __PATHSCALE__
    unsigned eax, edx;

    asm volatile ("rdtsc" : "=a" (eax), "=d" (edx));

    m_total_time -= ((unsigned long long) edx << 32) + eax;
#elif defined __ia64__ && defined __INTEL_COMPILER
    m_total_time -= __getReg(_IA64_REG_AR_ITC);
#elif defined __ia64__ && defined __GNUC__
    long long time;
    asm volatile ("mov %0=ar.itc" : "=r" (time));
    m_total_time -= time;
#elif defined __PPC__ && (defined __GNUC__ || defined __xlC__)
    int high, low, retry;

    asm
    (
        "0:\n\t"
        "mfspr %0,269\n\t"
        "mfspr %1,268\n\t"
        "mfspr %2,269\n\t"
        "cmpw %2,%0\n\t"
        "bne 0b\n\t"
        "subfc %3,%1,%3\n\t"
        "subfe %4,%0,%4"
    :
        "=r" (high), "=r" (low), "=r" (retry),
        "=r" (m_total_time_low), "=r" (m_total_time_high)
    :
        "3" (m_total_time_low), "4" (m_total_time_high)
    :
        "cc"
    );
#endif
}

inline void HWTimer::stop()
{
#if defined __x86_64__ && defined __INTEL_COMPILER && defined _OPENMP
    asm volatile
    (
        "rdtsc\n\t"
        "shlq $32,%%rdx\n\t"
        "leaq (%%rax,%%rdx),%%rax\n\t"
        "lock;addq %%rax,%0"
    :
        "+m" (m_total_time)
    :
    :
        "rax", "rdx"
    );
#elif defined __i386__ && defined __INTEL_COMPILER && defined _OPENMP
    asm volatile
    (
        "rdtsc\n\t"
        "lock;addl %%eax, %0\n\t"
        "lock;adcl %%edx, %1"
    :
        "+m" (m_total_time_low), "+m" (m_total_time_high)
    :
    :
        "eax", "edx"
    );
#elif (defined __i386__ || defined __x86_64__) && (defined __GNUC__ || defined __INTEL_COMPILER)
    asm volatile
    (
        "rdtsc\n\t"
        "addl %%eax, %0\n\t"
        "adcl %%edx, %1"
    :
        "+m" (m_total_time_low), "+m" (m_total_time_high)
    :
    :
        "eax", "edx"
    );
#elif (defined __i386__ || defined __x86_64__) && defined __PATHSCALE__
    unsigned eax, edx;

    asm volatile ("rdtsc\n\t" : "=a" (eax), "=d" (edx));
    m_total_time += ((unsigned long long) edx << 32) + eax;
#elif defined __ia64__ && defined __INTEL_COMPILER
    m_total_time += __getReg(_IA64_REG_AR_ITC);
#elif defined __ia64__ && defined __GNUC__
    long long time;
    asm volatile ("mov %0=ar.itc" : "=r" (time));
    m_total_time += time;
#elif defined __PPC__ && (defined __GNUC__ || defined __xlC__)
    int high, low, retry;

    asm
    (
        "0:\n\t"
        "mfspr %0,269\n\t"
        "mfspr %1,268\n\t"
        "mfspr %2,269\n\t"
        "cmpw %2,%0\n\t"
        "bne 0b\n\t"
        "addc %3,%3,%1\n\t"
        "adde %4,%4,%0"
    :
        "=r" (high), "=r" (low), "=r" (retry),
        "=r" (m_total_time_low), "=r" (m_total_time_high)
    :
        "3" (m_total_time_low), "4" (m_total_time_high)
    :
        "cc"
    );
#endif

#if defined __x86_64__ && defined __INTEL_COMPILER && defined _OPENMP
    asm volatile ("lock;addq $1,%0" : "+m" (m_count));
#elif defined __i386__ && defined __INTEL_COMPILER && defined _OPENMP
    asm volatile
    (
        "lock;addl $1,%0\n\t"
        "lock;adcl $0,%1"
    :
        "+m" (m_count_low), "+m" (m_count_high)
    );
#else
    ++ m_count;
#endif
}

}  // namespace DPPP
}  // namespace LOFAR

#endif
