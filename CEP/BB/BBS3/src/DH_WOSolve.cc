//  DH_WOSolve.cc:
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

#include <BBS3/DH_WOSolve.h>
#include <PL/TPersistentObject.h>
#include <Common/KeyValueMap.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{

const unsigned int MaxKSTypeLength = 8;
const unsigned int MaxNoStartSols = 16;
const unsigned int MaxParamNameLength = 16;
const unsigned int MaxNumberOfParam = 32;

DH_WOSolve::DH_WOSolve (const string& name)
  : DH_PL(name, "DH_WOSolve", 1),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsInitialize      (0),
    itsNextInterval    (0),
    itsUseSVD          (0),
    itsPODHWO          (0)
{
  LOG_TRACE_FLOW("DH_WOSolve constructor");
  setExtraBlob("Extra", 1);
}

DH_WOSolve::DH_WOSolve(const DH_WOSolve& that)
  : DH_PL(that),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsInitialize      (0),
    itsNextInterval    (0),
    itsUseSVD          (0),
    itsPODHWO          (0)
{
  LOG_TRACE_FLOW("DH_WOSolve copy constructor");
  setExtraBlob("Extra", 1);
}

DH_WOSolve::~DH_WOSolve()
{
  LOG_TRACE_FLOW("DH_WOSolve destructor");
  delete itsPODHWO;
}

DataHolder* DH_WOSolve::clone() const
{
  return new DH_WOSolve(*this);
}

void DH_WOSolve::initPO (const string& tableName)
{                         
  itsPODHWO = new PO_DH_WOSOLVE(*this);
  itsPODHWO->tableName (tableName);
}

PL::PersistentObject& DH_WOSolve::getPO() const
{
  return *itsPODHWO;
} 

void DH_WOSolve::preprocess()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("Status", BlobField<unsigned int>(1));
  addField ("KSType", BlobField<char>(1, MaxKSTypeLength));
  addField ("Initialize", BlobField<unsigned int>(1));
  addField ("NextInterval", BlobField<unsigned int>(1));
  addField ("UseSVD", BlobField<unsigned int>(1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffers.
  for (unsigned int k=0; k<MaxKSTypeLength; k++)
  {
    itsKSType[k] = 0;
  }

  *itsWOID = -1;
  *itsStatus = DH_WOSolve::New;
  *itsInitialize = 0;
  *itsNextInterval = 0;
  *itsUseSVD = 0;
}

void DH_WOSolve::fillDataPointers()
{
  // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsStatus = getData<unsigned int> ("Status");
  itsKSType = getData<char> ("KSType");
  itsInitialize = getData<unsigned int> ("Initialize");
  itsNextInterval = getData<unsigned int> ("NextInterval");
  itsUseSVD = getData<unsigned int> ("UseSVD");
}

void DH_WOSolve::postprocess()
{
  itsWOID = 0;
  itsStatus = 0;
  itsKSType = 0;
  itsInitialize = 0;
  itsNextInterval = 0;
  itsUseSVD = 0;
}

void DH_WOSolve::setKSType(const string& ksType)
{
  ASSERTSTR(ksType.size() < MaxKSTypeLength, "KS type name is too long");
  char* ptr;
  ptr = itsKSType;
  strcpy(ptr, ksType.c_str());
}

void DH_WOSolve::setVarData(const KeyValueMap& msArgs,
			    int timeInterval,
			    vector<string>& pNames)
{
  BlobOStream& bos = createExtraBlob();
  // Put strategy arguments into extra blob
  bos << msArgs;

  bos << timeInterval;

  // Put parameter names into extra blob
  bos.putStart("names", 1);
  vector<string>::const_iterator iter;
  int paramNo = pNames.size();
  bos << paramNo;             // First write the number of parameters to follow
  for (iter = pNames.begin(); iter != pNames.end(); iter++)
  {
    bos << *iter;
  }

  bos.putEnd();
}

bool DH_WOSolve::getVarData(KeyValueMap& msArgs,
			    int& timeInterval,
			    vector<string>& pNames)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
  else
  {
    // Get ms arguments
    bis >> msArgs;

    bis >> timeInterval;

    // Get parameter names.
    bis.getStart("names");
    pNames.clear();
    unsigned int nr;
    bis >> nr;
    pNames.resize(nr);
    for (unsigned int i=0; i < nr; i++)
    {
      bis >> pNames[i];
    }
    bis.getEnd();

    return true;
  }  

}

void DH_WOSolve::dump()
{
  cout << "DH_WOSolve: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "Initialize? = " << getInitialize() << endl;
  cout << "NextInterval? = " << getNextInterval() << endl;

  KeyValueMap sArguments;
  int timeInterval;
  vector<string> pNames;
  if (getVarData(sArguments, timeInterval, pNames))
  { 
    cout << "Time interval = " << timeInterval << endl;
    cout << "Number of parameters = "  << pNames.size() << endl;
    
    cout << "Parameter names : " << endl;
    for (unsigned int i = 0; i < pNames.size(); i++)
    {
      cout << pNames[i] << endl ;
    }
  }
}

void DH_WOSolve::clearData()
{
  clearExtraBlob();
  setWorkOrderID(-1);
  setStatus(DH_WOSolve::New);
  setKSType("");
  setInitialize(true);
  setNextInterval(true);
  setUseSVD(false);
}

namespace PL {

void DBRep<DH_WOSolve>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["STATUS"] == itsStatus;
  cols["KSTYPE"] == itsKSType;
  cols["INITIALIZE"] == itsInitialize;
  cols["NEXTINTERVAL"] == itsNextInterval;
  cols["USESVD"] == itsUseSVD;
}

void DBRep<DH_WOSolve>::toDBRep (const DH_WOSolve& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsStatus = obj.getStatus();
  itsKSType = obj.getKSType();
  itsInitialize = obj.getInitialize();
  itsNextInterval = obj.getNextInterval();
  itsUseSVD = obj.getUseSVD();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_WOSolve>;

}  // end namespace PL

} // namespace LOFAR
