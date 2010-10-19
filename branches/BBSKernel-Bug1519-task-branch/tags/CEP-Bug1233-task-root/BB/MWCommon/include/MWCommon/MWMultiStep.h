//# MWMultiStep.h: A step consisting of several other steps
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

#ifndef LOFAR_MWCOMMON_MWMULTISTEP_H
#define LOFAR_MWCOMMON_MWMULTISTEP_H

// @file
// @brief A step consisting of several other steps.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWStep.h>
#include <list>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief A step consisting of several other steps.

  // This class makes it possible to form a list of MWStep objects.
  // Note that the class itself is an MWStep, so the list can be nested.
  // The \a visit function will call \a visit of each step in the list.
  //
  // It uses the standard MWStep functionality (factory and visitor) to
  // create and process the object.
  // The object can be converted to/from blob, so it can be sent to workers.

  class MWMultiStep: public MWStep
  {
  public:
    virtual ~MWMultiStep();

    // Clone the step object.
    virtual MWMultiStep* clone() const;

    // Create a new object of this type.
    static MWStep::ShPtr create();

    // Register the create function in the MWStepFactory.
    static void registerCreate();

    // Add a clone of a step object.
    void push_back (const MWStep&);

    // Add a step object.
    void push_back (const MWStep::ShPtr&);

    // Give the (unique) class name of the MWStep.
    virtual std::string className() const;

    // Visit the object, which visits each step.
    virtual void visit (MWStepVisitor&) const;

    // Convert to/from blob.
    // Note that reading back from a blob uses MWStepFactory to
    // create the correct objects.
    // @{
    virtual void toBlob (LOFAR::BlobOStream&) const;
    virtual void fromBlob (LOFAR::BlobIStream&);
    // @}

    // Print the contents and type. Indent as needed.
    virtual void print (std::ostream& os, const std::string& indent) const;

    // Define functions and so to iterate in the STL way.
    // @{
    typedef std::list<MWStep::ShPtr>::const_iterator const_iterator;
    const_iterator begin() const
      { return itsSteps.begin(); }
    const_iterator end() const
      { return itsSteps.end(); }
    // @}

  private:
    std::list<MWStep::ShPtr> itsSteps;
  };

}} //# end namespaces

#endif
