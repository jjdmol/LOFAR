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

#include<lofar_config.h>
#include <MSLofar/MSLofar.h


namespace LOFAR {

  MeasurementSet::MeasurementSet()
  {}

  MeasurementSet::MeasurementSet(const String &tableName,
                                 TableOption option) 
    : MeasurementSet (tableName, option);
  {
    initRefs();
  }

  MeasurementSet::MeasurementSet(const String &tableName,
                                 const TableLock& lockOptions,
                                 TableOption option) 
    : MeasurementSet (tableName, lockOptions, option);
  {
    initRefs();
  }

  MeasurementSet::MeasurementSet(SetupNewTable &newTab, uInt nrrow,
                                 Bool initialize)
    : MeasurementSet (newTab, nrrow, initialize);
  {}

  MeasurementSet::MeasurementSet(SetupNewTable &newTab,
                                 const TableLock& lockOptions, uInt nrrow,
                                 Bool initialize)
    : MeasurementSet (newTab, lockOptions, nrrow, initialize);
  {}

  MeasurementSet::MeasurementSet(const Table &table)
    : MeasurementSet (table)
  {
    initRefs();
  }

  MeasurementSet::MeasurementSet(const MeasurementSet &other)
    : MeasurementSet (other)
  {
    if (!isNull()) initRefs();
  }

  MeasurementSet::~MeasurementSet()
  {}

