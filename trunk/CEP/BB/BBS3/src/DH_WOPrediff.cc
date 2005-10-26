//  DH_WOPrediff.cc:
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <BBS3/DH_WOPrediff.h>
#include <PL/TPersistentObject.h>
#include <APS/ParameterSet.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{

const unsigned int MaxKSTypeLength = 16;
const unsigned int MaxModelTypeLength = 16;

DH_WOPrediff::DH_WOPrediff (const string& name)
  : DH_PL(name, "DH_WOPrediff", 1),
    itsWOID            (0),
    itsSCID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsDoNothing       (0),
    itsNewBaselines    (0),
    itsNewDomain       (0),
    itsNewPeelSources  (0),
    itsSubtractSources (0),
    itsWritePredData   (0),
    itsWriteInDataCol  (0),
    itsStartFreq       (0),
    itsFreqLength      (0),
    itsStartTime       (0),
    itsTimeLength      (0),
    itsModelType       (0),
    itsCalcUVW         (0),
    itsUseAutoCorr     (0),
    itsCleanUp         (0),
    itsUpdateParms     (0),
    itsSolutionID      (0),
    itsPODHWO          (0)
{
  LOG_TRACE_FLOW("DH_WOPrediff constructor");
  setExtraBlob("Extra", 1);
}

DH_WOPrediff::DH_WOPrediff(const DH_WOPrediff& that)
  : DH_PL(that),
    itsWOID            (0),
    itsSCID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsDoNothing       (0),
    itsNewBaselines    (0),
    itsNewDomain       (0),
    itsNewPeelSources  (0),
    itsSubtractSources (0),
    itsWritePredData   (0),
    itsWriteInDataCol  (0),
    itsStartFreq       (0),
    itsFreqLength      (0),
    itsStartTime       (0),
    itsTimeLength      (0),
    itsModelType       (0),
    itsCalcUVW         (0),
    itsUseAutoCorr     (0),
    itsCleanUp         (0),
    itsUpdateParms     (0),
    itsSolutionID      (0),
    itsPODHWO          (0)
{
  LOG_TRACE_FLOW("DH_WOPrediff copy constructor");
  setExtraBlob("Extra", 1);
}

DH_WOPrediff::~DH_WOPrediff()
{
  LOG_TRACE_FLOW("DH_WOPrediff destructor");
  delete itsPODHWO;
}

DataHolder* DH_WOPrediff::clone() const
{
  return new DH_WOPrediff(*this);
}

void DH_WOPrediff::initPO (const string& tableName)
{                         
  itsPODHWO = new PO_DH_WOPrediff(*this);
  itsPODHWO->tableName (tableName);
  setPOInitialized();
}

PL::PersistentObject& DH_WOPrediff::getPO() const
{
  return *itsPODHWO;
} 

