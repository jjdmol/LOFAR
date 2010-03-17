//# ParmValue.h: A class containing the values of a parameter
//#
//# Copyright (C) 2008
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

// @file
// @brief A class containing the values of a parameter
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_PARMVALUE_H
#define LOFAR_PARMDB_PARMVALUE_H

#include <ParmDB/Grid.h>
#include <ParmDB/Axis.h>
#include <Common/lofar_smartptr.h>
#include <casa/Arrays/Array.h>
#include <string>
#include <vector>

namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief A class containing the values of a parameter,
  // ParmValue holds the values of a given parameter and domain.
  // The object does not hold the name and domain info itself. Instead its
  // parent object ParmValueSet holds this information.
  //
  // The value is a 2-dim array holding scalar values or the coefficients
  // of a 2-dim funklet. 

  class ParmValue
  {
  public:
    // Define a shared pointer for this type.
    typedef shared_ptr<ParmValue> ShPtr;

    // Define the possible funklet types.
    enum FunkletType {
      // A constant scalar
      Scalar  = 0,
      // A polynomial
      Polc    = 1,
      // A polynomial of logs
      PolcLog = 2
    };

    // Construct with the given scalar value.
    explicit ParmValue (double value=0.);

    // Copy constructor makes a deep copy.
    ParmValue (const ParmValue&);

    ~ParmValue();

    // Assignment makes a deep copy.
    ParmValue& operator= (const ParmValue&);

    // Set as a single scalar value.
    void setScalar (double value);

    // Set as an array of coefficients.
    void setCoeff (const casa::Array<double>&);

    // Set as an array of scalar values with the grid.
    // The shape of grid and values must match.
    void setScalars (const Grid&, const casa::Array<double>&);

    // Set the errors.
    // They must have the same shape as the values, so the values must have
    // been set before.
    void setErrors (const casa::Array<double>&);

    // Get the value shape.
    // <group>
    uint nx() const
      { return static_cast<uint>(itsValues.shape()[0]); }
    uint ny() const
      { return static_cast<uint>(itsValues.shape()[1]); }
    // </group>

    // Get the values.
    // <group>
    const casa::Array<double>& getValues() const
      { return itsValues; }
    casa::Array<double>& getValues()
      { return itsValues; }
    // </group>

    // Get the grid.
    const Grid& getGrid() const
      { return itsGrid; }

    // Are there errors? If false, the result of getErrors is undefined.
    bool hasErrors() const
      { return itsErrors != 0; }

    // Get the arrays with errors. Undefined if <src>getErrors()==false</src>.
    // <group>
    const casa::Array<double>& getErrors() const
      { return *itsErrors; }
    casa::Array<double>& getErrors()
      { return *itsErrors; }
    // </group>

    // Get/set the rowid to remember where the value is stored in the ParmDB.
    // <group>
    int getRowId() const
      { return itsRowId; }
    void setRowId (int rowId)
      { itsRowId = rowId; }
    // </group>
    
  private:
    // Make a deep copy of that.
    void copyOther (const ParmValue& that);

    /// Data members.
    Grid                 itsGrid;
    casa::Array<double>  itsValues;
    casa::Array<double>* itsErrors;
    int                  itsRowId;
  };



  // @brief A class holding information of multiple domains of a a parameter.
  // ParmValueSet holds the information of multiple domains of a parameter.
  // It has a grid defining the domains held.
  // <br>The object can be used by BBSKernel and kept in a MeqParmFunklet.
  // Each of its Funklet objects can keep a reference to the appropriate
  // underlying ParmValue object or to its values.

  class ParmValueSet
  {
  public:

    // Create a parameterset with the given default
    // parm value (which is by default a scalar).
    // If the funklet type is a scalar, the value in the default must contain
    // one value only.
    explicit ParmValueSet (const ParmValue& defaultValue = ParmValue(),
                           ParmValue::FunkletType = ParmValue::Scalar,
                           double perturbation = 1e-6,
                           bool pertRel = true);

    // Create the parameterset for the given domain grid and ParmValue objects.
    // If the funklet type is a scalar, the values in the ParmValues must
    // contain one value only.
    ParmValueSet (const Grid& domainGrid,
                  const std::vector<ParmValue::ShPtr>& values,
                  const ParmValue& defaultValue = ParmValue(),
                  ParmValue::FunkletType type = ParmValue::Scalar,
                  double perturbation = 1e-6,
                  bool pertRel = true);

    // Create the parameterset for the given grid from the given ParmValueSet
    // which should have only one ParmValue.
    // This is meant for solvable parameters using a default value.
    void setSolveGrid (const Grid& solveGrid);

    // Get the funklet type.
    ParmValue::FunkletType getType() const
      { return itsType; }

    // Get/set the mask telling which coefficients are solvable.
    // The array can be empty meaning that all coefficients are solvable.
    // <group>
    const casa::Array<bool>& getSolvableMask() const
      { return itsSolvableMask; }
    void setSolvableMask (const casa::Array<bool>& mask)
      { itsSolvableMask.assign (mask); }
    // <group>

    // Get the perturbation value.
    double getPerturbation() const
      { return itsPerturbation; }

    // Is the perturbation relative or absolute?
    bool getPertRel() const
      { return itsPertRel; }

    // Get access to the grid info, so a domain can be looked up.
    const Grid& getGrid() const
      { return itsDomainGrid; }

    // Get the nr of ParmValues.
    uint size() const
      { return itsValues.size(); }

    // No ParmValues?
    bool empty() const
      { return itsValues.size() == 0; }

    // Get the default ParmValue.
    const ParmValue& getDefParmValue() const
      { return itsDefaultValue; }

    // Get the first ParmValue. If there are no ParmValues, the default
    // ParmValue is returned.
    const ParmValue& getFirstParmValue() const;

    // Get the i-th ParmValue.
    // <group>
    const ParmValue& getParmValue (int i) const
      { return *(itsValues[i]); }
    ParmValue& getParmValue (int i)
      { return *(itsValues[i]); }
    // </group>

    // Get/set the dirty flag.
    // The dirty flag has to be set when a new value is given to a ParmValue. 
    // It indicates that the value has to be written later on.
    // When written, the flag will be cleared.
    // <group>
    bool isDirty() const
      { return itsDirty; }
    void setDirty (bool dirty=true)
      { itsDirty = dirty; }
    // </group>

  private:
    // Helper functions for setSolveGrid.
    // <group>
    void createValues (const Grid& solveGrid);
    void checkGrid (const Grid& solveGrid);
    void addValues (const Grid& solveGrid);
    void addCoeffValues (const Grid& solveGrid);
    ParmValue::ShPtr copyParmCoeff (const ParmValue::ShPtr& pval);
    // </group>

    /// Data members.
    ParmValue::FunkletType itsType;
    double                 itsPerturbation;
    bool                   itsPertRel;
    casa::Array<bool>      itsSolvableMask;
    Grid                   itsDomainGrid;
    std::vector<ParmValue::ShPtr> itsValues;
    ParmValue              itsDefaultValue;
    bool                   itsDirty;
  };

  // @}

} //# end namespace BBS
} //# end namspace LOFAR

#endif
