//# ParmCache.cc: A class dealing with caching and handling ParmDB entries
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>
#include <ParmDB/Parm.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/AxisMapping.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>

using namespace casa;

namespace LOFAR {
namespace BBS {

  Parm::Parm (ParmCache& cache, ParmId parmid)
    : itsCache   (&cache),
      itsParmId  (parmid)
  {}

  Parm::Parm (ParmCache& cache, const string& name)
    : itsCache   (&cache),
      itsParmId  (cache.getParmSet().find(name))
  {}

  void Parm::setSolveGrid (const Grid& solveGrid)
  {
    itsCache->setSolveGrid (itsParmId, solveGrid);
    itsSolveGrid = solveGrid;
    // Solve grid is set, so calculate the perturbations.
    calcPerturbations();
  }
  
  uint Parm::getCoeffSize (bool useMask)
  {
    // Use the first ParmValue.
    const ParmValueSet& pvset = itsCache->getValueSet(itsParmId);
    const ParmValue& pv = pvset.getFirstParmValue();
    // The ParmValue can contain a scalar array or coeff.
    // For a scalar array, only one coeff is used.
    if (! pv.hasCoeff()) {
      return 1;
    }
    const Array<double>& values = pv.getValues();
    const Array<bool>&   mask   = pvset.getSolvableMask();
    if (!useMask  ||  mask.size() == 0) {
      return values.size();
    }
    return ntrue(mask);
  }

  vector<double> Parm::getCoeff (const Location& where, bool useMask)
  {
    const ParmValueSet& pvset = itsCache->getValueSet(itsParmId);
    // Find the location in the ParmValueSet grid given the location in
    // the solve grid.
    uint cellId = GridMapping::findCellId (itsCache->getAxisMappingCache(),
					   where, itsSolveGrid,
					   pvset.getGrid());
    const ParmValue& pv = pvset.getParmValue(cellId);
    return makeCoeff (pv.getValues(), pvset.getSolvableMask(), useMask);
  }

  vector<double> Parm::makeCoeff (const Array<double>& values,
				  const Array<Bool>& mask,
				  bool useMask)
  {
    ASSERT (values.contiguousStorage());
    if (!useMask  ||  mask.size() == 0) {
      return vector<double> (values.cbegin(), values.cend());
    }
    ASSERT (values.shape().isEqual(mask.shape()) && mask.contiguousStorage());
    vector<double> solvCoeff;
    solvCoeff.reserve (values.size());
    const double* valp = values.data();
    const bool* maskp  = mask.data();
    for (uint i=0; i<values.size(); ++i) {
      if (maskp[i]) {
	solvCoeff.push_back (valp[i]);
      }
    }
    return solvCoeff;
  }

  void Parm::setCoeff (const Location& where,
		       const double* newValues, uint nvalues,
		       bool useMask)
  {
    ParmValueSet& pvset = itsCache->getValueSet(itsParmId);
    pvset.setDirty();
    const Array<bool>& mask = pvset.getSolvableMask();
    uint cellId = GridMapping::findCellId (itsCache->getAxisMappingCache(),
					   where, itsSolveGrid,
					   pvset.getGrid());
    ParmValue& pv = pvset.getParmValue(cellId);
    Array<double> values (pv.getValues());
    ASSERT (values.contiguousStorage());
    if (!useMask  ||  mask.size() == 0) {
      ASSERT (nvalues == values.size());
      std::copy (newValues, newValues+nvalues, values.cbegin());
      return;
    }
    ASSERT (values.shape().isEqual(mask.shape()) && mask.contiguousStorage());
    double* valp = values.data();
    const bool* maskp  = mask.data();
    for (uint i=0; i<values.size(); ++i) {
      if (maskp[i]) {
	valp[i] = newValues[i];
	nvalues--;
      }
    }
    ASSERT (nvalues == 0);
    // Coefficients have changed, so recalculate the perturbations.
    calcPerturbations();
  }

