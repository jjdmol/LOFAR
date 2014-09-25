//# Timer.cc: Singleton that can be used to measure the time spent in classes derived from Expr.
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

#include <lofar_config.h>
#include <BBSKernel/Expr/Timer.h>

#include <lofar_config.h>
#include <BBSKernel/Expr/Timer.h>

#include <BBSKernel/Expr/Expr.h>
#include <Common/lofar_typeinfo.h>

#ifdef HAVE_CPLUS_DEMANGLE
#include <demangle.h>
#endif

namespace LOFAR
{
namespace BBS
{

void TimerImpl::reset()
{
    itsTimers.clear();
}

void TimerImpl::dump(ostream &out) const
{
#ifndef LOFAR_BBSKERNEL_EXPR_TIMER
    out << "Timing of expression tree nodes not supported, please recompile"
        " with -DLOFAR_BBSKERNEL_EXPR_TIMER to enable this feature." << endl;
#else
    typedef map<string, NSTimer>::const_iterator IterType;

    double total = 0.0;
    for(IterType it = itsTimers.begin(), end = itsTimers.end(); it != end;
        ++it)
    {
        string name = it->first;

#ifdef HAVE_CPLUS_DEMANGLE
        char *fqname = cplus_demangle(name.c_str(),
            DMGL_ANSI | DMGL_PARAMS | DMGL_TYPES);
        if(fqname)
        {
            name = string(fqname);
            free(fqname);
        }
#endif
        const NSTimer &timer = it->second;
        double elapsed = timer.getElapsed();
        total += elapsed;
        unsigned long long count = timer.getCount();
        double average = count > 0 ? elapsed / count : 0.0;

        out << "TIMER s EXPRTIMER " << name << " total " << elapsed
            << " count " << count << " avg " << average << endl;
    }

    out << "TIMER s EXPRTIMER ALL total " << total << endl;
#endif
}

void TimerImpl::reset(const ExprBase *expr)
{
    string name(typeid(*expr).name());
    itsTimers[name].reset();
}

void TimerImpl::start(const ExprBase *expr)
{
    string name(typeid(*expr).name());
    start(name);
}

void TimerImpl::stop(const ExprBase *expr)
{
    string name(typeid(*expr).name());
    stop(name);
}

void TimerImpl::start(const string &name)
{
    itsTimers[name].start();
}

void TimerImpl::stop(const string &name)
{
    itsTimers[name].stop();
}

} //# namespace BBS
} //# namespace LOFAR
