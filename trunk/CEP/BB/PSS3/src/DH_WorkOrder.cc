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


#include <PSS3/DH_WorkOrder.h>
#include <PL/TPersistentObject.h>
#include <Common/Debug.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{

int DH_WorkOrder::theirWriteCount = 0;

const unsigned int MaxKSTypeLength = 8;
const unsigned int MaxNoStartSols = 16;
const unsigned int MaxArgsSize = 32;
const unsigned int MaxParamNameLength = 16;
const unsigned int MaxNumberOfParam = 32;

DH_WorkOrder::DH_WorkOrder (const string& name)
  : DH_PL(name, "DH_WorkOrder"),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsStrategyNo      (0),
    itsNoStartSols     (0),
    itsStartSolutions  (0),
    itsArgsSize        (0),
    itsStrategyArgs    (0),
    itsNumberOfParam   (0),
    itsParamNames      (0),
    itsPODHWO          (0)
{}

DH_WorkOrder::DH_WorkOrder(const DH_WorkOrder& that)
  : DH_PL(that),
    itsWOID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsStrategyNo      (0),
    itsNoStartSols     (0),
    itsStartSolutions  (0),
    itsArgsSize        (0),
    itsStrategyArgs    (0),
    itsNumberOfParam   (0),
    itsParamNames      (0),
    itsPODHWO          (0)
{}

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
  addField ("StartSolutions", BlobField<int>(1, MaxNoStartSols));
  addField ("ArgsSize", BlobField<unsigned int>(1));
  addField ("StrategyArgs", BlobField<char>(1, MaxArgsSize));
  addField ("NumberOfParam", BlobField<unsigned int> (1));
  addField ("ParamNames", BlobField<char>(1, MaxParamNameLength,
					  MaxNumberOfParam));
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffers.
  for (unsigned int k=0; k<MaxKSTypeLength; k++)
  {
    itsKSType[k] = 0;
  }
  for (unsigned int m=0; m<MaxNoStartSols; m++)
  {
    *itsStartSolutions = 0;
  }
  for (unsigned int i=0; i<MaxNumberOfParam; i++) 
  {
    itsParamNames[i]= 0;     
  }
  *itsWOID = -1;
  *itsStatus = DH_WorkOrder::New;
  *itsStrategyNo = 0;
  *itsNoStartSols = 0;
  *itsArgsSize = 0;
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
  itsStartSolutions = getData<int> ("StartSolutions");
  itsArgsSize = getData<unsigned int> ("ArgsSize");
  itsStrategyArgs = getData<char> ("StrategyArgs");
  itsNumberOfParam = getData<unsigned int> ("NumberOfParam");
  itsParamNames = getData<char> ("ParamNames");
}

void DH_WorkOrder::postprocess()
{
  itsWOID = 0;
  itsStatus = 0;
  itsKSType = 0;
  itsStrategyNo = 0;
  itsNoStartSols = 0;
  itsStartSolutions = 0;
  itsStrategyArgs = 0;
  itsNumberOfParam = 0;
  itsParamNames = 0;
}

void DH_WorkOrder::setKSType(const string& ksType)
{
  AssertStr(ksType.size() < MaxKSTypeLength, "KS type name is too long");
  char* ptr;
  ptr = itsKSType;
  strcpy(ptr, ksType.c_str());
}

void DH_WorkOrder::useSolutionNumbers(const vector<int>& ids)
{ 
  unsigned int noSols = ids.size();
  AssertStr(noSols <= MaxNoStartSols, "The number of start solutions "
	    << noSols << " is larger than the maximum "
	    << MaxNoStartSols );
  for (unsigned int i = 0; i<noSols; i++)
  {
    itsStartSolutions[i] = ids[i];
  }
  setNoStartSolutions(noSols);
}

void DH_WorkOrder::getSolutionNumbers(vector<int>& ids) const
{ 
  ids.clear();
  ids.resize(getNoStartSolutions());
  for (int i=0; i<getNoStartSolutions(); i++)
  {
    ids[i] = itsStartSolutions[i];
  }
}

void DH_WorkOrder::getParamNames(vector<string>& names) const
{
  names.clear();
  names.resize(getNumberOfParam());
  char* ptr = itsParamNames;
  for (unsigned int i = 0; i < getNumberOfParam(); i++)
  {
     names[i] = ptr;
     ptr += MaxParamNameLength;
  }
}

void DH_WorkOrder::setParamNames(vector<string>& names)
{
  AssertStr(names.size() <= MaxNumberOfParam, 
	    "The number of work order parameters " << names.size() 
	    << " is larger than the maximum " << MaxNumberOfParam);

  vector<string>::const_iterator iter;
  int paramNo = 0;

  char* ptr;
  ptr = itsParamNames;
  for (iter = names.begin(); iter != names.end(); iter++)
  {
    AssertStr(iter->length() < MaxParamNameLength, 
	      "Parameter name " << *iter << " is longer than maximum "
	      << MaxParamNameLength << ".");
    strcpy(ptr, iter->c_str());
    ptr += MaxParamNameLength;
    paramNo++;
  }
  setNumberOfParam(paramNo);
}

void DH_WorkOrder::dump()
{
  cout << "DH_WorkOrder: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "Strategy number = " << getStrategyNo() << endl;
  cout << "Number of start solutions = " << getNoStartSolutions() << endl;
  vector<int> sols;
  getSolutionNumbers(sols);
  cout << "Start solutions : " << endl;
  for (unsigned int i = 0; i < sols.size(); i++)
  {
    cout << sols[i] << endl;
  }
  cout << "Number of parameters = "  << getNumberOfParam() << endl;
  vector<string> pNames;
  getParamNames(pNames);
  cout << "Parameter names : " << endl;
  for (unsigned int i = 0; i < pNames.size(); i++)
  {
     cout <<  pNames[i] << endl ;
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
  cols["ARGSSIZE"] == itsArgsSize;
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
  itsArgsSize = obj.getArgsSize();
  itsNumberOfParam = obj.getNumberOfParam();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_WorkOrder>;
template class DBRep<DH_WorkOrder>;

}  // end namespace PL

} // namespace LOFAR
