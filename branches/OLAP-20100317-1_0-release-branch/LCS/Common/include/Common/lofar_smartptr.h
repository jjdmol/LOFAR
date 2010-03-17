//# lofar_smartptr.h: namespace wrapper for Boost Smart Pointers
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

#ifndef LOFAR_COMMON_SMARTPTR_H
#define LOFAR_COMMON_SMARTPTR_H

// \file
// namespace wrapper for Boost Smart Pointers

#if !defined(HAVE_BOOST)
#error Boost Smart Pointer classes are required.
#endif

#if defined(USE_THREADS)
# define BOOST_AC_USE_PTHREADS
#else
# define BOOST_DISABLE_THREADS
#endif

#include <boost/smart_ptr.hpp>

namespace LOFAR
{
  using boost::scoped_ptr;
  using boost::scoped_array;
  using boost::shared_ptr;
  using boost::shared_array;
  using boost::weak_ptr;
  using boost::intrusive_ptr;
  using boost::enable_shared_from_this;
  using boost::static_pointer_cast;
  using boost::const_pointer_cast;
  using boost::dynamic_pointer_cast;
}

#endif
