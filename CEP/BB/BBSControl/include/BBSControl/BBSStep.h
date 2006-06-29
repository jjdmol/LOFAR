//# BBSStep.h: The properties for solvable parameters
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
// The properties for solvable parameters

//# Includes
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

    class BBSStep
    {
    public:

      virtual ~BBSStep();

      // Return the name of \c this.
      const string& name() const { return itsName; }

      // Print the contents of \c this into the output stream \a os.
      virtual void print(ostream& os) const;

//       // Return a pointer to the current BBSMultiStep. Since the base class is
//       // not a BBSMultiStep, it will return a null pointer.
//       virtual BBSMultiStep* getMultiStep() { return 0; }

//       // Add a BBSStep to the current BBSStep.
//       // \note You can \e only add a BBSStep to a BBSMultiStep. 
//       // \throw AssertError if \c this is not an instance of BBSMultiStep.
//       virtual void addStep(const BBSStep*& aStep);

      // Create a new step object. The new step can either be a BBSSingleStep
      // or a BBSMultiStep object. This is determined by examining the
      // parameter set \a parSet. If this set contains a key
      // <tt>Step.<em>name</em>.Steps</tt>, then \a aName is a BBSMultiStep,
      // otherwise it is a SingleStep.
      static BBSStep* create(const string& name,
			     const ACC::APS::ParameterSet& parSet);

    protected:
      BBSStep(const string& name, 
	      const ACC::APS::ParameterSet& parSet);

    private:
      // This struct contains two vectors of stations ID's, which, when paired
      // element-wise, define the baselines to be used in the current step.
      struct Selection
      {
	vector<uint32> station1;
	vector<uint32> station2;
      };

      // Name of this step.
      string                 itsName;

      // Selection of baselines for this step.
      Selection              itsSelection;

      // The sources in the source model.
      vector<string>         itsSources;

      // Write the contents of a BBSStep to an output stream.
      friend ostream& operator<<(ostream&, const BBSStep&);
      friend ostream& operator<<(ostream&, const BBSStep::Selection&); 
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
