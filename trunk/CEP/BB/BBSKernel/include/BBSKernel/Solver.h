//# Solver.h: Calculate parameter values using a least squares fitter
//#
//# Copyright (C) 2004
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

#ifndef LOFAR_BB_BBS_SOLVER_H
#define LOFAR_BB_BBS_SOLVER_H

// \file
// Calculates parameter values using a least squares fitter

#include <scimath/Fitting/LSQaips.h>
#include <BBSKernel/ParmData.h>
#include <BBSKernel/Quality.h>
#include <ParmDB/ParmDB.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

// Solver calculates new parameter values from the equations given by the
// Prediffer class.
class Solver
{

public:
  // Create Solver object.
  Solver();

  // Destructor
  ~Solver();

  // Initialize the solvable parm data.
  void initSolvableParmData (int nrPrediffers,
			     const vector<MeqDomain>& solveDomains,
			     const MeqDomain& workDomain);

  // Set the solvable parm data for a given prediffer.
  void setSolvableParmData (const ParmDataInfo&, int prediffer);

  // Get the solvable parameter name info.
  const ParmDataInfo& getSolvableParmData() const
    { return itsParmInfo; }

  // Merge the fitters from the given Prediffer with the global fitter.
  void mergeFitters (const vector<casa::LSQFit>&, int prediffer);

  // Solve all the matrices.
  void solve (bool useSVD);

  // Get the solvable values for the given fitter (i.e. solve domain).
  const vector<double>& getSolvableValues (uint fitterIndex) const;

  // Get quality for the given fitter (i.e. solve domain).
  const Quality& getQuality (uint fitterIndex) const;

  // Show the relevant info.
  void show (std::ostream&);
  
  // Log the quality indicators, and the coefficients of the solvable
  // parameters to a ParmDB.
  void log(LOFAR::ParmDB::ParmDB &table, const string &stepName);

private:
  // Copy constructor and assignment are not allowed.
  // <group>
  Solver(const Solver& other);
  Solver& operator=(const Solver& other);
  // </group>

  // The solver combines the results from multiple Prediffers.
  // Each Prediffer casn have a subset of the solvable parms and a
  // subset of the solve domains.
  // setSolvableData tells for each Prediffer its solvable parms and
  // its solve domains (as contained in ParmDataInfo). For each parm,
  // ParmData gives the number of solvable coefficients per solve domain
  // (in ParmData::PDInfo) and the number of perturbed values (which is the
  // maximum of the number of solvable coefficients for all solve domains).
  //
  // So Solver has to combine the solvable parm info from all Prediffers:
  // - It has to put the LSQFit info from the Prediffers at the correct
  //   place in the global LSQFit objects.
  // - The parm coefficients in the Prediffers have to updated correctly.
  // - The parms in the ParmDB have to updated correctly.
  // For this purpose it holds a few maps per Prediffer:
  // - index of the i-th Prediffer parm in the global parm vector.
  // - per parm a vector telling the number of scids per solve domain.
  // - per parm a vector telling the first scid in the LSQFit per solve domain.
  struct FitterData
  {
    casa::LSQFit   fitter;
    Quality        quality;
    std::vector<double> solvableValues;
    int            nused;
    int            nflagged;
  };
  
  struct PredifferInfo
  {
    vector<int>           solveDomainIndices;
    vector<vector<uint> > scidMap; //# map scids in Prediffer fitters to global
  };

  map<string,int>       itsNameMap;        //# Map parameter name to index
  vector<FitterData>    itsFitters;
  ParmDataInfo          itsParmInfo;       //# Global parm info
  vector<PredifferInfo> itsPredInfo;       //# prediffer info
  bool                  itsDoSet;          //# true = do itsSolver.set
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
