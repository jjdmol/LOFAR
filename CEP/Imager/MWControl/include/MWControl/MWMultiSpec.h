//# MWMultiSpec.h: A multi step in the MWSpec composite pattern
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

#ifndef LOFAR_MWCONTROL_MWMULTISPEC_H
#define LOFAR_MWCONTROL_MWMULTISPEC_H

// @file
// @brief Class representing a multi step in the MWSpec composite pattern
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <MWControl/MWSpec.h>
#include <MWCommon/MWMultiStep.h>

namespace LOFAR { namespace CEP {

  // @ingroup MWControl
  // @brief Class representing a multi step in the MWSpec composite pattern.

  // This is the class representing a multi step in the MWSpec composite
  // pattern (see Design Patterns, Gamma et al, 1995).
  // It holds the information about the step in a ParameterSet object.

  class MWMultiSpec : public MWMultiStep
  {
  public:
    // Default constructor.
    MWMultiSpec();

    // Construct from the given .parset file.
    // Unspecified items are taken from the parent specification.
    MWMultiSpec(const std::string& name,
		const ParameterSet& parset,
		const MWSpec* parent);

    virtual ~MWMultiSpec();

    // Create as a new MWStep object.
    static MWStep::ShPtr create();

    // Clone the step object.
    virtual MWMultiSpec* clone() const;

    // Print the contents and type. Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

  private:
    MWSpec itsSpec;
  };

}} // end namespaces

#endif
