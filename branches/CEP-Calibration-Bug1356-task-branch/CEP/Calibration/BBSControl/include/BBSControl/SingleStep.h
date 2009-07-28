//# SingleStep.h: Derived leaf class of the Step composite pattern.
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

#ifndef LOFAR_BBSCONTROL_BBSSINGLESTEP_H
#define LOFAR_BBSCONTROL_BBSSINGLESTEP_H

// \file
// Derived leaf class of the Step composite pattern.

//# Includes
#include <BBSControl/Step.h>

namespace LOFAR
{
  namespace BBS
  {
    // \ingroup BBSControl
    // @{

    // This is a so-called \e leaf class in the Step composite pattern (see
    // Gamma, 1995).
    // \note %SingleStep not implemented as a leaf class; it contains a
    // number of data members that are common to "real" Step leaf classes,
    // like SolveStep.
    class SingleStep : public Step
    {
    public:
      virtual ~SingleStep();

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // Return the operation type of \c *this as a string.
      virtual const string& operation() const = 0;

      // Return the name of the column to write data to.
      string outputColumn() const { return itsOutputColumn; }
      // Should the flags be written?
      bool writeFlags() const { return itsWriteFlags; }

    protected:
      // Default constructor. Construct an empty SingleStep object and make
      // it a child of the Step object \a parent.
      SingleStep(const Step* parent = 0);

      // Construct a SingleStep having the name \a name. Configuration
      // information for this step can be retrieved from the parameter set \a
      // parset, by searching for keys <tt>Step.\a name</tt>. \a parent
      // is a pointer to the Step object that is the parent of \c *this.
      SingleStep(const string& name, const Step* parent);

      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this,
      // overriding the default values, "inherited" from the parent step
      // object.
      virtual void read(const ParameterSet& ps);

      // Name of the column to write data to.
      string          itsOutputColumn;
      bool            itsWriteFlags;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
