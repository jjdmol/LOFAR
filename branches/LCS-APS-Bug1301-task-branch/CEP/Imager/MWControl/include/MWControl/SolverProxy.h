//# SolverProxy.h: Base class for BBSKernel solver behaviour
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

#ifndef LOFAR_MWCONTROL_SOLVERPROXY_H
#define LOFAR_MWCONTROL_SOLVERPROXY_H

// @file
// @brief Base class for BBSKernel solver behaviour.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/WorkerProxy.h>


namespace LOFAR { namespace CEP {

  // @ingroup MWControl
  // @brief Base class for BBSKernel solver behaviour.

  // This class is the base class for BBSKernel solver behaviour.
  // A derived class needs to do the concrete implementation of the
  // virtual BBSProxy functions.
  // By registering the desired concrete class with type name "Solver"
  // in the WorkerFactory, the MW framework will use that class as the
  // solver proxy.
  //
  // In this way it is possible to use a simple test class (which can
  // merely output the command on cout) to check the control flow.
  // This is used by the test program \a tMWControl.

class SolverProxy: public WorkerProxy
  {
  public:
    SolverProxy()
    {}

    virtual ~SolverProxy();

    // Get the work types supported by the proxy (which is solve).
    virtual std::vector<int> getWorkTypes() const;
  };

}} // end namespaces

#endif
