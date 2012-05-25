//# MSLofar.cc: Class handling a LOFAR MeasurementSet
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

#include <lofar_config.h>
#include <MSLofar/MSLofar.h>
#include <MSLofar/MSLofarAntenna.h>
#include <MSLofar/MSLofarField.h>
#include <MSLofar/MSLofarObservation.h>
#include <tables/Tables/SetupNewTab.h>

using namespace casa;

namespace LOFAR {

  MSLofar::MSLofar()
  {}

  MSLofar::MSLofar (const String &tableName,
                    TableOption option) 
    : MeasurementSet (tableName, option)
  {
    initRefs();
  }

  MSLofar::MSLofar (const String &tableName,
                    const TableLock& lockOptions,
                    TableOption option) 
    : MeasurementSet (tableName, lockOptions, option)
  {
    initRefs();
  }

  MSLofar::MSLofar (SetupNewTable &newTab, uInt nrrow,
                    Bool initialize)
    : MeasurementSet (newTab, nrrow, initialize)
  {
  }

  MSLofar::MSLofar (SetupNewTable &newTab,
                    const TableLock& lockOptions, uInt nrrow,
                    Bool initialize)
    : MeasurementSet (newTab, lockOptions, nrrow, initialize)
  {
  }

  MSLofar::MSLofar (const Table &table)
    : MeasurementSet (table)
  {
    initRefs();
  }

  MSLofar::MSLofar (const MSLofar &other)
    : MeasurementSet (other)
  {
    if (!isNull()) {
      initRefs();
    }
  }

  MSLofar::~MSLofar()
  {}

  MSLofar& MSLofar::operator= (const MSLofar &other)
  {
    if (&other != this) {
      MeasurementSet::operator= (other);
      if (!isNull()) {
        initRefs (True);
      }
    }
    return *this;
  }

	
  MSLofar MSLofar::referenceCopy (const String& newTableName, 
                                  const Block<String>& writableColumns) const
  {
    return MSLofar (MeasurementSet::referenceCopy (newTableName,
                                                   writableColumns));
  }


  String MSLofar::stationTableName() const
  {
    if (station_p.isNull()) {
      return tableName()+"/LOFAR_STATION";
    }
    return station_p.tableName();
  }

  String MSLofar::antennaFieldTableName() const
  {
    if (antennaField_p.isNull()) {
      return tableName()+"/LOFAR_ANTENNA_FIELD";
    }
    return antennaField_p.tableName();
  }

  String MSLofar::elementFailureTableName() const
  {
    if (elementFailure_p.isNull()) {
      return tableName()+"/LOFAR_ELEMENT_FAILURE";
    }
    return elementFailure_p.tableName();
  }


  void MSLofar::initRefs (Bool clear)
  {
    // See if a LOFAR info line has to be added.
    Bool addInfoLine = False;
    if (!isNull()) {
      addInfoLine = (this->tableInfo().type().empty());
    }
    // Initialize parent object.
    MeasurementSet::initRefs (clear);
    // Initialize this object.
    if (isNull() || clear) {
      // clear subtable references
      station_p        = MSStation();
      antennaField_p   = MSAntennaField();
      elementFailure_p = MSElementFailure();
    }
    if (!isNull()) {
      if (addInfoLine) {
        this->tableInfo().readmeAddLine("This is a LOFAR MeasurementSet Table");
      }
      // Use the MS's TableLock options for the subtables.
      if (this->tableOption() != Table::Scratch) { 
        if (this->keywordSet().isDefined("ANTENNA")) {
          antenna_p = MSLofarAntenna (this->keywordSet().asTable
                                      ("ANTENNA", lockOptions()));
        }
        if (this->keywordSet().isDefined("FIELD")) {
          field_p = MSLofarField (this->keywordSet().asTable
                                  ("FIELD", lockOptions()));
        }
        if (this->keywordSet().isDefined("OBSERVATION")) {
          observation_p = MSLofarObservation (this->keywordSet().asTable
                                              ("OBSERVATION", lockOptions()));
        }
        if (this->keywordSet().isDefined("LOFAR_STATION")) {
          station_p = MSStation (this->keywordSet().asTable
                                 ("LOFAR_STATION", lockOptions()));
        }
        if (this->keywordSet().isDefined("LOFAR_ANTENNA_FIELD")) {
          antennaField_p = MSAntennaField (this->keywordSet().asTable
                                           ("LOFAR_ANTENNA_FIELD",
                                            lockOptions()));
        }
        if (this->keywordSet().isDefined("LOFAR_ELEMENT_FAILURE")) {
          elementFailure_p = MSElementFailure (this->keywordSet().asTable
                                               ("LOFAR_ELEMENT_FAILURE",
                                                lockOptions()));
        }
      } else {
        // It's scratch...don't bother about the lock as 
        // we can't close and reopen subtables, hence we can't use the version 
        // of asTable that does that.
        if (this->keywordSet().isDefined("ANTENNA")) {
          antenna_p = MSLofarAntenna (this->keywordSet().asTable
                                      ("ANTENNA"));
        }
        if (this->keywordSet().isDefined("FIELD")) {
          field_p = MSLofarField (this->keywordSet().asTable
                                  ("FIELD"));
        }
        if (this->keywordSet().isDefined("OBSERVATION")) {
          observation_p = MSLofarObservation (this->keywordSet().asTable
                                              ("OBSERVATION"));
        }
        if (this->keywordSet().isDefined("LOFAR_STATION")) {
          station_p = MSStation (this->keywordSet().asTable
                                 ("LOFAR_STATION"));
        }
        if (this->keywordSet().isDefined("LOFAR_ANTENNA_FIELD")) {
          antennaField_p = MSAntennaField (this->keywordSet().asTable
                                 ("LOFAR_ANTENNA_FIELD"));
        }
        if (this->keywordSet().isDefined("LOFAR_ELEMENT_FAILURE")) {
          elementFailure_p = MSElementFailure (this->keywordSet().asTable
                                 ("LOFAR_ELEMENT_FAILURE"));
        }
      }
    }
  }