  MeasurementSet& MeasurementSet::operator=(const MeasurementSet &other)
{
    if (&other != this) {
      MeasurementSet::operator= (other);
      if (!isNull()) initRefs(True);
    }
    return *this;
}

	
  MeasurementSet MeasurementSet::referenceCopy(const String& newTableName, 
                                               const Block<String>& writableColumns) const
  {
    return MSLofar (MeasurementSet::referenceCopy (newTableName,
                                                   writableColumns));
  }


void MeasurementSet::initRefs(Bool clear)
{
  if (isNull()||clear) {
    // clear subtable references
    antenna_p=MSAntenna();
    dataDesc_p=MSDataDescription();
    doppler_p=MSDoppler();
    feed_p=MSFeed();
    field_p=MSField();
    flagCmd_p=MSFlagCmd();
    freqOffset_p=MSFreqOffset();
    history_p=MSHistory();
    observation_p=MSObservation();
    pointing_p=MSPointing();
    polarization_p=MSPolarization();
    processor_p=MSProcessor();
    source_p=MSSource();
    spectralWindow_p=MSSpectralWindow();
    state_p=MSState();
    sysCal_p=MSSysCal();
    weather_p=MSWeather();
  }
  if (!isNull()) {
    // write the table info if needed
    if (this->tableInfo().type()=="") {
      String reqdType=this->tableInfo().type(TableInfo::MEASUREMENTSET);
      this->tableInfo().setType(reqdType);
      String reqdSubType=this->tableInfo().subType(TableInfo::MEASUREMENTSET);
      this->tableInfo().setSubType(reqdSubType);
      this->tableInfo().readmeAddLine("This is a MeasurementSet Table"
				      " holding measurements from a Telescope");
    }
    if(this->tableOption() != Table::Scratch){
      if (this->keywordSet().isDefined("ANTENNA"))
	antenna_p=MSAntenna(this->keywordSet().asTable("ANTENNA", mainLock_p));
      if (this->keywordSet().isDefined("DATA_DESCRIPTION"))
	dataDesc_p=MSDataDescription(this->keywordSet().
				     asTable("DATA_DESCRIPTION", mainLock_p));
      if (this->keywordSet().isDefined("DOPPLER"))
	doppler_p=MSDoppler(this->keywordSet().asTable("DOPPLER", mainLock_p));
      if (this->keywordSet().isDefined("FEED"))
	feed_p=MSFeed(this->keywordSet().asTable("FEED", mainLock_p));
      if (this->keywordSet().isDefined("FIELD"))
	field_p=MSField(this->keywordSet().asTable("FIELD", mainLock_p));
      if (this->keywordSet().isDefined("FLAG_CMD"))
	flagCmd_p=MSFlagCmd(this->keywordSet().asTable("FLAG_CMD", 
						       mainLock_p));
      if (this->keywordSet().isDefined("FREQ_OFFSET"))
	freqOffset_p=MSFreqOffset(this->keywordSet().
				  asTable("FREQ_OFFSET", mainLock_p));
      if (this->keywordSet().isDefined("HISTORY"))
	history_p=MSHistory(this->keywordSet().asTable("HISTORY", mainLock_p));
      if (this->keywordSet().isDefined("OBSERVATION"))
	observation_p=MSObservation(this->keywordSet().
				    asTable("OBSERVATION",mainLock_p));
      if (this->keywordSet().isDefined("POINTING"))
	pointing_p=MSPointing(this->keywordSet().
			      asTable("POINTING", mainLock_p));
      if (this->keywordSet().isDefined("POLARIZATION"))
	polarization_p=MSPolarization(this->keywordSet().
				      asTable("POLARIZATION",mainLock_p));
      if (this->keywordSet().isDefined("PROCESSOR"))
	processor_p=MSProcessor(this->keywordSet().
				asTable("PROCESSOR", mainLock_p));
      if (this->keywordSet().isDefined("SOURCE"))
	source_p=MSSource(this->keywordSet().asTable("SOURCE", mainLock_p));
      if (this->keywordSet().isDefined("SPECTRAL_WINDOW"))
	spectralWindow_p=MSSpectralWindow(this->keywordSet().
					  asTable("SPECTRAL_WINDOW",mainLock_p));
      if (this->keywordSet().isDefined("STATE"))
	state_p=MSState(this->keywordSet().asTable("STATE", mainLock_p));
      if (this->keywordSet().isDefined("SYSCAL"))
	sysCal_p=MSSysCal(this->keywordSet().asTable("SYSCAL", mainLock_p));
      if (this->keywordSet().isDefined("WEATHER"))
	weather_p=MSWeather(this->keywordSet().asTable("WEATHER", mainLock_p));
    }
    else{ //if its scratch...don't bother about the lock as 
          //We can't close and reopen subtables hence we can't use the version 
      //of astable that does that.
      if (this->keywordSet().isDefined("ANTENNA"))
	antenna_p=MSAntenna(this->keywordSet().asTable("ANTENNA"));
      if (this->keywordSet().isDefined("DATA_DESCRIPTION"))
	dataDesc_p=MSDataDescription(this->keywordSet().
				     asTable("DATA_DESCRIPTION"));
      if (this->keywordSet().isDefined("DOPPLER"))
	doppler_p=MSDoppler(this->keywordSet().asTable("DOPPLER"));
      if (this->keywordSet().isDefined("FEED"))
	feed_p=MSFeed(this->keywordSet().asTable("FEED"));
      if (this->keywordSet().isDefined("FIELD"))
	field_p=MSField(this->keywordSet().asTable("FIELD"));
      if (this->keywordSet().isDefined("FLAG_CMD"))
	flagCmd_p=MSFlagCmd(this->keywordSet().asTable("FLAG_CMD"));
      if (this->keywordSet().isDefined("FREQ_OFFSET"))
	freqOffset_p=MSFreqOffset(this->keywordSet().
				  asTable("FREQ_OFFSET"));
      if (this->keywordSet().isDefined("HISTORY"))
	history_p=MSHistory(this->keywordSet().asTable("HISTORY"));
      if (this->keywordSet().isDefined("OBSERVATION"))
	observation_p=MSObservation(this->keywordSet().
				    asTable("OBSERVATION"));
      if (this->keywordSet().isDefined("POINTING"))
	pointing_p=MSPointing(this->keywordSet().
			      asTable("POINTING"));
      if (this->keywordSet().isDefined("POLARIZATION"))
	polarization_p=MSPolarization(this->keywordSet().
				      asTable("POLARIZATION"));
      if (this->keywordSet().isDefined("PROCESSOR"))
	processor_p=MSProcessor(this->keywordSet().
				asTable("PROCESSOR"));
      if (this->keywordSet().isDefined("SOURCE"))
	source_p=MSSource(this->keywordSet().asTable("SOURCE"));
      if (this->keywordSet().isDefined("SPECTRAL_WINDOW"))
	spectralWindow_p=MSSpectralWindow(this->keywordSet().
					  asTable("SPECTRAL_WINDOW"));
      if (this->keywordSet().isDefined("STATE"))
	state_p=MSState(this->keywordSet().asTable("STATE"));
      if (this->keywordSet().isDefined("SYSCAL"))
	sysCal_p=MSSysCal(this->keywordSet().asTable("SYSCAL"));
      if (this->keywordSet().isDefined("WEATHER"))
	weather_p=MSWeather(this->keywordSet().asTable("WEATHER"));


    }
  }
}

void MeasurementSet::createDefaultSubtables(Table::TableOption option)
{
    SetupNewTable antennaSetup(antennaTableName(),
			       MSAntenna::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::ANTENNA),
			       Table(antennaSetup));
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
			       MSField::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::FIELD), Table(fieldSetup));
    SetupNewTable historySetup(historyTableName(),
			       MSHistory::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::HISTORY), 
			       Table(historySetup));
    SetupNewTable observationSetup(observationTableName(),
			       MSObservation::requiredTableDesc(),option);
    rwKeywordSet().defineTable(MS::keywordName(MS::OBSERVATION), 
			       Table(observationSetup));
    SetupNewTable pointingSetup(pointingTableName(),
			       MSPointing::requiredTableDesc(),option);
    // Pointing table can be large, set some sensible defaults for storageMgrs
    IncrementalStMan ismPointing ("ISMPointing");
    StandardStMan ssmPointing("SSMPointing",32768);
    pointingSetup.bindAll(ismPointing,True);
    pointingSetup.bindColumn(MSPointing::columnName(MSPointing::ANTENNA_ID),
			     ssmPointing);
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
    initRefs();
}

