//# BBSMultiStep.h: Derived composite class of the BBSStep composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSMULTISTEP_H
#define LOFAR_BBSCONTROL_BBSMULTISTEP_H

// \file
// Derived composite class of the BBSStep composite pattern.

//# Includes
#include <BBSControl/BBSStep.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    // This is the so-called \e composite class in the composite pattern (see
    // Gamma, 1995). The composite class contains pointers to zero or more
    // BBSStep (component) objects.
    class BBSMultiStep : public BBSStep
    {
    public:
      // Construct a BBSMultiStep. \a name identifies the step name in the
      // parameter set file. It does \e not uniquely identify the step \e
      // object being created. The third argument is used to pass a
      // backreference to the parent BBSStep object.
      BBSMultiStep(const string& name,
		   const ACC::APS::ParameterSet& parset,
		   const BBSStep* parent);

      // Default constructor. Construct an empty BBSMultiStep object and make
      // it a child of the BBSStep object \a parent.
      BBSMultiStep(const BBSStep* parent = 0) :
        BBSStep(parent)
      {
      }

      virtual ~BBSMultiStep();

      // Accept a CommandVisitor that wants to process \c *this.
      virtual void accept(CommandVisitor &visitor) const;

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // Return the command type of \c *this as a string.
      virtual const string& type() const;

    private:
      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ACC::APS::ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ACC::APS::ParameterSet& ps);

      // Write the individual BBSStep objects in \a itsSteps, which make up
      // this BBSMultiStep, to the ParameterSet \a ps.
      void writeSteps(ACC::APS::ParameterSet& ps) const;

      // Read the individual BBSStep objects, which make up this BBSMultiStep,
      // from the ParameterSet \a ps and store them in \a itsSteps.
      void readSteps(const ACC::APS::ParameterSet& ps);

      // Implementation of getAllSteps() for BBSMultiStep. It retrieves all
      // steps by calling getAllSteps() on all steps that comprise this
      // multistep.
      virtual void doGetAllSteps(vector<const BBSStep*>& steps) const;

      // Check to see if there's an infinite recursion present in the
      // definition of a BBSMultiStep. This can happen when one of the steps
      // (identified by the argument \a name) defining the BBSMultiStep refers
      // directly or indirectly to that same BBSMultiStep. 
      void infiniteRecursionCheck(const string& name) const;

      // Vector holding a sequence of BBSSteps.
      vector<const BBSStep*> itsSteps;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
