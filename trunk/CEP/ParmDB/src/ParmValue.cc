//# ParmValue.cc: A class containing the values of a parameter
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
#include <ParmDB/ParmValue.h>
#include <Common/LofarTypes.h>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace BBS {

  ParmValue::ParmValue (double value)
    : itsErrors (0),
      itsRowId  (-1)
  {
    setScalar (value);
  }

  ParmValue::ParmValue (const ParmValue& that)
    : itsErrors (0)
  {
    copyOther (that);
  }

  ParmValue& ParmValue::operator= (const ParmValue& that)
  {
    if (this != &that) {
      delete itsErrors;
      itsErrors = 0;
      copyOther (that);
    }
    return *this;
  }

  ParmValue::~ParmValue()
  {
    delete itsErrors;
  }

  void ParmValue::copyOther (const ParmValue& that)
  {
    itsGrid     = that.itsGrid;
    itsRowId    = that.itsRowId;
    itsValues.assign (that.itsValues);     // ensure a copy is made
    if (that.itsErrors) {
      itsErrors = new Array<double>;
      *itsErrors = *that.itsErrors;
    }
  }

  void ParmValue::setScalar (double value)
  {
    itsValues.resize (IPosition(2,1,1));
    itsValues = value;
  }

  void ParmValue::setCoeff (const casa::Array<double>& values)
  {
    itsValues.assign (values);
  }

  void ParmValue::setScalars (const Grid& grid,
                              const casa::Array<double>& values)
  {
    ASSERT (int(grid.nx()) == values.shape()[0]  &&
            int(grid.ny()) == values.shape()[1]);
    itsValues.assign (values);
    itsGrid = grid;
  }

  void ParmValue::setErrors (const casa::Array<double>& errors)
  {
    // Make sure a copy is made of the errors.
    if (!itsErrors) {
      itsErrors = new Array<double>();
    }
    itsErrors->assign (errors);
  }



  ParmValueSet::ParmValueSet (const ParmValue& defaultValue,
                              ParmValue::FunkletType type,
                              double perturbation,
                              bool pertRel)
    : itsType         (type),
      itsPerturbation (perturbation),
      itsPertRel      (pertRel),
      itsDefaultValue (defaultValue),
      itsDirty        (false)
  {
    if (type == ParmValue::Scalar) {
      ASSERTSTR (defaultValue.getValues().size() == 1,
                 "Default value of funklet type SCALAR can have one value only");
    }
  }

  ParmValueSet::ParmValueSet (const Grid& domainGrid,
                              const std::vector<ParmValue::ShPtr>& values,
                              const ParmValue& defaultValue,
                              ParmValue::FunkletType type,
                              double perturbation, bool pertRel)
    : itsType         (type),
      itsPerturbation (perturbation),
      itsPertRel      (pertRel),
      itsDomainGrid   (domainGrid),
      itsValues       (values),
      itsDefaultValue (defaultValue),
      itsDirty        (false)
  {
    ASSERT (domainGrid.size() == values.size()  &&  values.size() > 0);
    if (type == ParmValue::Scalar) {
      ASSERTSTR (defaultValue.getValues().size() == 1,
                 "Default value of funklet type SCALAR can have one value only");
      for (uint i=0; i<values.size(); ++i) {
        ASSERTSTR (values[i]->getValues().size() == values[i]->getGrid().size(),
                   "ParmValues of funklet type SCALAR must contain scalar values");
      }
    }
  }

  const ParmValue& ParmValueSet::getFirstParmValue() const
  {
    return itsValues.empty()  ?  itsDefaultValue : *itsValues[0];
  }

  void ParmValueSet::setSolveGrid (const Grid& solveGrid)
  {
    // If the grid is empty, we must add the entire solve grid.
    if (itsDomainGrid.isDefault()) {
      createValues (solveGrid);
    } else {
      // If the entire solve grid is part of the values, check if the
      // grid matches.
      if (itsDomainGrid.getBoundingBox().contains(solveGrid.getBoundingBox())) {
        checkGrid (solveGrid);
      } else {
        // Part of the solve grid does not exist, so they need to be added.
        addValues (solveGrid);
      }
    }
  }

  void ParmValueSet::createValues (const Grid& solveGrid)
  {
    ASSERT (itsValues.empty());
    // If the ParmValue represents coefficients, copy it as often as needed.
    if (itsType != ParmValue::Scalar) {
      itsDomainGrid = solveGrid;
      uint nrv = itsDomainGrid.size();
      itsValues.reserve (nrv);
      for (uint i=0; i<nrv; ++i) {
        itsValues.push_back (ParmValue::ShPtr(new ParmValue(itsDefaultValue)));
      }
    } else {
      // Otherwise it is an array of scalar values, so form the array.
      Array<double> values(IPosition(2, solveGrid.nx(), solveGrid.ny()));
      // Set it to the default value.
      values = itsDefaultValue.getValues().data()[0];
      ParmValue::ShPtr newVal(new ParmValue());
      newVal->setScalars (solveGrid, values);
      itsValues.push_back (newVal);
      itsDomainGrid = Grid(vector<Box>(1, solveGrid.getBoundingBox()));
    }
  }

  void ParmValueSet::checkGrid (const Grid& solveGrid)
  {
    // Check if the solve grid intervals match the domain grid.
    // If the values represent coefficients, the domain grid is the final grid
    // which should match the solve grid.
    if (itsType != ParmValue::Scalar) {
      ASSERT (itsDomainGrid.checkIntervals (solveGrid));
    } else {
      // Each ParmValue has its own grid which has to be checked.
      if (itsValues.size() == 1) {
        // Only one value, so its grid should match.
        ASSERT (itsValues[0]->getGrid().checkIntervals (solveGrid));
      } else {
        // The domain grid is split, so check each part with the corresponding
        // subset of the solve grid.
        for (uint i=0; i<itsDomainGrid.size(); ++i) {
          ASSERT (itsValues[i]->getGrid().checkIntervals (solveGrid));
        }
      }
    }
  }

  void ParmValueSet::addValues (const Grid& solveGrid)
  {
    // Add values and extend the domain grid.
    // If the values represent coefficients, the domain grid is the final grid
    // which should match the solve grid.
    if (itsType != ParmValue::Scalar) {
      addCoeffValues (solveGrid);
    } else {
      // The values is an array of scalars.
      // For now only a single array can be handled.
      // If there are multiple arrays, it is (too) hard to decide which one
      // gets extended or if a new array has to be added.
      ASSERT (itsValues.size() == 1);
      ParmValue& value = *itsValues[0];
      int sx1,ex1,sx2,ex2,sy1,sy2,ey1,ey2;
      Axis::ShPtr xaxis = value.getGrid().getAxis(0)->combine
        (*solveGrid.getAxis(0), sx1, ex1, sx2, ex2);
      Axis::ShPtr yaxis = value.getGrid().getAxis(1)->combine
        (*solveGrid.getAxis(1), sy1, ey1, sy2, ey2);
      Grid newGrid(xaxis, yaxis);
      Array<double> newValues(IPosition(2, newGrid.nx(), newGrid.ny()));
      // Copy the old values.
      newValues(IPosition(2,sx1,sy1),
                IPosition(2,ex1-1,ey1-1)) = value.getValues();
      // Fill in the other values.
      // In the extreme case the old values are in the middle of the
      // new values, so all sides have to be filled.
      // The values before are filled with the first old value, the values
      // after with the last old value.
      // In this way we achieve that new solutions are initialized with
      // existing ones.
      // First copy the values before and after the x-part of the old values.
      for (int iy=sy1; iy<ey1; ++iy) {
        for (int ix=sx2; ix<sx1; ++ix) {
          newValues(IPosition(2,ix,iy)) = newValues(IPosition(2,sx1,iy));
        }
        for (int ix=ex1; ix<ex2; ++ix) {
          newValues(IPosition(2,ix,iy)) = newValues(IPosition(2,ex1-1,iy));
        }
      }
      // Now copy the values before and after the y-part of the old values.
      int nrx = newValues.shape()[0];
      for (int iy=sy2; iy<sy1; ++iy) {
        for (int ix=0; ix<nrx; ++ix) {
          newValues(IPosition(2,ix,iy)) = newValues(IPosition(2,ix,sy1));
        }
      }
      for (int iy=ey1; iy<ey2; ++iy) {
        for (int ix=0; ix<nrx; ++ix) {
          newValues(IPosition(2,ix,iy)) = newValues(IPosition(2,ix,ey1-1));
        }
      }
      value.setScalars (newGrid, newValues);
      itsDomainGrid = Grid(vector<Box>(1, newGrid.getBoundingBox()));
    }
  }

  void ParmValueSet::addCoeffValues (const Grid& solveGrid)
  {
    // Combine the domain grid and the solve grid to form the new domain grid.
    // The values sx and ex give for each axis the start and end of the old
    // axes in the new one.
    int sx1,ex1,sx2,ex2,sy1,sy2,ey1,ey2;
    Axis::ShPtr xaxis = itsDomainGrid.getAxis(0)->combine
      (*solveGrid.getAxis(0), sx1, ex1, sx2, ex2);
    Axis::ShPtr yaxis = itsDomainGrid.getAxis(1)->combine
      (*solveGrid.getAxis(1), sy1, ey1, sy2, ey2);
    Grid newGrid(xaxis, yaxis);
    // Now copy existing parm values and insert new ones as necessary.
    // Take care that the ParmValues are in the correct order
    // (i.e. in order of the cells in the new grid).
    int nx = xaxis->size();
    int ny = yaxis->size();
    vector<ParmValue::ShPtr> newValues(nx*ny);
    // Copy the old values.
    int oldnx = ex1-sx1;
    int oldny = ey1-sy1;
    DBGASSERT (oldnx*oldny == int(itsValues.size()));
    const ParmValue::ShPtr* oldValues = &(itsValues[0]);
    for (int iy=0; iy<ey1-sy1; ++iy) {
      for (int ix=0; ix<ex1-sx1; ++ix) {
        newValues[sx1+ix + (sy1+iy)*nx] = *oldValues++;
      }
    }
    // Fill in the other values.
    // In the extreme case the old values are in the middle of the
    // new values, so all sides have to be filled.
    // The values before are filled with the first old value, the values
    // after with the last old value.
    // In this way we achieve that new solutions are initialized with
    // existing ones.
    // First copy the values before and after the x-part of the old values.
    for (int iy=sy1; iy<ey1; ++iy) {
      for (int ix=sx2; ix<sx1; ++ix) {
        newValues[ix+iy*nx] = copyParmCoeff (newValues[sx1+iy*nx]);
      }
      for (int ix=ex1; ix<ex2; ++ix) {
        newValues[ix+iy*nx] = copyParmCoeff (newValues[ex1-1+iy*nx]);
      }
    }
    // Now copy the values before and after the y-part of the old values.
    for (int iy=sy2; iy<sy1; ++iy) {
      for (int ix=0; ix<nx; ++ix) {
        newValues[ix+iy*nx] = copyParmCoeff (newValues[ix+sy1*nx]);
      }
    }
    for (int iy=ey1; iy<ey2; ++iy) {
      for (int ix=0; ix<nx; ++ix) {
        newValues[ix+iy*nx] = copyParmCoeff (newValues[ix+(ey1-1)*nx]);
      }
    }
    // Use new values and new grid.
    itsValues.swap (newValues);
    itsDomainGrid = newGrid;
  }

  ParmValue::ShPtr ParmValueSet::copyParmCoeff (const ParmValue::ShPtr& pval)
  {
    ParmValue::ShPtr newpval (new ParmValue(*pval));
    newpval->setRowId (-1);
    return newpval;
  }

} //# end namespace BBS
} //# end namspace LOFAR
