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

#include <lofar_config.h>

#include <BBS3/DH_Solution.h>
#include <PL/TPersistentObject.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/LofarLogger.h>
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
  : DH_PL           (name, "DH_Solution", 1),
    itsID           (0),
    itsWOID         (0),
    itsIteration    (0),
    itsFit          (0),
    itsMu           (0),
    itsStdDev       (0),
    itsChi          (0),
    itsNumberOfParam(0),
    itsPODHSOL      (0)
{
  setExtraBlob("Extra", 1);
}

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
    itsPODHSOL      (0)
{
  setExtraBlob("Extra", 1);
}

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

  // Create the data blob (which calls fillPointers).
  createDataBlock();

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

bool DH_Solution::getSolution(vector<string>& names, vector<double>& values)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
 else
  {
    // Get parameter names
    names.clear();
    int size = getNumberOfParam();
    names.resize(size);
    for (int i=0; i<size; i++)
    { 
      bis >> names[i];
    }
    // Get parameter values.
    bis.getStart("values");
    values.clear();
    values.resize(size);
    for (int j=0; j < size; j++)
    {
      bis >> values[j];
    }
    bis.getEnd();
    return true;
  }  

}

void DH_Solution::setSolution(vector<string>& names, vector<double>& values)
{
  ASSERTSTR(names.size() == values.size(), 
	    "The number of parameter names and values are not equal.");
  BlobOStream& bos = createExtraBlob();
  // Put parameter names into extra blob
  vector<string>::const_iterator iter;
  for (iter = names.begin(); iter != names.end(); iter++)
  {
    bos << *iter;
  }
  setNumberOfParam(names.size());
  // Put parameter values into extra blob
  bos.putStart("values", 1);
  vector<double>::const_iterator valIter;
  for (valIter = values.begin(); valIter != values.end(); valIter++)
  {
    bos << *valIter;
  }
  bos.putEnd();
}

void DH_Solution::clearData()
{
  setID(-1);
  setWorkOrderID(-1);
  setIterationNo(-1);
  Quality q;
  setQuality(q);
  setNumberOfParam(0);
  clearExtraBlob();
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
//   DBGASSERTSTR(pNames.size() == pValues.size(), 
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


  vector<string> pNames;
  vector<double> pValues;
  getSolution(pNames, pValues);
  DBGASSERTSTR(pNames.size() == pValues.size(), 
	            "The number of parameters and their values do not match ");

  char strVal [20];
  for (unsigned int i = 0; i < pNames.size(); i++)
  {
    string::size_type pos;
    pos = pNames[i].find(".CP");
    if (pos!=string::npos)
    {
      string subStr;
      subStr = pNames[i].substr(pos+3, 4);
      cout << subStr << " ";
      break;
    } 
  }
  cout << getIterationNo() << " ";
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

}  // end namespace PL

} // namespace LOFAR
