//# Timer.h: Singleton that can be used to measure the time spent in classes
//# derived from Expr.
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_BBSKERNEL_EXPR_TIMER_H
#define LOFAR_BBSKERNEL_EXPR_TIMER_H

// \file
// Singleton that can be used to measure the time spent in classes derived from
// Expr.

#include <Common/lofar_map.h>
#include <Common/lofar_string.h>

#include <Common/Singleton.h>
#include <Common/Timer.h>

#ifdef LOFAR_BBSKERNEL_EXPR_TIMER
#define EXPR_TIMER_START()              Timer::instance().start(this)
#define EXPR_TIMER_STOP()               Timer::instance().stop(this)
#define EXPR_TIMER_START_NAMED(name)    Timer::instance().start(name)
#define EXPR_TIMER_STOP_NAMED(name)     Timer::instance().stop(name)
#else
#define EXPR_TIMER_START()
#define EXPR_TIMER_STOP()
#define EXPR_TIMER_START_NAMED(name)
#define EXPR_TIMER_STOP_NAMED(name)
#endif

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ExprBase;

class TimerImpl
{
public:
    // Remove all timers. After this, any call to start for a certain timer
    // will start counting from zero.
    void reset();

    // Dump all timers to the output stream.
    void dump(ostream &out) const;

    // Control timers that belong to classes derived from ExprBase.
    void reset(const ExprBase *expr);
    void start(const ExprBase *expr);
    void stop(const ExprBase *expr);

    // Control named timers.
    void start(const string &name);
    void stop(const string &name);

private:
    map<string, NSTimer>    itsTimers;
};

typedef Singleton<TimerImpl>        Timer;

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
