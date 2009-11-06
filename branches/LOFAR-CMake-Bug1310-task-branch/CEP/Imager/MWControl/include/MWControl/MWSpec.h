//# MWSpec.h: Base component class of the MWSpec composite pattern
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

#ifndef LOFAR_MWCONTROL_MWSPEC_H
#define LOFAR_MWCONTROL_MWSPEC_H

// @file
// @brief Base component class of the MWSpec composite pattern.
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <MWControl/MWParameterHandler.h>
#include <MWCommon/MWStep.h>
#include <string>
#include <vector>


namespace LOFAR { namespace CEP {

  // @ingroup MWControl
  // @brief Base component class of the MWSpec composite pattern.

  // This is the so-called \e component class in the MWSpec composite
  // pattern (see Gamma, 1995). It is the base class for all MWSpec
  // classes, both composite and leaf classes. It has data members that are
  // common to all MWSpec classes.
  //
  // The MWSpec objects contain the specification of the BBS steps to
  // perform in the MW framework. A step can be part of a composite
  // MWMultiSpec object and the specification in there acts as the default
  // value of a step. In that way it is possible to create a composite
  // step object, that can be used with various sky source models.
  //
  // The specification is given in a LOFAR .parset file. In there each
  // step has a name, say XX. Then the \a parset variables <tt>Step.XX.*</tt>
  // contain the specification of XX.
  // A composite object is made by specifying the names of the steps it
  // consists of as <tt>Step.COMP.Steps=["XX", "YY", "ZZ"]</tt>.
  //
  // An MWSpec hierarchy needs to be transformed to an MWStep hierarchy
  // to be able to process the steps. This is done by the derived MWSpecVisitor
  // class MWSpec2Step.
 
  class MWSpec
  {
  public:
    // Construct an empty object.
    MWSpec();

    // Construct a MWSpec. \a name identifies the spec name in the
    // parameter set file. It does \e not uniquely identify the spec \e
    // object being created. The third argument is used to pass a
    // backreference to the parent MWSpec object.
    MWSpec(const std::string& name, 
	   const ParameterSet& parSet,
	   const MWSpec* parent);

    // Destructor. 
    ~MWSpec();

    // Create a new spec object.
    // The new spec can be an MWGlobalSpec, MWLocalSpec, or MWMultiSpec object.
    // This is determined by examining the parameter set \a parSet.
    // If this set contains a key <tt>Step.<em>name</em>.Steps</tt>,
    // then \a aName is a MWMultiSpec,
    // otherwise <tt>Step.<em>name</em>.Global</tt> is examined.
    // <br>The third argument is used to pass a backreference to the parent
    // MWSpec object for the default values.
    // <br> The parset lines of the step are removed from itsParset to keep
    // it small.
    static MWStep::ShPtr createSpec (const std::string& name,
				     const ParameterSet& parSet,
				     const MWSpec* parent);

    // Convert to/from blob.
    // @{
    void toBlob (LOFAR::BlobOStream&) const;
    void fromBlob (LOFAR::BlobIStream&);
    // @}

    // Return the name of this spec.
    std::string getName() const
      { return itsName; }

    // Return the full name of this spec. The full name consists of the name
    // of this spec, preceeded by that of its parent, etc., separated by
    // dots.
    std::string fullName() const;

    // Return a pointer to the parent of this spec.
    const MWSpec* getParent() const
      { return itsParent; }

    // Get a copy of the full parameter set.
    ParameterSet copyFullParSet() const;

    // Get the full parameter set.
    const ParameterSet& getFullParSet() const
      { return itsFullParSet; }

    // Print the info for a given object type.
    void printSpec (std::ostream& os, const std::string& indent,
		    const std::string& type) const;

    // Check if an infinite recursion is given for a MultiSpec.
    void infiniteRecursionCheck (const string& name) const;

  private:
    // Name of this spec.
    std::string   itsName;
    // The parameter specification.
    ParameterSet  itsParSet;
    // The full parameter specification (merged with parents).
    ParameterSet  itsFullParSet;
    // Pointer to the parent of \c *this. All MWSpec objects have a parent,
    // except the top-level MWSpec object. The parent reference is used
    // to complete the specification. I.e. all parameters not specified
    // are inherited from the parent.
    // The class has no knowledge of possible parameters, so it simply merges
    // the parameter set. This is done when specifications are visited,
    // so the original specifications are stored.
    const MWSpec* itsParent;
  };
  
}} // end namespaces

#endif
