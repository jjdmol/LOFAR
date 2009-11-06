//# MWStrategySpec.h: Specification of a BBS strateg
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

#ifndef LOFAR_MWCONTROL_MWSTRATEGYSPEC_H
#define LOFAR_MWCONTROL_MWSTRATEGYSPEC_H

// @file
// @brief Specification of a BBS strategy
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <MWControl/MWMultiSpec.h>
#include <MWCommon/WorkDomainSpec.h>
#include <string>
#include <vector>
#include <iosfwd>

namespace LOFAR { namespace CEP {

  // @ingroup MWControl
  // @brief Specification of a BBS strategy

  // This class contains the specification of a BBS strategy.
  // It consists of two parts:
  // <ul>
  //  <li> The work domain specification which defines the work domain size and
  //       optionally the basic data selection and integration.
  //  <li> The name of the MWMultiSpec object containing the steps to be performed
  //       when processing the data for this strategy.
  // </ul>
  // The strategy specification is read from a LOFAR .parset file.

  class MWStrategySpec
  {
  public:
    // Default constructor.
    MWStrategySpec()
      {}

    // Construct a MWStrategySpec having the name \a name. Configuration
    // information for this step can be retrieved from the parameter set \a
    // parset, by searching for keys <tt>Strategy.\a name</tt>.
    MWStrategySpec (const std::string& name, const ParameterSet& parset);

    // Get the work domain specification.
    WorkDomainSpec getWorkDomainSpec() const;

    // Print the contents in human readable form into the output stream.
    void print (std::ostream& os) const;

    // Return the step (possibly multi step) specification in this strategy.
    const MWMultiSpec& getSteps() const
      { return itsSteps; }

    // @name Accessor methods
    // @{
    const std::string&       getName() const
      { return itsName; }
    std::vector<std::string> getStations() const;
    std::string              getInputData() const;
    std::vector<std::string> getCorrType() const;
    std::string              getCorrSelection() const;
    DomainShape              getWorkDomainSize() const;
    DomainShape              getIntegration() const;
    // @}

  private:
    // The name of the strategy.
    std::string  itsName;
    // The strategy parset.
    ParameterSet itsParSet;
    // The step(s) in this strategy.
    MWMultiSpec  itsSteps;
  };


  // Write the contents of a BBSStrategy to an output stream.
  std::ostream& operator<< (std::ostream&, const MWStrategySpec&);

}} // end namespaces

#endif
