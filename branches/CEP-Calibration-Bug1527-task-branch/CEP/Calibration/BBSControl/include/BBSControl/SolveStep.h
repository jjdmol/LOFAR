//# SolveStep.h: The properties for solvable parameters
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_BBSSOLVESTEP_H
#define LOFAR_BBSCONTROL_BBSSOLVESTEP_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBSControl/SingleStep.h>
#include <BBSControl/Types.h>
#include <BBSKernel/Solver.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

#include <measures/Measures/MDirection.h>


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
      virtual CommandResult accept(CommandVisitor &visitor) const;

      // Return the operation type of \c *this as a string.
      virtual const string& operation() const;

      // Print the contents of \c *this in human readable form into the output
      // stream \a os.
      virtual void print(ostream& os) const;

      // @name Accessor methods
      // @{
      vector<string>    parms()             const { return itsParms; }
      vector<string>    exclParms()         const { return itsExclParms; }
      vector<uint32>    calibrationGroups() const
      { return itsCalibrationGroups; }
      bool              globalSolution()    const
      { return !itsCalibrationGroups.empty(); }
      CellSize          cellSize()          const { return itsCellSize; }
      uint32            cellChunkSize()     const { return itsCellChunkSize; }
      bool              propagate()         const { return itsPropagateFlag; }
      bool              resample()          const { return itsResampleFlag; }
      CellSize          resampleCellSize()  const
      { return itsResampleCellSize; }
      double            flagDensityThreshold() const
      { return itsFlagDensityThreshold; }
      bool              shift()             const { return itsShiftFlag; }
      casa::MDirection  direction()         const { return itsDirection; }
      SolverOptions     solverOptions()     const { return itsSolverOptions; }
      // @}

      // Return the command type of \c *this as a string.
      virtual const string& type() const;

    private:
      // Write the contents of \c *this into the ParameterSet \a ps.
      virtual void write(ParameterSet& ps) const;

      // Read the contents from the ParameterSet \a ps into \c *this.
      virtual void read(const ParameterSet& ps);

      void parseResampleCellSize(const ParameterSet& ps);
      void parseDirection(const ParameterSet& ps);

      // Names of the parameters to fit.
      vector<string>    itsParms;
      // Names of the parameters to exclude from fitting.
      vector<string>    itsExclParms;
      // Vector of calibration groups.
      vector<uint32>    itsCalibrationGroups;
      // Solution cell size.
      CellSize          itsCellSize;
      // Number of cells (along the time axis) processed together.
      uint32            itsCellChunkSize;
      // Resample observed visbility data?
      bool              itsResampleFlag;
      // Resolution on which to fit the model.
      CellSize          itsResampleCellSize;
      // Maximal flag density.
      double            itsFlagDensityThreshold;
      // Phase shift observed visibility data?
      bool              itsShiftFlag;
      // Direction to phase shift the visibility data to.
      // TODO: Extend casacore with I/O stream operators for MDirection
      // instances (alla MVAngle but including the reference type, e.g. J2000).
      vector<string>    itsDirectionASCII;
      casa::MDirection  itsDirection;
      // Propagate solutions?
      bool              itsPropagateFlag;
      // Solver options.
      SolverOptions     itsSolverOptions;
    };

    // @}

  } // namespace BBS

} // namespace LOFAR

#endif