void DH_WOPrediff::init()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("SCID", BlobField<int>(1));
  addField ("Status", BlobField<unsigned int>(1));
  addField ("KSType", BlobField<char>(1, MaxKSTypeLength));
  addField ("DoNothing", BlobField<unsigned int>(1));
  addField ("NewBaselines", BlobField<unsigned int>(1));
  addField ("NewDomain", BlobField<unsigned int>(1));
  addField ("NewPeelSources", BlobField<unsigned int>(1));
  addField ("SubtractSources", BlobField<unsigned int>(1));
  addField ("WritePredData", BlobField<unsigned int>(1));
  addField ("WriteInDataCol", BlobField<unsigned int>(1));
  addField ("StartFreq", BlobField<double>(1));
  addField ("FreqLength", BlobField<double>(1));
  addField ("StartTime", BlobField<double>(1));
  addField ("TimeLength", BlobField<double>(1));
  addField ("ModelType", BlobField<char>(1, MaxModelTypeLength));
  addField ("CalcUVW", BlobField<unsigned int>(1));
  addField ("UseAutoCorr", BlobField<unsigned int>(1));
  addField ("CleanUp", BlobField<unsigned int>(1));
  addField ("UpdateParms", BlobField<unsigned int>(1));
  addField ("SolutionID", BlobField<int>(1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffers.
  for (unsigned int k=0; k<MaxKSTypeLength; k++)
  {
    itsKSType[k] = 0;
  }

  for (unsigned int m=0; m<MaxModelTypeLength; m++)
  {
    itsModelType[m] = 0;
  }

  *itsWOID = 0;
  *itsSCID = -1;
  *itsStatus = DH_WOPrediff::New;
  *itsDoNothing = 0;
  *itsNewBaselines = 0;
  *itsNewDomain = 0;
  *itsNewPeelSources = 0;
  *itsSubtractSources = 0;
  *itsWritePredData = 0;
  *itsWriteInDataCol = 0;
  *itsStartFreq = 0;
  *itsFreqLength = 0;
  *itsStartTime = 0;
  *itsTimeLength = 0;
  *itsCalcUVW = 0;
  *itsUseAutoCorr = 0;
  *itsCleanUp = 0;
  *itsUpdateParms = 0;
  *itsSolutionID = -1;
}

void DH_WOPrediff::fillDataPointers()
{
  // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsSCID = getData<int> ("SCID");
  itsStatus = getData<unsigned int> ("Status");
  itsKSType = getData<char> ("KSType");
  itsDoNothing = getData<unsigned int> ("DoNothing");
  itsNewBaselines = getData<unsigned int> ("NewBaselines");
  itsNewDomain = getData<unsigned int> ("NewDomain");
  itsNewPeelSources = getData<unsigned int> ("NewPeelSources");
  itsSubtractSources = getData<unsigned int> ("SubtractSources");
  itsWritePredData = getData<unsigned int> ("WritePredData");
  itsWriteInDataCol = getData<unsigned int> ("WriteInDataCol");
  itsStartFreq = getData<double> ("StartFreq");
  itsFreqLength = getData<double> ("FreqLength");
  itsStartTime = getData<double> ("StartTime");
  itsTimeLength = getData<double> ("TimeLength");
  itsModelType = getData<char> ("ModelType");
  itsCalcUVW = getData<unsigned int> ("CalcUVW");
  itsUseAutoCorr = getData<unsigned int> ("UseAutoCorr");
  itsCleanUp = getData<unsigned int> ("CleanUp");
  itsUpdateParms = getData<unsigned int> ("UpdateParms");
  itsSolutionID = getData<int> ("SolutionID");
}

void DH_WOPrediff::setKSType(const string& ksType)
{
  ASSERTSTR(ksType.size() < MaxKSTypeLength, "KS type name is too long");
  char* ptr;
  ptr = itsKSType;
  strcpy(ptr, ksType.c_str());
}

string DH_WOPrediff::getModelType() const
{
  return itsModelType;
}

void DH_WOPrediff::setModelType(const string& type)
{
  ASSERTSTR(type.size() < MaxModelTypeLength, "Model type name is too long");
  char* ptr;
  ptr = itsModelType;
  strcpy(ptr, type.c_str());
}

void DH_WOPrediff::setVarData(const ParameterSet& predArgs,
			      vector<int>& antNrs,
			      vector<string>& pNames,
			      vector<string>& exPNames,
			      vector<int>& peelSrcs,
			      vector<int>& corrs)
{
  BlobOStream& bos = createExtraBlob();
  // Put prediffer arguments into extra blob
  string buffer;
  predArgs.writeBuffer(buffer);
  bos << buffer;

  // Put parameter names into extra blob
  bos.putStart("antNrs", 1);
  vector<int>::const_iterator iter;
  int nAnt = antNrs.size();
  bos << nAnt;
  for (iter = antNrs.begin(); iter != antNrs.end(); iter++)
  {
    bos << *iter;
  }
  bos.putEnd();

  // Put parameter names into extra blob
  bos.putStart("names", 1);
  vector<string>::const_iterator sIter;
  int nParam = pNames.size();
  bos << nParam;
  for (sIter = pNames.begin(); sIter != pNames.end(); sIter++)
  {
    bos << *sIter;
  }
  bos.putEnd();

  // Put exclude paramater names into extra blob
  bos.putStart("exNames", 1);
  vector<string>::const_iterator xIter;
  int nExParam = exPNames.size();
  bos << nExParam;
  for (xIter = exPNames.begin(); xIter != exPNames.end(); xIter++)
  {
    bos << *xIter;
  }
  bos.putEnd();

  // Put start solutions into extra blob
  bos.putStart("peelSrcs", 1);
  unsigned int nPeelSrcs = peelSrcs.size();
  bos << nPeelSrcs;
  for (iter = peelSrcs.begin(); iter != peelSrcs.end(); iter++)
  {
    bos << *iter;
  }
  bos.putEnd();

  // Put correlations into extra blob
  bos.putStart("correlations", 1);
  unsigned int nCorrs = corrs.size();
  bos << nCorrs;
  for (iter = corrs.begin(); iter != corrs.end(); iter++)
  {
    bos << *iter;
  }
  bos.putEnd();
}

bool DH_WOPrediff::getVarData(ParameterSet& predArgs,
			      vector<int>& antNrs,
			      vector<string>& pNames,
			      vector<string>& exPNames,
			      vector<int>& peelSrcs,
			      vector<int>& corrs)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
  else
  {
    // Get prediffer arguments
    string buffer;
    bis >> buffer;
    predArgs.clear();
    predArgs.adoptBuffer(buffer);

    // Get antenna numbers.
    bis.getStart("antNrs");
    antNrs.clear();
    int nr;
    bis >> nr;
    antNrs.resize(nr);
    for (int i=0; i < nr; i++)
    {
      bis >> antNrs[i];
    }
    bis.getEnd();
    
    // Get parameter names.
    bis.getStart("names");
    pNames.clear();
    int nmbr;
    bis >> nmbr;
    pNames.resize(nmbr);
    for (int i=0; i < nmbr; i++)
    {
      bis >> pNames[i];
    }
    bis.getEnd();

    // Get exclude parameter names.
    bis.getStart("exNames");
    exPNames.clear();
    int nrExP;
    bis >> nrExP;
    exPNames.resize(nrExP);
    for (int k=0; k < nrExP; k++)
    {
      bis >> exPNames[k];
    }
    bis.getEnd();

    // Get source numbers
    bis.getStart("peelSrcs");
    peelSrcs.clear();
    int number;
    bis >> number;
    peelSrcs.resize(number);
    for (int j=0; j < number; j++)
    {
      bis >> peelSrcs[j];
    }
    bis.getEnd();

    bis.getStart("correlations");
    corrs.clear();
    int nrCorr;
    bis >> nrCorr;
    corrs.resize(nrCorr);
    for (int l=0; l < nrCorr; l++)
    {
      bis >> corrs[l];
    }
    bis.getEnd();

    return true;
  }  

}

