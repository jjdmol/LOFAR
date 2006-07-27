//# BBSStep.h: Base component class of the BBSStep composite pattern.
//#
//# Copyright (C) 2006
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

#ifndef LOFAR_BBSCONTROL_BBSSTEP_H
#define LOFAR_BBSCONTROL_BBSSTEP_H

// \file
// Base component class of the BBSStep composite pattern.

//# Includes
#include <BBSControl/BBSStructs.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iosfwd.h>
#include <Common/LofarTypes.h>

namespace LOFAR
{
  //# Forward Declarations.
  namespace ACC { namespace APS { class ParameterSet; } }

  namespace BBS
  {
    //# Forward Declarations.
    class BBSMultiStep;

    // \addtogroup BBS
    // @{

    // This is the so-called \e component class in the BBSStep composite
    // pattern (see Gamma, 1995). It is the base class for all BBSStep
    // classes, both composite and leaf classes. It has data members that are
    // common to all BBSStep classes.
    class BBSStep
    {
    public:

      // Destructor.
      virtual ~BBSStep();

      // Initialize the data members of the current BBSStep by looking up
      // their key/value pairs in the given ParameterSet. If a key/value pair
      // is not present, then the value will be that of the parent BBSStep
      // object.

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // Return the name of this step.
      const string& getName() const { return itsName; }

      // Return a pointer to the parent of this step.
      const BBSStep* getParent() const { return itsParent; }

      // Return the full name of this step. The full name consists of the name
      // of this step, preceeded by that of its parent, etc., separated by
      // dots.
      string fullName() const;

      // Create a new step object. The new step can either be a BBSSingleStep
      // or a BBSMultiStep object. This is determined by examining the
      // parameter set \a parSet. If this set contains a key
      // <tt>Step.<em>name</em>.Steps</tt>, then \a aName is a BBSMultiStep,
      // otherwise it is a SingleStep. The third, optional, argument is used
      // to pass a backreference to the parent BBSStep object.
      static BBSStep* create(const string& name,
			     const ACC::APS::ParameterSet& parSet,
			     const BBSStep* parent = 0);

    protected:
      // Construct a BBSStep. \a name identifies the step name in the
      // parameter set file. It does \e not uniquely identify the step \e
      // object being created. The third argument is used to pass a
      // backreference to the parent BBSStep object.
      BBSStep(const string& name, 
	      const ACC::APS::ParameterSet& parSet,
	      const BBSStep* parent);

    private:
      // Override the default values, "inherited" from the parent step object,
      // for those members that are specified in \a parSet.
      void setParms(const ACC::APS::ParameterSet& parSet);

      // Name of this step.
      string                 itsName;

      // Pointer to the parent of \c *this. All BBSStep objects have a parent,
      // except the top-level BBSStep object. The parent reference is used,
      // among other things, to initialize the data members of the child
      // object with those of its parent.
      const BBSStep*         itsParent;

      // Selection of baselines for this step.
      Baselines              itsBaselines;

      // Parameters describing which correlation products for which
      // polarizations should be used for this step.
      Correlation            itsCorrelation;

      // Parameters describing the amount of integration that must be applied
      // to the data. Integration can be useful to decrease the amount of
      // data.
      Integration            itsIntegration;

      // The sources in the source model for the current patch.
      vector<string>         itsSources;

      // Extra sources outside the current patch that may contribute to the
      // current patch. They should be taken into account in order to improve
      // the predictions of source parameters for the current patch.
      vector<string>         itsExtraSources;

      // A list of instrument models to be used for this step.
      vector<string>         itsInstrumentModels;

    };

    // Write the contents of a BBSStep to an output stream.
    ostream& operator<<(ostream&, const BBSStep&);

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
