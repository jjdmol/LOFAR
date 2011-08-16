//# MSLofarAntennaColumns.h: provides easy access to LOFAR's MSAntenna columns
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

#ifndef MSLOFAR_MSLOFARANTENNACOLUMNS_H
#define MSLOFAR_MSLOFARANTENNACOLUMNS_H

#include <ms/MeasurementSets/MSAntennaColumns.h>

namespace LOFAR {

  //# Forward Declaration
  class MSLofarAntenna;

  // This class provides read-only access to the columns in the MSLofarAntenna
  // Table. It does the declaration of all the Scalar and ArrayColumns with the
  // correct types, so the application programmer doesn't have to worry about
  // getting those right. There is an access function for every predefined
  // column. Access to non-predefined columns will still have to be done with
  // explicit declarations.

  class ROMSLofarAntennaColumns: public casa::ROMSAntennaColumns
  {
  public:

    // Create a columns object that accesses the data in the specified Table.
    ROMSLofarAntennaColumns(const MSLofarAntenna& msLofarAntenna);

    // The destructor does nothing special.
    ~ROMSLofarAntennaColumns();

    // Access to columns.
    // <group>
    const casa::ROScalarColumn<casa::Int>& stationId() const
      { return stationId_p; }
    const casa::ROArrayColumn<casa::Double>& phaseReference() const
      { return phaseReference_p; }
    const casa::ROArrayQuantColumn<casa::Double>& phaseReferenceQuant() const 
      { return phaseReferenceQuant_p; }
    const casa::ROScalarMeasColumn<casa::MPosition>& phaseReferenceMeas() const 
      { return phaseReferenceMeas_p; }
    // </group>

  protected:
    //# Default constructor creates a object that is not usable. Use the attach
    //# function correct this.
    ROMSLofarAntennaColumns();

    //# Attach this object to the supplied table.
    void attach (const MSLofarAntenna& msLofarAntenna);

  private:
    //# Make the assignment operator and the copy constructor private to prevent
    //# any compiler generated one from being used.
    ROMSLofarAntennaColumns(const ROMSLofarAntennaColumns&);
    ROMSLofarAntennaColumns& operator=(const ROMSLofarAntennaColumns&);

    //# required columns
    casa::ROScalarColumn<casa::Int> stationId_p;
    casa::ROArrayColumn<casa::Double> phaseReference_p;
    //# Access to Measure columns
    casa::ROScalarMeasColumn<casa::MPosition> phaseReferenceMeas_p;
    //# Access to Quantum columns
    casa::ROArrayQuantColumn<casa::Double> phaseReferenceQuant_p;
  };


  // This class provides read/write access to the columns in the MSLofarAntenna
  // Table. It does the declaration of all the Scalar and ArrayColumns with the
  // correct types, so the application programmer doesn't have to
  // worry about getting those right. There is an access function
  // for every predefined column. Access to non-predefined columns will still
  // have to be done with explicit declarations.

  class MSLofarAntennaColumns: public casa::MSAntennaColumns
  {
  public:

    // Create a columns object that accesses the data in the specified Table.
    MSLofarAntennaColumns(MSLofarAntenna& msLofarAntenna);

    // The destructor does nothing special.
    ~MSLofarAntennaColumns();

    // Read-write access to required columns.
    // <group>
    const casa::ROScalarColumn<casa::Int>& stationId() const
      { return roStationId_p; }
    casa::ScalarColumn<casa::Int>& stationId()
      { return rwStationId_p; }
    const casa::ROArrayColumn<casa::Double>& phaseReference() const
      { return roPhaseReference_p; }
    casa::ArrayColumn<casa::Double>& phaseReference()
      { return rwPhaseReference_p; }
    const casa::ROArrayQuantColumn<casa::Double>& phaseReferenceQuant() const 
      { return roPhaseReferenceQuant_p; }
    casa::ArrayQuantColumn<casa::Double>& phaseReferenceQuant()
      { return rwPhaseReferenceQuant_p; }
    const casa::ROScalarMeasColumn<casa::MPosition>& phaseReferenceMeas() const
      { return roPhaseReferenceMeas_p; }
    casa::ScalarMeasColumn<casa::MPosition>& phaseReferenceMeas()
      { return rwPhaseReferenceMeas_p; }
    // </group>

  protected:
    //# Default constructor creates a object that is not usable. Use the attach
    //# function correct this.
    MSLofarAntennaColumns();

    //# Attach this object to the supplied table.
    void attach(MSLofarAntenna& msLofarAntenna);

  private:
    //# Make the assignment operator and the copy constructor private to prevent
    //# any compiler generated one from being used.
    MSLofarAntennaColumns(const MSLofarAntennaColumns&);
    MSLofarAntennaColumns& operator=(const MSLofarAntennaColumns&);

    //# required columns
    casa::ROScalarColumn<casa::Int> roStationId_p;
    casa::ScalarColumn<casa::Int>   rwStationId_p;
    casa::ROArrayColumn<casa::Double> roPhaseReference_p;
    casa::ArrayColumn<casa::Double>   rwPhaseReference_p;
    //# Access to Measure columns
    casa::ROScalarMeasColumn<casa::MPosition> roPhaseReferenceMeas_p;
    casa::ScalarMeasColumn<casa::MPosition>   rwPhaseReferenceMeas_p;
    //# Access to Quantum columns
    casa::ROArrayQuantColumn<casa::Double> roPhaseReferenceQuant_p;
    casa::ArrayQuantColumn<casa::Double>   rwPhaseReferenceQuant_p;
  };

} //# end namespace

#endif
