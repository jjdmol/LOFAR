//# lofar_thread.h: namespace wrapper for Boost.Thread
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_COMMON_THREAD_H
#define LOFAR_COMMON_THREAD_H

// \file
// namespace wrapper for Boost.Thread

#if !defined(HAVE_BOOST)
#error Boost.Thread classes are required.
#endif

#ifndef USE_THREADS
#error Threading support unavailable: it should be explicitly enabled \
with USE_THREADS
#endif

#define	BOOST_SP_USE_PTHREADS

#include <boost/thread.hpp>

namespace LOFAR
{
//   using boost::barrier;
  using boost::condition;
  using boost::lock_error;
  using boost::thread_resource_error;
  using boost::mutex;
  using boost::try_mutex;
  using boost::timed_mutex;
  using boost::call_once;
  using boost::recursive_mutex;
  using boost::recursive_try_mutex;
  using boost::recursive_timed_mutex;
  using boost::thread;
  using boost::thread_group;
  using boost::xtime;
  using boost::xtime_get;
}

#define LOFAR_ONCE_INIT BOOST_ONCE_INIT

#endif
