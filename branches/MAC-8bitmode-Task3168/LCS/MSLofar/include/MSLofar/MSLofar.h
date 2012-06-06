//# MSLofar.h: Class handling a LOFAR MeasurementSet
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

#ifndef MSLOFAR_MSLOFAR_H
#define MSLOFAR_MSLOFAR_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <MSLofar/MSLofarAntenna.h>
#include <MSLofar/MSLofarField.h>
#include <MSLofar/MSLofarObservation.h>
#include <MSLofar/MSStation.h>
#include <MSLofar/MSAntennaField.h>
#include <MSLofar/MSElementFailure.h>

namespace LOFAR {

  // This class accesses the data in a LOFAR MeasurementSet.
  // It is derived from the casacore MeasurementSet class and adds the
  // LOFAR specific columns and subtables.

  class MSLofar: public casa::MeasurementSet
  {
  public:

    // This constructs an empty MSLofar object, only useful to assign to
    // (it is not a valid MS yet).
    MSLofar();

    // Construct from an existing LOFAR MS table using default locking options.
    MSLofar (const casa::String &tableName,
             casa::Table::TableOption = casa::Table::Old);

    // Construct from an existing LOFAR MS table using given locking options.
    MSLofar (const casa::String &tableName, const casa::TableLock& lockOptions,
             casa::Table::TableOption = casa::Table::Old);

    // Construct a new LOFAR MS using default locking options.
    MSLofar (casa::SetupNewTable &newTab,casa:: uInt nrrow = 0,
             casa::Bool initialize = casa::False);

    // Construct a new LOFAR MS using given locking options.
    MSLofar (casa::SetupNewTable &newTab, const casa::TableLock& lockOptions,
             casa::uInt nrrow = 0, casa::Bool initialize = casa::False);

    // Construct from an existing Table object.
    MSLofar (const casa::Table &table);

    // Copy constructor (reference semantics).
    MSLofar (const MSLofar &other);
    // </group>

    // As with tables, the destructor writes the table if necessary.
    ~MSLofar();

    //  Assignment operator, reference semantics
    MSLofar& operator= (const MSLofar&);

    // Make a special copy of this MS which references all columns from
    // this MS except those mentioned; those are empty and writable.
    // Each forwarded column has the same writable status as the underlying
    // column. The mentioned columns all use the AipsIO storage manager.
    // The main use of this is for the synthesis package where corrected and
    // model visibilities are stored as new DATA columns in an MS which 
    // references the raw MS for the other columns. Except for these special
    // cases, the use of this function will be rare.
    MSLofar referenceCopy (const casa::String& newTableName,
                        const casa::Block<casa::String>& writableColumns) const;

    // Return the name of each of the LOFAR specific subtables.
    // This should be used by the filler to create the subtables in the
    // correct location.
    // <group>
    casa::String stationTableName() const;
    casa::String antennaFieldTableName() const;
    casa::String elementFailureTableName() const;
    // </group>
    
    // Access functions for the LOFAR specific subtables, using the MS-like
    // interface for each.
    // <group>
    MSLofarAntenna& antenna() {return antenna_p;}
    MSLofarField& field() {return field_p;}
    MSLofarObservation& observation() {return observation_p;}
    MSStation& station() {return station_p;}
    MSAntennaField& antennaField() {return antennaField_p;}
    MSElementFailure& elementFailure() {return elementFailure_p;}
    const MSLofarAntenna& antenna() const {return antenna_p;}
    const MSLofarField& field() const {return field_p;}
    const MSLofarObservation& observation() const {return observation_p;}
    const MSStation& station() const {return station_p;}
    const MSAntennaField& antennaField() const {return antennaField_p;}
    const MSElementFailure& elementFailure() const {return elementFailure_p;}
    // </group>

    // Initialize the references to the subtables. You need to call
    // this only if you assign new subtables to the table keywords.
    // This also checks for validity of the table and its subtables.
    // Set clear to True to clear the subtable references (used in assignment)
    void initRefs (casa::Bool clear=casa::False);

    // Create default subtables: fills the required subtable keywords with
    // tables of the correct type, mainly for testing and as an example of
    // how to do this for specific fillers. In practice these tables will
    // often have more things specified, like dimensions of arrays and
    // storage managers for the various columns.
    void createDefaultSubtables
    (casa::Table::TableOption option=casa::Table::Scratch);

    // Flush all the tables and subtables associated with this
    // MeasurementSet. This function calls the Table::flush() function on the
    // main table and all the standard subtables including optional
    // subtables. See the Table class for a description of the sync argument.
    void flush (casa::Bool sync=casa::False);

  private:
    //# keep references to the subtables
    MSLofarAntenna     antenna_p;
    MSLofarField       field_p;
    MSLofarObservation observation_p;
    MSStation          station_p;
    MSAntennaField     antennaField_p;
    MSElementFailure   elementFailure_p;
  };


} //# end namespace

#endif
