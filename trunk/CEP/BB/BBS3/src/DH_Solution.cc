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
    itsWOID         (0),
    itsFit          (0),
    itsMu           (0),
    itsStdDev       (0),
    itsChi          (0),
    itsPODHSOL      (0)
{
  setExtraBlob("Extra", 1);
}

DH_Solution::DH_Solution(const DH_Solution& that)
  : DH_PL   (that),
    itsWOID         (0),
    itsFit          (0),
    itsMu           (0),
    itsStdDev       (0),
    itsChi          (0),
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
  addField ("WOID", BlobField<int>(1));
  addField ("Fit", BlobField<double>(1));
  addField ("Mu", BlobField<double>(1));
  addField ("StdDev", BlobField<double>(1));
  addField ("Chi", BlobField<double>(1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();

  *itsWOID = -1;
  *itsFit = 0;
  *itsMu = 0;
  *itsStdDev =0;
  *itsChi = 0;
}

void DH_Solution::fillDataPointers()
{
 // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsFit = getData<double> ("Fit");
  itsMu = getData<double> ("Mu");
  itsStdDev = getData<double> ("StdDev");
  itsChi = getData<double> ("Chi");
}

void DH_Solution::postprocess()
{
  itsWOID = 0;
  itsFit = 0;
  itsMu = 0;
  itsStdDev = 0;
  itsChi = 0;
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

bool DH_Solution::getSolution(vector<ParmData>& pData)
{
  bool found;
  int version;
  BlobIStream& bis = getExtraBlob(found, version);
  if (!found) {
    return false;
  }
  else
  {
    // Get parameter data
    pData.clear();
    int size;
    bis >> size;
    pData.resize(size);
    for (int i=0; i<size; i++)
    { 
      bis >> pData[i];
    }
    bis.getEnd();
    return true;
  }  

}

void DH_Solution::setSolution(const vector<ParmData>& pData)
{
  BlobOStream& bos = createExtraBlob();
  // Put vector length into extra blob
  vector<ParmData>::const_iterator iter;
  int nParms = pData.size();
  bos << nParms;
  for (iter = pData.begin(); iter != pData.end(); iter++)
  {
    bos << *iter;
  }
}

void DH_Solution::clearData()
{
  setWorkOrderID(-1);
  Quality q;
  setQuality(q);
  clearExtraBlob();
}

void DH_Solution::dump()
{
  vector<ParmData> pData;
  getSolution(pData);
  
  cout << "Parm data : " << endl;
  for (unsigned int i = 0; i < pData.size(); i++)
  {
    cout << pData[i] << endl;
  }
  cout << "Quality = " << getQuality() << endl;

}

namespace PL {

void DBRep<DH_Solution>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["FIT"] == itsFit;
  cols["MU"] == itsMu;
  cols["STDDEV"] == itsStdDev;
  cols["CHI"] == itsChi;
}

void DBRep<DH_Solution>::toDBRep (const DH_Solution& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsFit = obj.getQuality().itsFit;
  itsMu = obj.getQuality().itsMu;
  itsStdDev = obj.getQuality().itsStddev;
  itsChi = obj.getQuality().itsChi;
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_Solution>;

}  // end namespace PL

} // namespace LOFAR
