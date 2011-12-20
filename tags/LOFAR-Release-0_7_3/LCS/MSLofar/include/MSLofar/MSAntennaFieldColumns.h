//# MSAntennaFieldColumns.h: provides easy access to MSAntennaField columns
//# Copyright (C) 2011
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
//#
//# @author Ger van Diepen

#ifndef MSLOFAR_MSANTENNAFIELDCOLUMNS_H
#define MSLOFAR_MSANTENNAFIELDCOLUMNS_H

#include <casa/aips.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>
#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/BasicSL/String.h>

namespace LOFAR {

  //# Forward Declaration
  class MSAntennaField;

  // This class provides read-only access to the columns in the MSAntennaField
  // Table. It does the declaration of all the Scalar and ArrayColumns with the
  // correct types, so the application programmer doesn't have to worry about
  // getting those right. There is an access function for every predefined
  // column. Access to non-predefined columns will still have to be done with
  // explicit declarations.

  class ROMSAntennaFieldColumns
  {
  public:

    // Create a columns object that accesses the data in the specified Table.
    ROMSAntennaFieldColumns(const MSAntennaField& msAntennaField);

    // The destructor does nothing special.
    ~ROMSAntennaFieldColumns();

    // Access to columns.
    // <group>
    const casa::ROScalarColumn<casa::Int>& antennaId() const
      { return antennaId_p; }
    const casa::ROScalarColumn<casa::String>& name() const
      { return name_p; }
    const casa::ROArrayColumn<casa::Double>& position() const
      { return position_p; }
    const casa::ROArrayQuantColumn<casa::Double>& positionQuant() const 
      { return positionQuant_p; }
    const casa::ROScalarMeasColumn<casa::MPosition>& positionMeas() const 
      { return positionMeas_p; }
    const casa::ROArrayColumn<casa::Double>& coordinateAxes() const
      { return coordinateAxes_p; }
    const casa::ROArrayQuantColumn<casa::Double>& coordinateaxesQuant() const 
      { return coordinateAxesQuant_p; }
    const casa::ROArrayColumn<casa::Double>& elementOffset() const
      { return elementOffset_p; }
    const casa::ROArrayQuantColumn<casa::Double>& elementOffsetQuant() const 
      { return elementOffsetQuant_p; }
    const casa::ROArrayColumn<casa::Bool>& elementFlag() const
      { return elementFlag_p; }
    const casa::ROScalarColumn<casa::Double>& tileRotation() const
      { return tileRotation_p; }
    const casa::ROScalarQuantColumn<casa::Double>& tileRotationQuant() const 
      { return tileRotationQuant_p; }
    const casa::ROArrayColumn<casa::Double>& tileElementOffset() const
      { return tileElementOffset_p; }
    const casa::ROArrayQuantColumn<casa::Double>& tileElementOffsetQuant() const 
      { return tileElementOffsetQuant_p; }
    // </group>

    // Convenience function that returns the number of rows
    // in any of the columns.
    casa::uInt nrow() const
      { return antennaId_p.nrow(); }

  protected:
    //# Default constructor creates a object that is not usable. Use the attach
    //# function correct this.
    ROMSAntennaFieldColumns();

    //# Attach this object to the supplied table.
    void attach (const MSAntennaField& msAntennaField);

  private:
    //# Make the assignment operator and the copy constructor private to prevent
    //# any compiler generated one from being used.
    ROMSAntennaFieldColumns(const ROMSAntennaFieldColumns&);
    ROMSAntennaFieldColumns& operator=(const ROMSAntennaFieldColumns&);

    //# required columns
    casa::ROScalarColumn<casa::Int> antennaId_p;
    casa::ROScalarColumn<casa::String> name_p;
    casa::ROArrayColumn<casa::Double> position_p;
    casa::ROArrayColumn<casa::Double> coordinateAxes_p;
    casa::ROArrayColumn<casa::Double> elementOffset_p;
    casa::ROArrayColumn<casa::Bool> elementFlag_p;
    casa::ROScalarColumn<casa::Double> tileRotation_p;
    casa::ROArrayColumn<casa::Double> tileElementOffset_p;

    //# Access to Measure columns
    casa::ROScalarMeasColumn<casa::MPosition> positionMeas_p;

