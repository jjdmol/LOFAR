//# BBSSolveStep.h: The properties for solvable parameters
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

#ifndef LOFAR_BBSCONTROL_BBSSOLVESTEP_H
#define LOFAR_BBSCONTROL_BBSSOLVESTEP_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBSControl/BBSSingleStep.h>
#include <BBSControl/BBSStructs.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBS
    // @{

    class BBSSolveStep : public BBSSingleStep
    {
    public:
      // Default constructor. Construct an empty BBSSolveStep object and make
      // it a child of the BBSStep object \a parent.
      BBSSolveStep(const BBSStep* parent = 0);

      // Construct a BBSSolveStep having the name \a name. Configuration
      // information for this step can be retrieved from the parameter set \a
      // parset, by searching for keys <tt>Step.\a name</tt>. \a parent
      // is a pointer to the BBSStep object that is the parent of \c *this.
      BBSSolveStep(const string& name,
		   const ACC::APS::ParameterSet& parset,
		   const BBSStep* parent);

      virtual ~BBSSolveStep();

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

    protected:
      // Write the contents of \c *this into the blob output stream \a bos.
      virtual void write(BlobOStream& bos) const;

      // Read the contents from the blob input stream \a bis into \c *this.
      virtual void read(BlobIStream& bis);

    private:
      uint32 itsMaxIter;            ///< Maximum number of iterations
      double itsEpsilon;            ///< Convergence threshold
      double itsMinConverged;       ///< Fraction that must have converged
      vector<string> itsParms;      ///< Names of the solvable parameters
      vector<string> itsExclParms;  ///< Parameters to be excluded from solve
      DomainSize itsDomainSize;     ///< Solve domain size.

      // Return the type of \c *this as a string.
      virtual const string& type() const;

    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif

