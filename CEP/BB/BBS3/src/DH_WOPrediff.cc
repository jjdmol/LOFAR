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
#include <Common/KeyValueMap.h>
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
    itsStatus          (0),
    itsKSType          (0),
    itsInitialize      (0),
    itsNextInterval    (0),
    itsFirstChan       (0),
    itsLastChan        (0),
    itsTimeInterval    (0),
    itsDDID            (0),
    itsModelType       (0),
    itsCalcUVW         (0),
    itsLockMappedMem   (0),
    itsPODHWO          (0)
{
  LOG_TRACE_FLOW("DH_WOPrediff constructor");
  setExtraBlob("Extra", 1);
}

DH_WOPrediff::DH_WOPrediff(const DH_WOPrediff& that)
  : DH_PL(that),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsInitialize      (0),
    itsNextInterval    (0),
    itsFirstChan       (0),
    itsLastChan        (0),
    itsTimeInterval    (0),
    itsDDID            (0),
    itsModelType       (0),
    itsCalcUVW         (0),
    itsLockMappedMem   (0),
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
}

PL::PersistentObject& DH_WOPrediff::getPO() const
{
  return *itsPODHWO;
} 

void DH_WOPrediff::preprocess()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("Status", BlobField<unsigned int>(1));
  addField ("KSType", BlobField<char>(1, MaxKSTypeLength));
  addField ("Initialize", BlobField<unsigned int>(1));
  addField ("NextInterval", BlobField<unsigned int>(1));
  addField ("FirstChan", BlobField<int>(1));
  addField ("LastChan", BlobField<int>(1));
  addField ("TimeInterval", BlobField<int>(1));
  addField ("DDID", BlobField<int>(1));
  addField ("ModelType", BlobField<char>(1, MaxModelTypeLength));
  addField ("CalcUVW", BlobField<unsigned int>(1));
  addField ("LockMappedMem", BlobField<unsigned int>(1));

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

  *itsWOID = -1;
  *itsStatus = DH_WOPrediff::New;
  *itsInitialize = 0;
  *itsNextInterval = 0;
  *itsFirstChan = 0;
  *itsLastChan = 0;
  *itsTimeInterval = 0;
  *itsDDID = 0;
  *itsCalcUVW = 0;
  *itsLockMappedMem = 0;
}

void DH_WOPrediff::fillDataPointers()
{
  // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsStatus = getData<unsigned int> ("Status");
  itsKSType = getData<char> ("KSType");
  itsInitialize = getData<unsigned int> ("Initialize");
  itsNextInterval = getData<unsigned int> ("NextInterval");
  itsFirstChan = getData<int> ("FirstChan");
  itsLastChan = getData<int> ("LastChan");
  itsTimeInterval = getData<int> ("TimeInterval");
  itsDDID = getData<int> ("DDID");
  itsModelType = getData<char> ("ModelType");
  itsCalcUVW = getData<unsigned int> ("CalcUVW");
  itsLockMappedMem = getData<unsigned int> ("LockMappedMem");
}

void DH_WOPrediff::postprocess()
{
  itsWOID = 0;
  itsStatus = 0;
  itsKSType = 0;
  itsInitialize = 0;
  itsNextInterval = 0;
  itsFirstChan = 0;
  itsLastChan = 0;
  itsTimeInterval = 0;
  itsDDID = 0;
  itsModelType = 0;
  itsCalcUVW = 0;
  itsLockMappedMem = 0;
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

void DH_WOPrediff::setVarData(const KeyValueMap& predArgs,
		  vector<int>& antNrs,
		  vector<string>& pNames,
		  vector<int>& peelSrcs)
{
  BlobOStream& bos = createExtraBlob();
  // Put prediffer arguments into extra blob
  bos << predArgs;

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

  // Put start solutions into extra blob
  bos.putStart("peelSrcs", 1);
  unsigned int nPeelSrcs = peelSrcs.size();
  bos << nPeelSrcs;
  for (iter = peelSrcs.begin(); iter != peelSrcs.end(); iter++)
  {
    bos << *iter;
  }

  bos.putEnd();

}

bool DH_WOPrediff::getVarData(KeyValueMap& predArgs,
			      vector<int>& antNrs,
			      vector<string>& pNames,
			      vector<int>& peelSrcs)
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
    bis >> predArgs;

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

    // Get start solutions
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
    return true;
  }  

}

void DH_WOPrediff::dump()
{
  cout << "DH_WOPrediff: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "Initialize? = " << getInitialize() << endl;
  cout << "NextInterval? = " << getNextInterval() << endl;
  cout << "First channel = " << getFirstChannel() << endl;
  cout << "Last channel = " << getLastChannel() << endl;
  cout << "Time interval = " << getTimeInterval() << endl;
  cout << "DDID = " << getDDID() << endl;
  cout << "Model type = " << getModelType() << endl;
  cout << "Calc UVW = " << getCalcUVW() << endl;
  cout << "Lock mapped memory = " << getLockMappedMemory() << endl;

  KeyValueMap sArguments;
  vector<int> antNrs;
  vector<string> pNames;
  vector<int> srcs;
  if (getVarData(sArguments, antNrs, pNames, srcs))
  { 
    cout << "MS name = " << sArguments.getString ("MSName", "notfound") 
	 << endl;
    cout << "Database host = " << sArguments.getString ("DBHost", "notfound") 
	 << endl;
    cout << "Database type = " << sArguments.getString ("DBType", "notfound") 
	 << endl;
    cout << "Database name = " << sArguments.getString ("DBName", "notfound")
	 << endl;
    cout << "Database password = " << sArguments.getString ("DBPwd", "notfound")
	 << endl;
    cout << "Meq table name = " << sArguments.getString ("meqTableName", "notfound")
	 << endl;
    cout << "Sky table name = " << sArguments.getString ("skyTableName", "notfound")
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

    cout << "Source numbers : " << endl;
    for (unsigned int i = 0; i < srcs.size(); i++)
    {
      cout << srcs[i] << endl ;
    }
  }

}

void DH_WOPrediff::clearData()
{
  clearExtraBlob();
  setWorkOrderID(-1);
  setStatus(DH_WOPrediff::New);
  setKSType("");
  setInitialize(true);
  setNextInterval(true);
  setFirstChannel(-1);
  setLastChannel(-1);
  setTimeInterval(0);
  setDDID(0);
  setModelType("");
  setCalcUVW(false);
  setLockMappedMemory(false);
}

namespace PL {

void DBRep<DH_WOPrediff>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["STATUS"] == itsStatus;
  cols["KSTYPE"] == itsKSType;
  cols["INITIALIZE"] == itsInitialize;
  cols["NEXTINTERVAL"] == itsNextInterval;
  cols["FIRSTCHAN"] == itsFirstChan;
  cols["LASTCHAN"] == itsLastChan;
  cols["TIMEINTERVAL"] == itsTimeInterval;
}

void DBRep<DH_WOPrediff>::toDBRep (const DH_WOPrediff& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsStatus = obj.getStatus();
  itsKSType = obj.getKSType();
  itsInitialize = obj.getInitialize();
  itsNextInterval = obj.getNextInterval();
  itsFirstChan = obj.getFirstChannel();
  itsLastChan = obj.getLastChannel();
  itsTimeInterval = obj.getTimeInterval();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_WOPrediff>;

}  // end namespace PL

} // namespace LOFAR
