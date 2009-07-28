//# MultiStep.h: Derived composite class of the Step composite pattern.
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
// Derived composite class of the Step composite pattern.

//# Includes
#include <BBSControl/Step.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace BBS
  {
    // \ingroup BBSControl
    // @{

    // This is the so-called \e composite class in the composite pattern (see
    // Gamma, 1995). The composite class contains pointers to zero or more
    // Step (component) objects.
    class MultiStep : public Step
    {
    public:
      typedef vector< shared_ptr<const Step> >::const_iterator const_iterator;
      typedef vector< shared_ptr<const Step> >::const_reverse_iterator
        const_reverse_iterator;

      // Construct a MultiStep. \a name identifies the step name in the
      // parameter set file. It does \e not uniquely identify the step \e
      // object being created. The third argument is used to pass a
      // backreference to the parent Step object.
      MultiStep(const string& name,
                const ParameterSet& parset,
                const Step* parent);

      // Default constructor. Construct an empty MultiStep object and make
      // it a child of the Step object \a parent.
      MultiStep(const Step* parent = 0) :
        Step(parent)
      {
      }

      virtual ~MultiStep();

      // Accept a CommandVisitor that wants to process \c *this.
      virtual CommandResult accept(CommandVisitor &visitor) const;

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // Return the command type of \c *this as a string.
      virtual const string& type() const;
      
      // Provide const iteration over the steps that compose this multi-step.
      const_iterator begin() const
      { return itsSteps.begin(); }
      const_iterator end() const
      { return itsSteps.end(); }

      // Provide const reverse iteration over the steps that compose this multi-
      // step (needed for StrategyIterator).
      const_reverse_iterator rbegin() const
      { return itsSteps.rbegin(); }
      const_reverse_iterator rend() const
      { return itsSteps.rend(); }

    private:
      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ParameterSet& ps);

      // Write the individual Step objects in \a itsSteps, which make up
      // this MultiStep, to the ParameterSet \a ps.
      void writeSteps(ParameterSet& ps) const;

      // Read the individual Step objects, which make up this MultiStep,
      // from the ParameterSet \a ps and store them in \a itsSteps.
      void readSteps(const ParameterSet& ps);

      // Check to see if there's an infinite recursion present in the
      // definition of a MultiStep. This can happen when one of the steps
      // (identified by the argument \a name) defining the MultiStep refers
      // directly or indirectly to that same MultiStep. 
      void infiniteRecursionCheck(const string& name) const;

      // Vector holding a sequence of Steps.
      vector< shared_ptr<const Step> > itsSteps;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