Bool MeasurementSet::makeComplexData()
{
  // for now we use an extremely simplistic implementation (should find out
  // storage managers and tiles and keep things the same)
  if (tableDesc().isColumn(MS::columnName(MS::DATA))) return False;
  if (!tableDesc().isColumn(MS::columnName(MS::FLOAT_DATA))) return False;

  // we have FLOAT_DATA but not DATA
  // add DATA
  addColumn(ArrayColumnDesc<Complex>("DATA",2));
  
  // now copy data across from FLOAT_DATA
  ArrayColumn<Float> floatData(*this,MS::columnName(MS::FLOAT_DATA));
  ArrayColumn<Complex> data(*this,MS::columnName(MS::DATA));
  for (uInt i=0; i<nrow(); i++) {
    Array<Float> floatArr(floatData(i));
    Array<Complex> dataArr(floatArr.shape());
    convertArray(dataArr,floatArr);
    data.put(i,dataArr);
  }
  return True;
}

Bool MeasurementSet::validateMeasureRefs()
{
  Bool ok=True;
  // check main table
  {
    Int nCol = tableDesc().ncolumn();
    for (Int i=0; i<nCol; i++) {
      Int fld = tableDesc()[i].keywordSet().fieldNumber("MEASINFO");
      if (fld>=0) {
	Int refFld = tableDesc()[i].keywordSet().asRecord(fld).
	  fieldNumber("Ref");
	if (refFld<0 || tableDesc()[i].keywordSet().asRecord(fld).
	    asString(refFld) == "") {
	  cerr << "Missing Measure reference for column "<<tableDesc()[i].name()
	       << endl;
	  ok = False;
	}
      }
    }
  }
  // check all subtables
  Int nKey = keywordSet().nfields();
  for (Int i=0; i<nKey; i++) {
    if (keywordSet().type(i)== TpTable) {
      Table tab = keywordSet().asTable(i);
      Int nCol = tab.tableDesc().ncolumn();
      for (Int i=0; i<nCol; i++) {
	Int fld = tab.tableDesc()[i].keywordSet().fieldNumber("MEASINFO");
	if (fld>=0) {
	  Int refFld = tab.tableDesc()[i].keywordSet().asRecord(fld).
	    fieldNumber("Ref");
	  if (refFld<0 || tab.tableDesc()[i].keywordSet().asRecord(fld).
	      asString(refFld) == "") {
	    cerr << "Missing Measure reference for column "
		 <<tab.tableDesc()[i].name()<<" in subtable "<<tab.tableName() 
		 << endl;
	    ok = False;
	  }
	}
      }
    }
  }
  return ok;
}

