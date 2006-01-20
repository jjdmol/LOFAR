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
#include <APS/ParameterSet.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <TransportPostgres/TH_DB.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{

const unsigned int MaxKSTypeLength = 16;
const unsigned int MaxModelTypeLength = 16;

DH_WOPrediff::DH_WOPrediff (const string& name)
  : DH_DB(name, "DH_WOPrediff", 1),
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
    itsMaxIterations    (0),
    itsStartFreq       (0),
    itsFreqLength      (0),
    itsStartTime       (0),
    itsTimeLength      (0),
    itsModelType       (0),
    itsCalcUVW         (0),
    itsUseAutoCorr     (0),
    itsCleanUp         (0),
    itsUpdateParms     (0),
    itsSolutionID      (0)
{
  LOG_TRACE_FLOW("DH_WOPrediff constructor");
  setExtraBlob("Extra", 1);
}

DH_WOPrediff::DH_WOPrediff(const DH_WOPrediff& that)
  : DH_DB(that),
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
    itsMaxIterations    (0),
    itsStartFreq       (0),
    itsFreqLength      (0),
    itsStartTime       (0),
    itsTimeLength      (0),
    itsModelType       (0),
    itsCalcUVW         (0),
    itsUseAutoCorr     (0),
    itsCleanUp         (0),
    itsUpdateParms     (0),
    itsSolutionID      (0)
{
  LOG_TRACE_FLOW("DH_WOPrediff copy constructor");
  setExtraBlob("Extra", 1);
}

DH_WOPrediff::~DH_WOPrediff()
{
  LOG_TRACE_FLOW("DH_WOPrediff destructor");
}

DataHolder* DH_WOPrediff::clone() const
{
  return new DH_WOPrediff(*this);
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
  addField ("MaxIterations", BlobField<int>(1));
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
  *itsMaxIterations = 1;
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
  itsMaxIterations = getData<int> ("MaxIterations");
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

void DH_WOPrediff::dump() const
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
  cout << "Number of iterations = " << getMaxIterations() << endl;
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
  if (const_cast<DH_WOPrediff*>(this)->getVarData(sArguments, antNrs,
						  pNames, exPNames,
						  srcs, corrs))
  { 
    cout << "MS name = " << sArguments.getString ("MSName")
	 << endl;
    cout << "General MS path = " << sArguments.getString ("generalMSPath")
	 << endl;
    cout << "Subset MS path = " << sArguments.getString ("subsetMSPath")
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
  setMaxIterations(1);
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

string DH_WOPrediff::createInsertStatement(TH_DB* th)
{
   ostringstream q;
   q << "INSERT INTO bbs3woprediffer (data, woid, scid, status, kstype, donothing, newbaselines, newdomain, newsources, subtractsources, writepreddata, writeindatacol, maxiterations, startfreq, freqlength, starttime, timelength, cleanup, updateparms, solutionid) VALUES ('";
   th->addDBBlob(this, q);
   q << "', "
     << getWorkOrderID() << ", "
     << getStrategyControllerID() << ", "
     << getStatus() << ", '"
     << getKSType() << "', "
     << getDoNothing() << ", "
     << getNewBaselines() << ", "
     << getNewDomain() << ", "
     << getNewPeelSources() << ", "
     << getSubtractSources() << ", "
     << getWritePredData() << ", "
     << getWriteInDataCol() << ", "
     << getMaxIterations() << ", "
     << getStartFreq() << ", "
     << getFreqLength() << ", "
     << getStartTime() << ", "
     << getTimeLength() << ", "
     << getCleanUp() << ", "
     << getUpdateParms() << ", "
     << getSolutionID() << ");";
   return q.str();
}

string DH_WOPrediff::createUpdateStatement(TH_DB* th)
{
  // This implementation assumes only the status has changed. So only the blob and
  // the status field are updated!
  ostringstream q;
  q << "UPDATE bbs3woprediffer SET data='";
  th->addDBBlob(this, q);
  q << "', status=" << getStatus() 
    <<" WHERE woid=" << getWorkOrderID();
  return q.str();
}

int DH_WOPrediff::getMaxSCID(TH_DB* th)
{
  string query("SELECT MAX(SCID) FROM bbs3woprediffer");
  char res[10];
  if (th->queryDB(query, res, 10) <= 0)
  {
    return 0;
  }
  else
  {
    return atoi(res);
  }
}

int DH_WOPrediff::getMaxWOID(TH_DB* th)
{
  string query("SELECT MAX(WOID) FROM bbs3woprediffer");
  char res[10];
  if (th->queryDB(query, res, 10) <= 0)
  {
    return 0;
  }
  else
  {
    return atoi(res);
  }
}

} // namespace LOFAR
