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

DH_WOSolve::DH_WOSolve (const string& name)
  : DH_PL(name, "DH_WOSolve", 1),
    itsWOID            (0),
    itsSCID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsNewDomain       (0),
    itsUseSVD          (0),
    itsCleanUp         (0),
    itsPODHWO          (0)
{
  LOG_TRACE_FLOW("DH_WOSolve constructor");
  setExtraBlob("Extra", 1);
}

DH_WOSolve::DH_WOSolve(const DH_WOSolve& that)
  : DH_PL(that),
    itsWOID            (0),
    itsSCID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsNewDomain       (0),
    itsUseSVD          (0),
    itsCleanUp         (0),
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
  addField ("SCID", BlobField<int>(1));
  addField ("Status", BlobField<unsigned int>(1));
  addField ("KSType", BlobField<char>(1, MaxKSTypeLength));
  addField ("NewDomain", BlobField<unsigned int>(1));
  addField ("UseSVD", BlobField<unsigned int>(1));
  addField ("CleanUp", BlobField<unsigned int>(1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();
  // Initialize the buffers.
  for (unsigned int k=0; k<MaxKSTypeLength; k++)
  {
    itsKSType[k] = 0;
  }

  *itsWOID = 0;
  *itsSCID = -1;
  *itsStatus = DH_WOSolve::New;
  *itsNewDomain = 0;
  *itsUseSVD = 0;
  *itsCleanUp = 0;
}

void DH_WOSolve::fillDataPointers()
{
  // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsSCID = getData<int> ("SCID");
  itsStatus = getData<unsigned int> ("Status");
  itsKSType = getData<char> ("KSType");
  itsNewDomain = getData<unsigned int> ("NewDomain");
  itsUseSVD = getData<unsigned int> ("UseSVD");
  itsCleanUp = getData<unsigned int> ("CleanUp");
}

void DH_WOSolve::postprocess()
{
  itsWOID = 0;
  itsSCID = 0;
  itsStatus = 0;
  itsKSType = 0;
  itsNewDomain = 0;
  itsUseSVD = 0;
  itsCleanUp = 0;
}

void DH_WOSolve::setKSType(const string& ksType)
{
  ASSERTSTR(ksType.size() < MaxKSTypeLength, "KS type name is too long");
  char* ptr;
  ptr = itsKSType;
  strcpy(ptr, ksType.c_str());
}

void DH_WOSolve::dump()
{
  cout << "DH_WOSolve: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Controller ID = " << getStrategyControllerID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "NewDomain? = " << getNewDomain() << endl;
  cout << "UseSVD? = " << getUseSVD() << endl;
  cout << "Clean up = " << getCleanUp() << endl;
}

void DH_WOSolve::clearData()
{
  clearExtraBlob();
  setWorkOrderID(-1);
  setStrategyControllerID(-1);
  setStatus(DH_WOSolve::New);
  setKSType("");
  setNewDomain(true);
  setUseSVD(false);
  setCleanUp(false);
}

namespace PL {

void DBRep<DH_WOSolve>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["SCID"] == itsSCID;
  cols["STATUS"] == itsStatus;
  cols["KSTYPE"] == itsKSType;
  cols["NEWDOMAIN"] == itsNewDomain;
  cols["USESVD"] == itsUseSVD;
  cols["CLEANUP"] == itsCleanUp;
}

void DBRep<DH_WOSolve>::toDBRep (const DH_WOSolve& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsSCID = obj.getStrategyControllerID();
  itsStatus = obj.getStatus();
  itsKSType = obj.getKSType();
  itsNewDomain = obj.getNewDomain();
  itsUseSVD = obj.getUseSVD();
  itsCleanUp = obj.getCleanUp();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_WOSolve>;

}  // end namespace PL

} // namespace LOFAR
