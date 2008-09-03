//# MWStepFactory.h: Factory pattern to make the correct MWStep object
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

#ifndef LOFAR_MWCOMMON_MWSTEPFACTORY_H
#define LOFAR_MWCOMMON_MWSTEPFACTORY_H

// @file
// @brief Factory pattern to make the correct MWStep object
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWStep.h>
#include <map>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Factory pattern to make the correct MWStep object

  // This class contains a map of names to \a create functions
  // of derived MWStep objects. It is used to reconstruct the correct
  // MWStep object when reading it back from a blob.
  //
  // The map is static, so there is only one instance in a program.
  // Usually the functions will be registered at the beginning of a program.

  class MWStepFactory
  {
  public:
    // Define the signature of the function to create an MWStep object.
    typedef MWStep::ShPtr Creator();

    // Add a creator function.
    static void push_back (const std::string& name, Creator*);
    
    // Create the derived MWStep object with the given name.
    // An exception is thrown if the name is not in the map.
    static MWStep::ShPtr create (const std::string& name);

  private:
    static std::map<std::string, Creator*> itsMap;
  };


}} //# end namespaces

#endif
