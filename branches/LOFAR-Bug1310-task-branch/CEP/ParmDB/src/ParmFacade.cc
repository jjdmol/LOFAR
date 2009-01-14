//# ParmFacade.cc: Object access the parameter database
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

#include <lofar_config.h>
#include <ParmDB/ParmFacade.h>
#include <ParmDB/ParmDB.h>
#include <ParmDB/ParmSet.h>
#include <ParmDB/ParmCache.h>
#include <ParmDB/Parm.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <casa/Utilities/Regex.h>

using namespace std;
using namespace casa;

// Create tParmFacade.in_mep with parmdb using:
//   create tablename='tParmFacade.in_mep'
//   add parm1 domain=[1,5,4,10],values=2
//   add parm2 domain=[1,5,4,10],values=[2,0.1],nx=2
//   add parm3 type='expression',expression='parm1*parm2'

namespace LOFAR {
  namespace BBS {

    ParmFacade::ParmFacade (const string& tableName)
      : itsPDB(ParmDBMeta("casa", tableName))
    {}

    ParmFacade::~ParmFacade()
    {}

    vector<double> ParmFacade::getRange (const string& parmNamePattern) const
    {
      string pp = parmNamePattern;
      if (pp.empty()) {
	pp = "*";
      }
      Box dom = itsPDB.getRange (pp);
      vector<double> res(4);
      res[0] = dom.lowerX();
      res[1] = dom.lowerY();
      res[2] = dom.upperX();
      res[3] = dom.upperY();
      return res;
    }

    // Get all parameter names in the table.
    vector<string> ParmFacade::getNames (const string& parmNamePattern) const
    {
      string pp = parmNamePattern;
      if (pp.empty()) {
	pp = "*";
      }
      return itsPDB.getNames(pp);
    }

    // Get the parameter values for the given parameters and domain.
    map<string, vector<double> >
    ParmFacade::getValuesMap (const string& parmNamePattern,
                              double freqv1, double freqv2, int nfreq,
                              double timev1, double timev2, int ntime,
                              bool asStartEnd)
    {
      return record2Map (getValues (parmNamePattern,
                                    freqv1, freqv2, nfreq,
                                    timev1, timev2, ntime,
                                    asStartEnd));
    }

    Record ParmFacade::getValues (const string& parmNamePattern,
                                  double freqv1, double freqv2, int nfreq,
                                  double timev1, double timev2, int ntime,
                                  bool asStartEnd)
    {
      // Create the predict grid.
      Axis::ShPtr axisx (new RegularAxis(freqv1, freqv2, nfreq, asStartEnd));
      Axis::ShPtr axisy (new RegularAxis(timev1, timev2, ntime, asStartEnd));
      return doGetValues (parmNamePattern, Grid(axisx, axisy));
    }

    Record ParmFacade::getValues (const string& parmNamePattern,
                                  const vector<double>& freqv1,
                                  const vector<double>& freqv2,
                                  const vector<double>& timev1,
                                  const vector<double>& timev2,
                                  bool asStartEnd)
    {
      // Create the predict grid.
      Axis::ShPtr axisx (new OrderedAxis(freqv1, freqv2, asStartEnd));
      Axis::ShPtr axisy (new OrderedAxis(timev1, timev2, asStartEnd));
      return doGetValues (parmNamePattern, Grid(axisx, axisy));
    }

    Record ParmFacade::doGetValues (const string& parmNamePattern,
                                    const Grid& predictGrid)
    {
      // Get all matching parm names.
      vector<string> names = getNames (parmNamePattern);
      // The output is returned in a record.
      Record out;
      // Form the names to get.
      // The returned parmId should be the index.
      ParmSet parmSet;
      for (uint i=0; i<names.size(); ++i) {
        ASSERT (parmSet.addParm (itsPDB, names[i]) == i);
      }
      const Axis& axisx = *predictGrid[0];
      const Axis& axisy = *predictGrid[1];
      uint nfreq = axisx.size();
      uint ntime = axisy.size();
      // Create and fill the cache for the given domain.
      Box domain (Point(axisx.lower(0), axisy.lower(0)),
                  Point(axisx.upper(nfreq-1), axisy.upper(ntime-1)));
      ParmCache parmCache(parmSet, domain);
      // Now create the Parm object for each parm and get the values.
      Array<double> result;
      for (uint i=0; i<names.size(); ++i) {
        Parm parm(parmCache, i);
        parm.getResult (result, predictGrid);
        // If the value is constant, the array has only one element.
        // In that case resize it to the full grid.
        if (result.nelements() == 1) {
          double dval = *result.data();
          result.resize (IPosition(2, axisx.size(), axisy.size()));
          result = dval;
        }
        out.define (names[i], result);
      }
      return out;
    }

    Record ParmFacade::getValuesGrid (const string& parmNamePattern,
                                      double sfreq, double efreq,
                                      double stime, double etime)
    {
      // Get all matching parm names.
      vector<string> names = getNames (parmNamePattern);
      // The output is returned in a record.
      Record out;
      Record ginfo;
      // Form the names to get.
      // The returned parmId should be the index.
      ParmSet parmSet;
      for (uint i=0; i<names.size(); ++i) {
        ASSERT (parmSet.addParm (itsPDB, names[i]) == i);
      }
      // Create and fill the cache for the given domain.
      Box domain (Point(sfreq, stime), Point(efreq, etime));
      ParmCache parmCache(parmSet, domain);
      // Now create the Parm object for each parm and get the values.
      Array<double> result;
      for (uint i=0; i<names.size(); ++i) {
        Grid grid (getGrid (parmCache.getValueSet(i), domain));
        Parm parm(parmCache, i);
        parm.getResult (result, grid);
        out.define (names[i], result);
        ginfo.define (names[i] + ";freqs",
                      Vector<double>(grid[0]->centers()));
        ginfo.define (names[i] + ";times",
                      Vector<double>(grid[1]->centers()));
        ginfo.define (names[i] + ";freqwidths",
                      Vector<double>(grid[0]->widths()));
        ginfo.define (names[i] + ";timewidths",
                      Vector<double>(grid[1]->widths()));
      }
      out.defineRecord ("_grid", ginfo);
      return out;
    }

    Grid ParmFacade::getGrid (const ParmValueSet& valueSet, const Box& domain)
    {
      Grid grid(valueSet.getGrid());
      if (valueSet.getType() == ParmValue::Scalar) {
        // For scalars the detailed grids have to be combined.
        vector<Grid> grids;
        grids.reserve (valueSet.size());
        for (uint i=0; i<valueSet.size(); ++i) {
          grids.push_back (valueSet.getParmValue(i).getGrid());
        }
        grid = Grid(grids, true);
      }
      return grid.subset (domain);
    }

    map<string,vector<double> >
    ParmFacade::record2Map (const Record& rec) const
    {
      map<string, vector<double> > out;
      // Copy all values from the record to the map.
      for (uint i=0; i<rec.nfields(); ++i) {
	// First make empty vector; thereafter copy values to it.
	vector<double>& vec = out[rec.name(i)];
	ASSERT (vec.size() == 0);
	// Get result and put in map.
	const Array<double>& arr = rec.asArrayDouble(i);
	bool deleteIt;
	const double* ptr = arr.getStorage (deleteIt);
	// Store the result in the vector.
	vec.assign (ptr, ptr+arr.nelements());
      }
      return out;
    }

  } // namespace ParmDB
} // namespace LOFAR