void MeasurementSet::flush(Bool sync) {
  MSTable<MSMainEnums::PredefinedColumns, MSMainEnums::PredefinedKeywords>::flush(sync);
  antenna_p.flush(sync);
  dataDesc_p.flush(sync);
  if (!doppler_p.isNull()) doppler_p.flush(sync);
  feed_p.flush(sync);
  field_p.flush(sync);
  flagCmd_p.flush(sync);
  if (!freqOffset_p.isNull())  freqOffset_p.flush(sync);
  history_p.flush(sync);
  observation_p.flush(sync);
  pointing_p.flush(sync);
  polarization_p.flush(sync);
  processor_p.flush(sync);
  if (!source_p.isNull())  source_p.flush(sync);
  spectralWindow_p.flush(sync);
  state_p.flush(sync);
  if (!sysCal_p.isNull())  sysCal_p.flush(sync);
  if (!weather_p.isNull())  weather_p.flush(sync);
}

void MeasurementSet::checkVersion()
{
  // Check that the MS is the latest version (2.0). Throw an
  // exception and advise the user to use the MS converter if it is not.
  //
  if (!keywordSet().isDefined("MS_VERSION") || 
      (keywordSet().isDefined("MS_VERSION") &&
       keywordSet().asFloat("MS_VERSION")!=2.0)) {
    throw(AipsError("These data are not in MSv2 format - use ms1toms2 to convert"));
  }
}

Record MeasurementSet::msseltoindex(const String& spw, const String& field, 
		      const String& baseline, const String& time, 
		      const String& scan, const String& uvrange, 
				    const String& taql){
  Record retval;
  MSSelection thisSelection;
  thisSelection.setSpwExpr(spw);
  thisSelection.setFieldExpr(field);
  thisSelection.setAntennaExpr(baseline);
  thisSelection.setTimeExpr(time);
  thisSelection.setScanExpr(scan);
  thisSelection.setUvDistExpr(uvrange);
  thisSelection.setTaQLExpr(taql);
  TableExprNode exprNode=thisSelection.toTableExprNode(this);
  Vector<Int> fieldlist=thisSelection.getFieldList();
  Vector<Int> spwlist=thisSelection.getSpwList();
  Vector<Int> scanlist=thisSelection.getScanList();
  Vector<Int> antenna1list=thisSelection.getAntenna1List();
  Vector<Int> antenna2list=thisSelection.getAntenna2List();
  Matrix<Int> chanlist=thisSelection.getChanList();
  Matrix<Int> baselinelist=thisSelection.getBaselineList();
  Vector<Int> ddIDList=thisSelection.getDDIDList();
  OrderedMap<Int, Vector<Int > > polMap=thisSelection.getPolMap();
  OrderedMap<Int, Vector<Vector<Int> > > corrMap=thisSelection.getCorrMap();

  retval.define("spw", spwlist);
  retval.define("field", fieldlist);
  retval.define("scan",scanlist);
  retval.define("antenna1", antenna1list);
  retval.define("antenna2", antenna2list);
  retval.define("baselines",baselinelist);
  retval.define("channel", chanlist);
  retval.define("dd",ddIDList);
  //  retval.define("polmap",polMap);
  // retrval.define("corrmap",corrMap);

  return retval;

}


} //# NAMESPACE CASA - END