void DH_WOPrediff::dump()
{
  cout << "DH_WOPrediff: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Controller ID = " << getStrategyControllerID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "Do nothing? = " << getDoNothing() << endl;
  cout << "New baselines? = " << getNewBaselines() << endl;
  cout << "New domain? = " << getNewDomain() << endl;
  cout << "New peel sources? = " << getNewPeelSources() << endl;
  cout << "Subtract peel sources? = " << getSubtractSources() << endl;
  cout << "Write predicted data? = " << getWritePredData() << endl;
  cout << "Write in DATA column? = " << getWriteInDataCol() << endl;
  cout << "Start frequency = " << getStartFreq() << endl;
  cout << "Frequency length = " << getFreqLength() << endl;
  cout << "Start time = " << getStartTime() << endl;
  cout << "Time length = " << getTimeLength() << endl;
  cout << "Model type = " << getModelType() << endl;
  cout << "Calc UVW = " << getCalcUVW() << endl;
  cout << "Use auto correlations = " << getUseAutoCorrelations() << endl;
  cout << "Clean up = " << getCleanUp() << endl;
  cout << "Update parameters = " << getUpdateParms() << endl;
  cout << "Solution id = " << getSolutionID() << endl;

  ParameterSet sArguments;
  vector<int> antNrs;
  vector<string> pNames;
  vector<string> exPNames;
  vector<int> srcs;
  vector<int> corrs;
  if (getVarData(sArguments, antNrs, pNames, exPNames, srcs, corrs))
  { 
    cout << "MS name = " << sArguments.getString ("MSName")
	 << endl;
    cout << "Database host = " << sArguments.getString ("DBHost")
	 << endl;
    cout << "Database type = " << sArguments.getString ("DBType")
	 << endl;
    cout << "Database name = " << sArguments.getString ("DBName")
	 << endl;
    cout << "Database password = " << sArguments.getString ("DBPwd")
	 << endl;
    cout << "Meq table name = " << sArguments.getString ("meqTableName")
	 << endl;
    cout << "Sky table name = " << sArguments.getString ("skyTableName")
	 << endl;
    cout << "Antenna numbers : [ " ;
    for (unsigned int i = 0; i < antNrs.size(); i++)
    {
      cout << antNrs[i] << ", ";
    }
    cout << " ]" << endl;
    cout << "Number of parameters = "  << pNames.size() << endl;
    
    cout << "Parameter names : " << endl;
    for (unsigned int i = 0; i < pNames.size(); i++)
    {
      cout << pNames[i] << endl ;
    }

    cout << "Number of exclude parameters = "  << exPNames.size() << endl;
    
    cout << "Exclude parameter names : " << endl;
    for (unsigned int i = 0; i < exPNames.size(); i++)
    {
      cout << exPNames[i] << endl ;
    }

    cout << "Source numbers : " << endl;
    for (unsigned int i = 0; i < srcs.size(); i++)
    {
      cout << srcs[i] << endl ;
    }

    cout << "Correlations : " << endl;
    for (unsigned int i = 0; i < corrs.size(); i++)
    {
      cout << corrs[i] << endl ;
    }

  }

}

