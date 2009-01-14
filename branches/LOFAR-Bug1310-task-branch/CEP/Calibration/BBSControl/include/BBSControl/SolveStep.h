//# SolveStep.h: The properties for solvable parameters
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
#include <BBSControl/SingleStep.h>
#include <BBSControl/Types.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBSControl
    // @{

    class SolveStep : public SingleStep
    {
    public:
      // Default constructor. Construct an empty SolveStep object and make
      // it a child of the Step object \a parent.
      SolveStep(const Step* parent = 0);

      // Construct a SolveStep having the name \a name. Configuration
      // information for this step can be retrieved from the parameter set \a
      // parset, by searching for keys <tt>Step.\a name</tt>. \a parent
      // is a pointer to the Step object that is the parent of \c *this.
      SolveStep(const string& name,
                const ParameterSet& parset,
                const Step* parent);

      virtual ~SolveStep();

      // Accept a CommandVisitor that wants to process \c *this.
      virtual void accept(CommandVisitor &visitor) const;

      // Return the operation type of \c *this as a string.
      virtual const string& operation() const;

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // @name Accessor methods
      // @{
      vector<string> parms()              const { return itsParms; }
      vector<string> exclParms()          const { return itsExclParms; }
      vector<uint32> calibrationGroups()  const { return itsCalibrationGroups; }
      CellSize       cellSize()           const { return itsCellSize; }
      uint32         cellChunkSize()      const { return itsCellChunkSize; }
      bool           propagate()          const { return itsPropagateFlag; }
      SolverOptions  solverOptions()      const { return itsSolverOptions; }
      // @}

      // Return the command type of \c *this as a string.
      virtual const string& type() const;

    private:
      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ParameterSet& ps);

      vector<string> itsParms;         ///< Names of the solvable parameters
      vector<string> itsExclParms;     ///< Parameters excluded from solve
      vector<uint32> itsCalibrationGroups; ///< Vector of calibration groups.
      CellSize       itsCellSize;      ///< Solution cell size.
      uint32         itsCellChunkSize; ///< Number of cells processed together.
      bool           itsPropagateFlag; ///< Propagate solutions?
      SolverOptions  itsSolverOptions; ///< Solver options
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif

