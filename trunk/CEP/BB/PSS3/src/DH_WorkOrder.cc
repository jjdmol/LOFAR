//  DH_WorkOrder.cc:
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

#include <PSS3/DH_WorkOrder.h>
#include <PL/TPersistentObject.h>
#include <Common/KeyValueMap.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/Debug.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{

int DH_WorkOrder::theirWriteCount = 0;

const unsigned int MaxKSTypeLength = 8;
const unsigned int MaxNoStartSols = 16;
const unsigned int MaxParamNameLength = 16;
const unsigned int MaxNumberOfParam = 32;

DH_WorkOrder::DH_WorkOrder (const string& name)
  : DH_PL(name, "DH_WorkOrder", 1),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsStrategyNo      (0),
    itsNoStartSols     (0),
    itsNumberOfParam   (0),
    itsPODHWO          (0)
{
  setExtraBlob("Extra", 1);
}

DH_WorkOrder::DH_WorkOrder(const DH_WorkOrder& that)
  : DH_PL(that),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsStrategyNo      (0),
    itsNoStartSols     (0),
    itsNumberOfParam   (0),
    itsPODHWO          (0)
{
 setExtraBlob("Extra", 1);
}

DH_WorkOrder::~DH_WorkOrder()
{
  TRACER4("DH_WorkOrder destructor");
  delete itsPODHWO;
}

DataHolder* DH_WorkOrder::clone() const
{
  return new DH_WorkOrder(*this);
}

void DH_WorkOrder::initPO (const string& tableName)
{                         
  itsPODHWO = new PO_DH_WO(*this);
  itsPODHWO->tableName (tableName);
}

PL::PersistentObject& DH_WorkOrder::getPO() const
{
  return *itsPODHWO;
} 

void DH_WorkOrder::preprocess()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("Status", BlobField<unsigned int>(1));
  addField ("KSType", BlobField<char>(1, MaxKSTypeLength));
  addField ("StrategyNo", BlobField<unsigned int>(1));
  addField ("NoStartSols", BlobField<int>(1));
  addField ("NumberOfParam", BlobField<unsigned int> (1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffers.
  for (unsigned int k=0; k<MaxKSTypeLength; k++)
  {
    itsKSType[k] = 0;
  }

  *itsWOID = -1;
  *itsStatus = DH_WorkOrder::New;
  *itsStrategyNo = 0;
  *itsNoStartSols = 0;
  *itsNumberOfParam = 0;

   // By default use the normal data size as current;
   // only if the user explicitly set another CurDataSize
   // we will send that length
   setCurDataSize(getDataSize());
  
}

void DH_WorkOrder::fillDataPointers()
{
  // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsStatus = getData<unsigned int> ("Status");
  itsKSType = getData<char> ("KSType");
  itsStrategyNo = getData<unsigned int> ("StrategyNo");
  itsNoStartSols = getData<int> ("NoStartSols");
  itsNumberOfParam = getData<unsigned int> ("NumberOfParam");
}

void DH_WorkOrder::postprocess()
{
  itsWOID = 0;
  itsStatus = 0;
  itsKSType = 0;
  itsStrategyNo = 0;
  itsNoStartSols = 0;
  itsNumberOfParam = 0;
}

void DH_WorkOrder::setKSType(const string& ksType)
{
  AssertStr(ksType.size() < MaxKSTypeLength, "KS type name is too long");
  char* ptr;
  ptr = itsKSType;
  strcpy(ptr, ksType.c_str());
}

void DH_WorkOrder::setVarData(const KeyValueMap& stratArgs,
			      vector<string>& pNames, 
			      vector<int>& startSols)
{
  BlobOStream& bos = createExtraBlob();
  // Put strategy arguments into extra blob
  bos << stratArgs;

  // Put parameter names into extra blob
  bos.putStart("names", 1);
  vector<string>::const_iterator iter;
  int paramNo = pNames.size();
  for (iter = pNames.begin(); iter != pNames.end(); iter++)
  {
    bos << *iter;
  }
  setNumberOfParam(paramNo);
  bos.putEnd();

  // Put start solutions into extra blob
  bos.putStart("sols", 1);
  unsigned int noSols = startSols.size();
  for (unsigned int i = 0; i<noSols; i++)
  {
    bos << startSols[i];
  }
  setNoStartSolutions(noSols);
  bos.putEnd();
}

bool DH_WorkOrder::getVarData(KeyValueMap& stratArgs,
			      vector<string>& pNames,
			      vector<int>& startSols)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
  else
  {
    // Get strategy arguments
    bis >> stratArgs;

    // Get parameter names.
    bis.getStart("names");
    pNames.clear();
    int nr = getNumberOfParam();
    pNames.resize(nr);
    for (int i=0; i < nr; i++)
    {
      bis >> pNames[i];
    }
    bis.getEnd();
    // Get start solutions
    bis.getStart("sols");
    startSols.clear();
    int number = getNoStartSolutions();
    startSols.resize(number);
    for (int j=0; j < number; j++)
    {
      bis >> startSols[j];
    }
    bis.getEnd();
    return true;
  }  

}

void DH_WorkOrder::dump()
{
  cout << "DH_WorkOrder: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "Strategy number = " << getStrategyNo() << endl;
  cout << "Number of start solutions = " << getNoStartSolutions() << endl;

  KeyValueMap sArguments;
  vector<string> pNames;
  vector<int> sols;
  if (getVarData(sArguments, pNames, sols))
  { 
    cout << "Start solutions : " << endl;
    for (unsigned int i = 0; i < sols.size(); i++)
    {
      cout << sols[i] << endl;
    }
    cout << "Number of parameters = "  << getNumberOfParam() << endl;
    
    cout << "Parameter names : " << endl;
    for (unsigned int i = 0; i < pNames.size(); i++)
    {
      cout << pNames[i] << endl ;
    }
  }
}

namespace PL {

void DBRep<DH_WorkOrder>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["STATUS"] == itsStatus;
  cols["KSTYPE"] == itsKSType;
  cols["STRATEGYNO"] == itsStrategyNo;
  cols["NOSTARTSOLS"] == itsNoStartSols;
  cols["NUMBEROFPARAM"] == itsNumberOfParam;
}

void DBRep<DH_WorkOrder>::toDBRep (const DH_WorkOrder& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsStatus = obj.getStatus();
  itsKSType = obj.getKSType();
  itsStrategyNo = obj.getStrategyNo();
  itsNoStartSols = obj.getNoStartSolutions();
  itsNumberOfParam = obj.getNumberOfParam();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_WorkOrder>;

}  // end namespace PL

} // namespace LOFAR