    //# Access to Quantum columns
    casa::ROArrayQuantColumn<casa::Double> positionQuant_p;
    casa::ROArrayQuantColumn<casa::Double> coordinateAxesQuant_p;
    casa::ROArrayQuantColumn<casa::Double> elementOffsetQuant_p;
    casa::ROScalarQuantColumn<casa::Double> tileRotationQuant_p;
    casa::ROArrayQuantColumn<casa::Double> tileElementOffsetQuant_p;
  };


  // This class provides read/write access to the columns in the MSAntennaField
  // Table. It does the declaration of all the Scalar and ArrayColumns with the
  // correct types, so the application programmer doesn't have to
  // worry about getting those right. There is an access function
  // for every predefined column. Access to non-predefined columns will still
  // have to be done with explicit declarations.

  class MSAntennaFieldColumns: public ROMSAntennaFieldColumns
  {
  public:

    // Create a columns object that accesses the data in the specified Table.
    MSAntennaFieldColumns(MSAntennaField& msAntennaField);

    // The destructor does nothing special.
    ~MSAntennaFieldColumns();

    // Read-write access to required columns.
    // <group>
    casa::ScalarColumn<casa::Int>& antennaId()
      { return antennaId_p; }
    casa::ScalarColumn<casa::String>& name()
      { return name_p; }
    casa::ArrayColumn<casa::Double>& position()
      { return position_p; }
    casa::ArrayQuantColumn<casa::Double>& positionQuant() 
      { return positionQuant_p; }
    casa::ScalarMeasColumn<casa::MPosition>& positionMeas() 
      { return positionMeas_p; }
    casa::ArrayColumn<casa::Double>& coordinateAxes()
      { return coordinateAxes_p; }
    casa::ArrayQuantColumn<casa::Double>& coordinateaxesQuant() 
      { return coordinateAxesQuant_p; }
    casa::ArrayColumn<casa::Double>& elementOffset()
      { return elementOffset_p; }
    casa::ArrayQuantColumn<casa::Double>& elementOffsetQuant()
      { return elementOffsetQuant_p; }
    casa::ArrayColumn<casa::Bool>& elementFlag()
      { return elementFlag_p; }
    casa::ScalarColumn<casa::Double>& tileRotation()
      { return tileRotation_p; }
    casa::ScalarQuantColumn<casa::Double>& tileRotationQuant() 
      { return tileRotationQuant_p; }
    casa::ArrayColumn<casa::Double>& tileElementOffset()
      { return tileElementOffset_p; }
    casa::ArrayQuantColumn<casa::Double>& tileElementOffsetQuant() 
      { return tileElementOffsetQuant_p; }
    // </group>

  protected:
    //# Default constructor creates a object that is not usable. Use the attach
    //# function correct this.
    MSAntennaFieldColumns();

    //# Attach this object to the supplied table.
    void attach(MSAntennaField& msAntennaField);

  private:
    //# Make the assignment operator and the copy constructor private to prevent
    //# any compiler generated one from being used.
    MSAntennaFieldColumns(const MSAntennaFieldColumns&);
    MSAntennaFieldColumns& operator=(const MSAntennaFieldColumns&);

    //# required columns
    casa::ScalarColumn<casa::Int> antennaId_p;
    casa::ScalarColumn<casa::String> name_p;
    casa::ArrayColumn<casa::Double> position_p;
    casa::ArrayColumn<casa::Double> coordinateAxes_p;
    casa::ArrayColumn<casa::Double> elementOffset_p;
    casa::ArrayColumn<casa::Bool> elementFlag_p;
    casa::ScalarColumn<casa::Double> tileRotation_p;
    casa::ArrayColumn<casa::Double> tileElementOffset_p;

    //# Access to Measure columns
    casa::ScalarMeasColumn<casa::MPosition> positionMeas_p;

    //# Access to Quantum columns
    casa::ArrayQuantColumn<casa::Double> positionQuant_p;
    casa::ArrayQuantColumn<casa::Double> coordinateAxesQuant_p;
    casa::ArrayQuantColumn<casa::Double> elementOffsetQuant_p;
    casa::ScalarQuantColumn<casa::Double> tileRotationQuant_p;
    casa::ArrayQuantColumn<casa::Double> tileElementOffsetQuant_p;
  };

} //# end namespace

#endif