  void MSLofar::createDefaultSubtables(Table::TableOption option)
  {
    SetupNewTable antennaSetup(antennaTableName(),
			       MSLofarAntenna::requiredTableDesc(),option);
    Table antt(antennaSetup);
    antt.flush();
    rwKeywordSet().defineTable(MS::keywordName(MS::ANTENNA), 
                               //    			       Table(antennaSetup));
    			       antt);
   SetupNewTable dataDescSetup(dataDescriptionTableName(),
			       MSDataDescription::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::DATA_DESCRIPTION), 
			       Table(dataDescSetup));
    SetupNewTable feedSetup(feedTableName(),
			       MSFeed::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::FEED), Table(feedSetup));
    SetupNewTable flagCmdSetup(flagCmdTableName(),
			       MSFlagCmd::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::FLAG_CMD), 
			       Table(flagCmdSetup));
    SetupNewTable fieldSetup(fieldTableName(),
			       MSLofarField::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::FIELD), Table(fieldSetup));
    SetupNewTable historySetup(historyTableName(),
			       MSHistory::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::HISTORY), 
			       Table(historySetup));
    SetupNewTable observationSetup(observationTableName(),
			       MSLofarObservation::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::OBSERVATION), 
			       Table(observationSetup));
    SetupNewTable pointingSetup(pointingTableName(),
			       MSPointing::requiredTableDesc(),option);
    // Pointing table can be large, set some sensible defaults for storageMgrs
    ///IncrementalStMan ismPointing ("ISMPointing");
    ///StandardStMan ssmPointing("SSMPointing",32768);
    ///pointingSetup.bindAll(ismPointing,True);
    ///pointingSetup.bindColumn(MSPointing::columnName(MSPointing::ANTENNA_ID),
    ///                         ssmPointing);
    rwKeywordSet().defineTable(MS::keywordName(MS::POINTING),
			       Table(pointingSetup));
    SetupNewTable polarizationSetup(polarizationTableName(),
			       MSPolarization::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::POLARIZATION),
			       Table(polarizationSetup));
    SetupNewTable processorSetup(processorTableName(),
			       MSProcessor::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::PROCESSOR),
			       Table(processorSetup));
    SetupNewTable spectralWindowSetup(spectralWindowTableName(),
			       MSSpectralWindow::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::SPECTRAL_WINDOW),  
			       Table(spectralWindowSetup));
    SetupNewTable stateSetup(stateTableName(),
			       MSState::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::STATE),  
			       Table(stateSetup));
    SetupNewTable stationSetup(stationTableName(),
                               MSStation::requiredTableDesc(), option);
    rwKeywordSet().defineTable("LOFAR_STATION", Table(stationSetup));
    SetupNewTable antFieldSetup(antennaFieldTableName(),
                                MSAntennaField::requiredTableDesc(), option);
    rwKeywordSet().defineTable("LOFAR_ANTENNA_FIELD", Table(antFieldSetup));
    SetupNewTable elemFailSetup(elementFailureTableName(),
                                MSElementFailure::requiredTableDesc(), option);
    rwKeywordSet().defineTable("LOFAR_ELEMENT_FAILURE", Table(elemFailSetup));
    initRefs();
  }

  void MSLofar::flush (Bool sync)
  {
    MeasurementSet::flush (sync);
    station_p.flush (sync);
    antennaField_p.flush (sync);
    elementFailure_p.flush (sync);
  }


} // end namespace
