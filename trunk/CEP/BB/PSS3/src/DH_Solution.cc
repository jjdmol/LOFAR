//  DH_Solution.cc:
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


#include <PSS3/DH_Solution.h>
#include <PL/TPersistentObject.h>
#include <Common/Debug.h>
#include <sstream>

namespace LOFAR
{

const unsigned int MaxNumberOfParam = 32;  // These need NOT be the same as in DH_WorkOrder
const unsigned int MaxParamNameLength = 16;

string dtos(double dbl) // Convert double to string
{
  char buf[10];
  sprintf(buf, "%.15g", dbl);
  return buf;
}

DH_Solution::DH_Solution (const string& name)
  : DH_PL           (name, "DH_Solution"),
    itsID           (0),
    itsWOID         (0),
    itsIteration    (0),
    itsFit          (0),
    itsMu           (0),
    itsStdDev       (0),
    itsChi          (0),
    itsNumberOfParam(0),
    itsParamValues  (0),
    itsParamNames   (0),
    itsPODHSOL      (0)
{}

DH_Solution::DH_Solution(const DH_Solution& that)
  : DH_PL   (that),
    itsID           (0),
    itsWOID         (0),
    itsIteration    (0),
    itsFit          (0),
    itsMu           (0),
    itsStdDev       (0),
    itsChi          (0),
    itsNumberOfParam(0),
    itsParamValues  (0),
    itsParamNames   (0),
    itsPODHSOL      (0)
{}

DH_Solution::~DH_Solution()
{
  delete itsPODHSOL;
}

DataHolder* DH_Solution::clone() const
{
  return new DH_Solution(*this);
}

void DH_Solution::initPO (const string& tableName)
{                         
  itsPODHSOL = new PO_DH_SOL(*this);
  itsPODHSOL->tableName (tableName);
}

PL::PersistentObject& DH_Solution::getPO() const
{
  return *itsPODHSOL;
} 

void DH_Solution::preprocess()
{
  // Add the fields to the data definition.
  addField ("ID", BlobField<int>(1));
  addField ("WOID", BlobField<int>(1));
  addField ("Iteration", BlobField<int>(1));
  addField ("Fit", BlobField<double>(1));
  addField ("Mu", BlobField<double>(1));
  addField ("StdDev", BlobField<double>(1));
  addField ("Chi", BlobField<double>(1));
  addField ("NumberOfParam", BlobField<unsigned int>(1));
  addField ("ParamValues", BlobField<double>(1, MaxNumberOfParam));
  addField ("ParamNames", BlobField<char>(1, MaxParamNameLength, 
                                          MaxNumberOfParam));
  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffers.
  for (unsigned int i=0; i<MaxNumberOfParam; i++) {
     itsParamValues[i] = 0;
     itsParamNames[i]= 0;
  }
  *itsID = -1;
  *itsWOID = -1;
  *itsIteration = -1;
  *itsFit = 0;
  *itsMu = 0;
  *itsStdDev =0;
  *itsChi = 0;
  *itsNumberOfParam = 0;
  // By default use the normal data size as current;
  // only if the user explicitly set another CurDataSize
  // we will send that length
  setCurDataSize(getDataSize());
}

void DH_Solution::fillDataPointers()
{
 // Fill in the pointers.
  itsID = getData<int> ("ID");
  itsWOID = getData<int> ("WOID");
  itsIteration = getData<int> ("Iteration");
  itsFit = getData<double> ("Fit");
  itsMu = getData<double> ("Mu");
  itsStdDev = getData<double> ("StdDev");
  itsChi = getData<double> ("Chi");
  itsNumberOfParam = getData<unsigned int> ("NumberOfParam");
  itsParamValues = getData<double> ("ParamValues");
  itsParamNames  = getData<char> ("ParamNames");
}

void DH_Solution::postprocess()
{
  itsID = 0;
  itsWOID = 0;
  itsIteration = 0;
  itsFit = 0;
  itsMu = 0;
  itsStdDev = 0;
  itsChi = 0;
  itsNumberOfParam = 0;
  itsParamValues = 0;
  itsParamNames = 0;
}

Quality DH_Solution::getQuality() const
{
  Quality qual;
  qual.itsFit = *itsFit;
  qual.itsMu = *itsMu;
  qual.itsStddev = *itsStdDev;
  qual.itsChi = *itsChi;
  return qual;
}

void DH_Solution::setQuality(const Quality& quality)
{
  *itsFit = quality.itsFit;
  *itsMu = quality.itsMu;
  *itsStdDev = quality.itsStddev;
  *itsChi = quality.itsChi;
}

void DH_Solution::getParamValues(vector<double>& values) const
{
  values.clear();
  values.resize(getNumberOfParam());
  for (unsigned int i = 0; i < getNumberOfParam(); i++)
  { 
      values[i] = itsParamValues[i];
  }
}

void DH_Solution::setParamValues(const vector<double>& values)
{
  AssertStr(values.size() <= MaxNumberOfParam, 
            "The number of solution parameter values " << values.size() 
            << " is larger than the maximum " << MaxNumberOfParam);
  vector<double>::const_iterator iter;
  int paramNo = 0;

  for (iter = values.begin(); iter != values.end(); iter++)
  {
    itsParamValues[paramNo] = *iter;
    paramNo++;
  }
  setNumberOfParam(paramNo);
}

void DH_Solution::getParamNames(vector<string>& names) const
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

void DH_Solution::setParamNames(const vector<string>& names)
{
  AssertStr(names.size() <= MaxNumberOfParam, 
            "The number of solution parameter names " << names.size() 
            << " is larger than the maximum " << MaxNumberOfParam);

  vector<string>::const_iterator iter;
  int numberOfParam = 0;
  char* ptr;
  ptr = itsParamNames;
  for (iter = names.begin(); iter != names.end(); iter++)
  {
    AssertStr(iter->length() < MaxParamNameLength, 
              "Parameter name " << *iter << " is longer than maximum "
	      << MaxParamNameLength <<".");
    strcpy(ptr, iter->c_str());
    ptr += MaxParamNameLength;    
    numberOfParam++;
  }
  setNumberOfParam(numberOfParam);
}

void DH_Solution::clearData()
{
  setID(-1);
  setWorkOrderID(-1);
  setIterationNo(-1);
  Quality q;
  setQuality(q);
  setNumberOfParam(0);
  for (unsigned int i=0; i<MaxNumberOfParam; i++) {
     itsParamValues[i] = 0;
     itsParamNames[i]=0;
  }
}

void DH_Solution::dump()
{
//   cout <<  "DH_Solution: " << endl;
//   cout <<  "ID =  " << getID() << endl;
//   cout <<  "workorder ID =  " << getWorkOrderID() << endl;
//   cout <<  "iteration  =  " << getIterationNo() << endl;
//   cout <<  "number of parameters =  " << getNumberOfParam() << endl;
//   vector<string> pNames;
//   getParamNames(pNames);
//   vector<double> pValues;
//   getParamValues(pValues);
//   DbgAssertStr(pNames.size() == pValues.size(), 
// 	            "The number of parameters and their values do not match ");
//   cout <<  "PARAMETERS" << endl;

//   char strVal [20];
//   for (unsigned int i = 0; i < pNames.size(); i++)
//   {
//     cout <<  pNames[i] <<  " = " ;
//     sprintf(strVal, "%1.10f ", pValues[i]);
//     cout << strVal << endl;
//   } 
//   cout << endl;

  cout << getIterationNo() << " " << endl;
  vector<string> pNames;
  getParamNames(pNames);
  vector<double> pValues;
  getParamValues(pValues);
  DbgAssertStr(pNames.size() == pValues.size(), 
	            "The number of parameters and their values do not match ");

  char strVal [20];
  for (unsigned int i = 0; i < pNames.size(); i++)
  {
    cout <<  pNames[i] <<  " " ;
  } 
  cout << endl;
  for (unsigned int i = 0; i < pNames.size(); i++)
  {
    sprintf(strVal, "%1.10f ", pValues[i]);
    cout << strVal << " ";
  }
  cout << endl;


}

namespace PL {

void DBRep<DH_Solution>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["BBID"] == itsID;
  cols["WOID"] == itsWOID;
  cols["ITERATION"] == itsIteration;
  cols["FIT"] == itsFit;
  cols["MU"] == itsMu;
  cols["STDDEV"] == itsStdDev;
  cols["CHI"] == itsChi;
  cols["NUMBEROFPARAM"] == itsNumberOfParam;
}

void DBRep<DH_Solution>::toDBRep (const DH_Solution& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsID = obj.getID();
  itsWOID = obj.getWorkOrderID();
  itsIteration = obj.getIterationNo();
  itsFit = obj.getQuality().itsFit;
  itsMu = obj.getQuality().itsMu;
  itsStdDev = obj.getQuality().itsStddev;
  itsChi = obj.getQuality().itsChi;
  itsNumberOfParam = obj.getNumberOfParam();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_Solution>;
template class DBRep<DH_Solution>;

}  // end namespace PL

} // namespace LOFAR