void DH_WOPrediff::clearData()
{
  clearExtraBlob();
  setWorkOrderID(-1);
  setStrategyControllerID(-1);
  setStatus(DH_WOPrediff::New);
  setKSType("");
  setDoNothing(false);
  setNewBaselines(true);
  setNewDomain(true);
  setNewPeelSources(true);
  setSubtractSources(false);
  setWritePredData(false);
  setWriteInDataCol(false);
  setStartFreq(0);
  setFreqLength(0);
  setStartTime(0);
  setTimeLength(0);
  setModelType("");
  setCalcUVW(false);
  setUseAutoCorrelations(true);
  setCleanUp(false);
  setUpdateParms(false);
  setSolutionID(0);
}

namespace PL {

void DBRep<DH_WOPrediff>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["SCID"] == itsSCID;
  cols["STATUS"] == itsStatus;
  cols["KSTYPE"] == itsKSType;
  cols["DONOTHING"] == itsDoNothing;
  cols["NEWBASELINES"] == itsNewBaselines;
  cols["NEWDOMAIN"] == itsNewDomain;
  cols["NEWSOURCES"] == itsNewPeelSources;
  cols["SUBTRACTSOURCES"] == itsSubtractSources;
  cols["WRITEPREDDATA"] == itsWritePredData;
  cols["WRITEINDATACOL"] == itsWriteInDataCol;
  cols["STARTFREQ"] == itsStartFreq;
  cols["FREQLENGTH"] == itsFreqLength;
  cols["STARTTIME"] == itsStartTime;
  cols["TIMELENGTH"] == itsTimeLength;
  cols["CLEANUP"] == itsCleanUp;
  cols["UPDATEPARMS"] == itsUpdateParms;
  cols["SOLUTIONID"] == itsSolutionID;
}

void DBRep<DH_WOPrediff>::toDBRep (const DH_WOPrediff& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsSCID = obj.getStrategyControllerID();
  itsStatus = obj.getStatus();
  itsKSType = obj.getKSType();
  itsDoNothing = obj.getDoNothing();
  itsNewBaselines = obj.getNewBaselines();
  itsNewDomain = obj.getNewDomain();
  itsNewPeelSources = obj.getNewPeelSources();
  itsSubtractSources = obj.getSubtractSources();
  itsWritePredData = obj.getWritePredData();
  itsWriteInDataCol = obj.getWriteInDataCol();
  itsStartFreq = obj.getStartFreq();
  itsFreqLength = obj.getFreqLength();
  itsStartTime = obj.getStartTime();
  itsTimeLength = obj.getTimeLength();
  itsCleanUp = obj.getCleanUp();
  itsUpdateParms = obj.getUpdateParms();
  itsSolutionID = obj.getSolutionID();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_WOPrediff>;

}  // end namespace PL

} // namespace LOFAR
