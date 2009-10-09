//# MWGlobalStep.h: A global step in the MWSpec composite pattern
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

#ifndef LOFAR_MWCONTROL_MWGLOBALSPEC_H
#define LOFAR_MWCONTROL_MWGLOBALSPEC_H

// @file
// @brief Class representing a global step in the MWSpec composite pattern
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <MWControl/MWSpec.h>
#include <MWCommon/MWGlobalStep.h>

namespace LOFAR { namespace CEP {

  // @ingroup MWControl
  // @brief Class representing a global step in the MWSpec composite pattern.

  // This is the class representing a global step in the MWSpec composite
  // pattern (see Design Patterns, Gamma et al, 1995).
  // It holds the information about the step in a ParameterSet object.

  class MWGlobalSpec : public MWGlobalStep
  {
  public:
    // Default constructor.
    MWGlobalSpec();

    // Construct from the given .parset file.
    // Unspecified items are taken from the parent specification.
    MWGlobalSpec(const std::string& name,
		 const ParameterSet& parset,
		 const MWSpec* parent);

    virtual ~MWGlobalSpec();

    // Create as a new MWStep object.
    static MWStep::ShPtr create();

    // Register the create function in the MWStepFactory.
    static void registerCreate();

    // Clone the step object.
    virtual MWGlobalSpec* clone() const;

    // Give the (unique) class name of this MWStep.
    virtual std::string className() const;

    // Get the parameter set.
    virtual ParameterSet getParms() const;

    // Convert to/from blob.
    // @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    // @}

    // Print the contents and type. Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

    // Visit the object, so the visitor can process it.
    virtual void visit (MWStepVisitor&) const;

  private:
    MWSpec itsSpec;
  };

}} // end namespaces

#endif
