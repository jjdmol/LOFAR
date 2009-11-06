//# MWParameterHandler.h: Handle the master-worker part of a .parset file
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

#ifndef LOFAR_MWCONTROL_MWPARAMETERHANDLER_H
#define LOFAR_MWCONTROL_MWPARAMETERHANDLER_H

// @file
// @brief Handle the master-worker part of a .parset file
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/ParameterHandler.h>

namespace LOFAR { namespace CEP {

  // Forward Declarations
  class MWMultiSpec;
  class MWStrategySpec;

  // @ingroup MWControl
  // @brief Handle the master-worker part of a .parset file

  // This class handles the processing of a .parset file
  // It has functions to retrieve the specific master-worker info
  // from the .parset file.
  // These can be data set info, strategy specifications (MWStrategySpec),
  // and step specifications (MWSpec).

  class MWParameterHandler: public ParameterHandler
  {
  public:
    explicit MWParameterHandler (const ParameterSet&);

    // Get the name of the data set.
    std::string getDataSetName() const;

    // Get the number of data parts.
    int getNParts() const;

    // Get all strategies specifications from the parameters.
    std::vector<MWStrategySpec> getStrategies() const;

    // Get all step specifications of a strategy from the parameters.
    MWMultiSpec getSteps (const std::string& name) const;
  };

}} // end namespaces

#endif
