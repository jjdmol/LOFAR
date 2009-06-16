//# WorkerFactory.h: Factory pattern to generate a WorkerProxy object
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

#ifndef LOFAR_MWCOMMON_WORKERFACTORY_H
#define LOFAR_MWCOMMON_WORKERFACTORY_H

// @file
// @brief Factory pattern to generate a WorkerProxy object.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/WorkerProxy.h>
#include <map>
#include <string>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Factory pattern to generate a WorkerProxy object.

  // This class contains a map of names to \a create functions
  // of derived WorkerProxy objects. It is used to construct the correct
  // WorkerProxy object given a type name.
  // In this way one can choose which worker to use. For example, it makes
  // it possible to use simple test workers to process prediffer and
  // solver operations to check the control logic.

  class WorkerFactory
  {
  public:
    // Define the signature of the function to create the worker.
    typedef WorkerProxy::ShPtr Creator ();

    // Add a creator function.
    void push_back (const std::string& name, Creator*);
    
    // Create the object of the given name.
    // An exception is thrown if the name is not in the map.
    WorkerProxy::ShPtr create (const std::string& name) const;

  private:
    std::map<std::string, Creator*> itsMap;
  };


}} //# end namespaces

#endif
