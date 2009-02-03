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
#include <ParmDB/ParmFacadeLocal.h>
#include <ParmDB/ParmFacadeDistr.h>
#include <tables/Tables/Table.h>

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
    {
      // If it is a table, open it directly.
      // Otherwise it is a distributed ParmDB.
      if (Table::isReadable(tableName)) {
        itsRep = ParmFacadeRep::ShPtr(new ParmFacadeLocal(tableName));
      } else {
        itsRep = ParmFacadeRep::ShPtr(new ParmFacadeDistr(tableName));
      }
    }

    ParmFacade::~ParmFacade()
    {}

    // Get the parameter values for the given parameters and domain.
    map<string, vector<double> >
    ParmFacade::getValuesMap (const string& parmNamePattern,
                              double freqv1, double freqv2, double freqStep,
                              double timev1, double timev2, double timeStep,
                              bool asStartEnd)
    {
      return record2Map (getValues (parmNamePattern,
                                    freqv1, freqv2, freqStep,
                                    timev1, timev2, timeStep,
                                    asStartEnd));
    }

    map<string,vector<double> >
    ParmFacade::record2Map (const Record& rec) const
    {
      map<string, vector<double> > out;
      // Copy all values (except the grid) from the record to the map.
      for (uint i=0; i<rec.nfields(); ++i) {
        const String& name = rec.name(i);
        if (name != "_grid") {
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
      }
      return out;
    }

  } // namespace ParmDB
} // namespace LOFAR
