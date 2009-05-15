//# MWError.h: Basic exception for master/worker related errors
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_MWCOMMON_MWERROR_H
#define LOFAR_MWCOMMON_MWERROR_H

// @file
// @brief Basic exception for master/worker related errors.
// @author Ger van Diepen (diepen AT astron nl)

#include <Common/Exception.h>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Basic exception for master/worker related errors.

  // This class defines the basic MW exception.
  // Only this basic exception is defined so far. In the future, some more 
  // fine-grained exceptions might be derived from it.
  EXCEPTION_CLASS(MWError, LOFAR::Exception);

}} //# end namespaces

#endif