  void Parm::revertCoeff()
  {
    // Moet ik nog over nadenken.
    // Coefficients have changed, so recalculate the perturbations.
    calcPerturbations();
  }

  void Parm::calcPerturbations()
  {
    const ParmValueSet& pvset = itsCache->getValueSet(itsParmId);
    const ParmValue& pv = pvset.getFirstParmValue();
    itsPerturbations = makeCoeff (pv.getValues(), pvset.getSolvableMask(),
				  true);
    double perturbation = pvset.getPerturbation();
    for (vector<double>::iterator iter=itsPerturbations.begin();
	 iter!=itsPerturbations.end(); ++iter) {
      if (pvset.getPertRel()  &&  std::abs(*iter) > 1e-10) {
	*iter *= perturbation;
      } else {
	*iter = perturbation;
      }
    }
  }

  void Parm::getResult (vector<Array<double> >& result,
			const Grid& predictGrid, bool perturb)
  {
    if (!perturb  ||  itsPerturbations.empty()) {
      // No perturbed values need to be calculated.
      if (result.empty()) {
	result.resize (1);
      }
      getResult (result[0], predictGrid);
    } else {
      // Perturbed values need to be calculated. Make room for them.
      result.resize (itsPerturbations.size() + 1);
      ParmValueSet& pvset = itsCache->getValueSet(itsParmId);
      if (pvset.getFirstParmValue().hasCoeff()) {
	// It is a funklet, so evaluate it.
	getResultCoeff (&(result[0]), predictGrid, pvset, itsPerturbations,
			itsCache->getAxisMappingCache());
      } else {
	// We have scalar values, thus only one perturbed value.
	// First get result and add perturbed value to it.
	getResult (result[0], predictGrid);
	result[1].resize (result[0].shape());
	result[1] = result[0] + itsPerturbations[0];
      }
    }
  }

  void Parm::getResult (Array<double>& result, const Grid& predictGrid)
  {
    // Get the values.
    ParmValueSet& pvset = itsCache->getValueSet(itsParmId);
    if (pvset.getFirstParmValue().hasCoeff()) {
      // It is a funklet, so evaluate it.
      getResultCoeff (&result, predictGrid, pvset, vector<double>(),
		      itsCache->getAxisMappingCache());
    } else if (pvset.getGrid().size() == 1) {
      // Optimize for the often occurring case of a single ParmValue object.
      const ParmValue& pval = pvset.getFirstParmValue();
      if (pval.getGrid().size() == 1) {
	// Only a single value, so size the array accordingly.
	result.resize (IPosition(2,1,1));
	result = pval.getValues();
      } else {
	// There are multiple values, so use the ParmValue's grid.
	getResultScalar (result, predictGrid, pval,
			 itsCache->getAxisMappingCache());
      }
    } else {
      // The hardest case; multiple ParmValues, possibly each with its own grid.
      getResultScalar (result, predictGrid, pvset,
		       itsCache->getAxisMappingCache());
    }
  }

  void Parm::getResultCoeff (Array<double>* resultVec, const Grid& predictGrid,
			     const ParmValueSet& pvset,
			     const vector<double>& perturbations,
			     AxisMappingCache& axisMappingCache)
  {
    Array<double>& result = *resultVec;
    const Axis& paxisx = *predictGrid.getAxis(0);
    const Axis& paxisy = *predictGrid.getAxis(1);
    const Axis& daxisx = *pvset.getGrid().getAxis(0);
    const Axis& daxisy = *pvset.getGrid().getAxis(1);
    // Get the x and y axis mapping of predict grid to domain grid.
    const AxisMapping& mapx = axisMappingCache.get (paxisx, daxisx);
    const AxisMapping& mapy = axisMappingCache.get (paxisy, daxisy);
    int nrdx = daxisx.size();
    const double* cenx = mapx.getScaledCenters();
    const double* ceny = mapy.getScaledCenters();
    // First calculate the main result.
    // Size the array as needed and get an iterator for it.
    result.resize (IPosition(2, paxisx.size(), paxisy.size()));
    Array<double>::iterator resultIter = result.begin();
    const double* pvaly = ceny;
    // Loop over all cells of the predict y-axis.
    for (AxisMapping::const_iterator ity=mapy.begin();
	 ity!=mapy.end(); ++ity) {
      int inxy = *ity * nrdx;
      double valy = *pvaly++;
      const double* pvalx = cenx;
      // Loop over all cells of the predict x-axis.
      for (AxisMapping::const_iterator itx=mapx.begin();
	   itx!=mapx.end(); ++itx) {
	double valx = *pvalx++;
	// Get the coefficients.
	const Array<double>& carr = pvset.getParmValue(*itx+inxy).getValues();
	const double* coeff = carr.data();
	int nrcx = carr.shape()[0];
	int nrcy = carr.shape()[1];
	// Calculate sigma(c[i,j] * x^i * y^j)
	double y = 1;
	double val = 0;
	for (int j=0; j<nrcy; ++j) {
	  double subval = 0;
	  for (int i=nrcx-1; i>0; i--) {
	    subval += coeff[i];
	    subval *= valx;
	  }
	  subval += coeff[0];
	  val += y*subval;
	  y *= valy;
	  coeff += nrcx;
	}
	*resultIter = val;
	++resultIter;
      }
    }
    // Now calculate all perturbed values if needed.
    if (! perturbations.empty()) {
      vector<double> pertCoeff(perturbations.size(), 0.);
      for (uint ip=0; ip<perturbations.size(); ++ip) {
	pertCoeff[ip] = perturbations[ip];
	Array<double>& result = resultVec[ip+1];
	// Size the array as needed and get an iterator for it.
	result.resize (IPosition(2, paxisx.size(), paxisy.size()));
	Array<double>::iterator resultIter = result.begin();
	const double* pvaly = ceny;
	// Loop over all cells of the predict y-axis.
	for (AxisMapping::const_iterator ity=mapy.begin();
	     ity!=mapy.end(); ++ity) {
	  int inxy = *ity * nrdx;
	  double valy = *pvaly++;
	  const double* pvalx = cenx;
	  // Loop over all cells of the predict x-axis.
	  for (AxisMapping::const_iterator itx=mapx.begin();
	       itx!=mapx.end(); ++itx) {
	    double valx = *pvalx++;
	    // Get the coefficients.
	    const Array<double>& carr = pvset.getParmValue(*itx+inxy).getValues();
	    DBGASSERT (carr.size() == perturbations.size());
	    const double* coeff = carr.data();
	    const double* pcoeff = &(pertCoeff[0]);
	    int nrcx = carr.shape()[0];
	    int nrcy = carr.shape()[1];
	    // Calculate sigma(c[i,j] * x^i * y^j)
	    double y = 1;
	    double val = 0;
	    for (int j=0; j<nrcy; ++j) {
	      double subval = 0;
	      for (int i=nrcx-1; i>0; i--) {
		subval += coeff[i] + pcoeff[i];
		subval *= valx;
	      }
	      subval += coeff[0] + pcoeff[0];
	      val += y*subval;
	      y *= valy;
	      coeff += nrcx;
	      pcoeff += nrcx;
	    }
	    *resultIter = val;
	    ++resultIter;
	  }
	}
	pertCoeff[ip] = 0.;
      }
    }
  }

  void Parm::getResultScalar (Array<double>& result, const Grid& predictGrid,
			      const ParmValue& pval,
			      AxisMappingCache& axisMappingCache)
  {
    const Axis& paxisx = *predictGrid.getAxis(0);
    const Axis& paxisy = *predictGrid.getAxis(1);
    const Axis& daxisx = *pval.getGrid().getAxis(0);
    const Axis& daxisy = *pval.getGrid().getAxis(1);
    // Get the x and y axis mapping of predict grid to domain grid.
    const AxisMapping& mapx = axisMappingCache.get (paxisx, daxisx);
    const AxisMapping& mapy = axisMappingCache.get (paxisy, daxisy);
    int nrdx = daxisx.size();
    const double* data = pval.getValues().data();
    // Size the array as needed and get an iterator for it.
    result.resize (IPosition(2, paxisx.size(), paxisy.size()));
    Array<double>::iterator resultIter = result.begin();
    // Loop over all cells of the predict y-axis.
    for (AxisMapping::const_iterator ity=mapy.begin();
	 ity!=mapy.end(); ++ity) {
      int inxy = *ity * nrdx;
      // Loop over all cells of the predict x-axis.
      for (AxisMapping::const_iterator itx=mapx.begin();
	   itx!=mapx.end(); ++itx) {
	*resultIter = data[*itx + inxy];
	++resultIter;
      }
    }
  }

  void Parm::getResultScalar (Array<double>& result, const Grid& predictGrid,
			      const ParmValueSet& pvset,
			      AxisMappingCache& axisMappingCache)
  {
    const Axis& paxisx = *predictGrid.getAxis(0);
    const Axis& paxisy = *predictGrid.getAxis(1);
    const Axis& saxisx = *pvset.getGrid().getAxis(0);
    const Axis& saxisy = *pvset.getGrid().getAxis(1);
    // Get the x and y axis mapping of predict grid to the set's domain grid.
    const AxisMapping& mapx = axisMappingCache.get (paxisx, saxisx);
    const AxisMapping& mapy = axisMappingCache.get (paxisy, saxisy);
    int nrsx = saxisx.size();
    // Get a raw pointer to the result data.
    bool deleteIt;
    double* resData= result.getStorage (deleteIt);
    int nrx = result.shape()[0];
    // Loop through the cells of pvset's grid.
    // Each cell is a ParmValue with its own grid.
    // Fill a part of the result from the ParmValue.
    const vector<int>& bordersx = mapx.getBorders();
    const vector<int>& bordersy = mapy.getBorders();
    int stx = 0;
    int sty = 0;
    for (uint iy=0; iy<bordersy.size(); ++iy) {
      int inxy = nrsx * mapy[sty];
      for (uint ix=0; ix<bordersx.size(); ++ix) {
	getResultPV (resData, nrx, stx, sty, bordersx[ix], bordersy[iy],
		     pvset.getParmValue (mapx[stx] + inxy), predictGrid);
      }
    }
    result.putStorage(resData, deleteIt);
  }

  void Parm::getResultPV (double* resData, int nrx, int stx, int sty,
			  int endx, int endy, const ParmValue& pval,
			  const Grid& predictGrid)
  {
    // Get pointer to the scalar values.
    const double* data = pval.getValues().data();
    // Get the axis of predict grid and domain grid.
    const Axis& paxisx = *predictGrid.getAxis(0);
    const Axis& paxisy = *predictGrid.getAxis(1);
    const Axis& daxisx = *pval.getGrid().getAxis(0);
    const Axis& daxisy = *pval.getGrid().getAxis(1);
    int nrdx = daxisx.size();
    // Loop through all relevant cells of the predict grid.
    // Find the corresponding cell in the domaingrid and copy its value.
    int inxy = 0;
    int inxx = 0;
    for (int iy=sty; iy<endy; ++iy) {
      // Set result pointer to the beginning of this chunk.
      resData += iy*nrx + stx;
      inxy = daxisy.locate (paxisy.center(iy), true, inxy);
      const double* pData = data + inxy*nrdx;
      for (int ix=stx; ix<endx; ++ix) {
	inxx = daxisx.locate (paxisx.center(ix), true, inxx);
	*resData++ = pData[inxx];
      }
    }
  }

} //# end namespace BBS
} //# end namspace LOFAR
