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
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <TransportPostgres/TH_DB.h>
#include <sstream>
#include <unistd.h> 

namespace LOFAR
{
const unsigned int MaxKSTypeLength = 8;

DH_WOSolve::DH_WOSolve (const string& name)
  : DH_DB(name, "DH_WOSolve", 1),
    itsWOID            (0),
    itsSCID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsIteration       (0),
    itsDoNothing       (0),
    itsNewDomain       (0),
    itsMaxIterations   (0),
    itsFitCriterion    (0),
    itsUseSVD          (0),
    itsCleanUp         (0)
{
  LOG_TRACE_FLOW("DH_WOSolve constructor");
  setExtraBlob("Extra", 1);
}

DH_WOSolve::DH_WOSolve(const DH_WOSolve& that)
  : DH_DB(that),
    itsWOID            (0),
    itsSCID            (0),
    itsStatus          (0),
    itsKSType          (0),
    itsIteration       (0),
    itsDoNothing       (0),
    itsNewDomain       (0),
    itsMaxIterations   (0),
    itsFitCriterion    (0),
    itsUseSVD          (0),
    itsCleanUp         (0)
{
  LOG_TRACE_FLOW("DH_WOSolve copy constructor");
  setExtraBlob("Extra", 1);
}

DH_WOSolve::~DH_WOSolve()
{
  LOG_TRACE_FLOW("DH_WOSolve destructor");
}

DataHolder* DH_WOSolve::clone() const
{
  return new DH_WOSolve(*this);
}

void DH_WOSolve::init()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("SCID", BlobField<int>(1));
  addField ("Status", BlobField<unsigned int>(1));
  addField ("KSType", BlobField<char>(1, MaxKSTypeLength));
  addField ("Iteration", BlobField<int>(1));
  addField ("DoNothing", BlobField<unsigned int>(1));
  addField ("NewDomain", BlobField<unsigned int>(1));
  addField ("MaxIterations", BlobField<int>(1));
  addField ("FitCriterion", BlobField<double>(1));
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
  *itsIteration = -1;
  *itsDoNothing = 0;
  *itsNewDomain = 0;
  *itsMaxIterations = 1;
  *itsFitCriterion = -1;
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
  itsIteration = getData<int> ("Iteration");
  itsDoNothing = getData<unsigned int> ("DoNothing");  
  itsNewDomain = getData<unsigned int> ("NewDomain");
  itsMaxIterations = getData<int> ("MaxIterations");
  itsFitCriterion = getData<double> ("FitCriterion");
  itsUseSVD = getData<unsigned int> ("UseSVD");
  itsCleanUp = getData<unsigned int> ("CleanUp");
}

void DH_WOSolve::setKSType(const string& ksType)
{
  ASSERTSTR(ksType.size() < MaxKSTypeLength, "KS type name is too long");
  char* ptr;
  ptr = itsKSType;
  strcpy(ptr, ksType.c_str());
}

void DH_WOSolve::dump() const
{
  cout << "DH_WOSolve: " << endl;
  cout << "ID = " << getWorkOrderID() << endl;
  cout << "Controller ID = " << getStrategyControllerID() << endl;
  cout << "Status = " << getStatus() << endl;
  cout << "KS Type = " << getKSType() << endl;
  cout << "Iteration number = " << getIteration() << endl;
  cout << "Do nothing? = " << getDoNothing() << endl;
  cout << "NewDomain? = " << getNewDomain() << endl;
  cout << "Number of iterations = " << getMaxIterations() << endl;
  cout << "Fit criterion = " << getFitCriterion() << endl;
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
  setIteration(-1);
  setDoNothing(false);
  setNewDomain(true);
  setMaxIterations(1);
  setFitCriterion(-1);
  setUseSVD(false);
  setCleanUp(false);
}

string DH_WOSolve::createInsertStatement(TH_DB* th)
{
   ostringstream q;
   q << "INSERT INTO bbs3wosolver (data, woid, scid, status, kstype, iteration,"
     << " donothing, newdomain, maxiterations, fitcriterion, usesvd, cleanup) VALUES ('";
   th->addDBBlob(this, q);
   q << "', "
     << getWorkOrderID() << ", "
     << getStrategyControllerID() << ", "
     << getStatus() << ", '"
     << getKSType() << "', "
     << getIteration() << ", "
     << getDoNothing() << ", "
     << getNewDomain() << ", "
     << getMaxIterations() << ", "
     << getFitCriterion() << ", "
     << getUseSVD() << ", "
     << getCleanUp() << ");";
   return q.str();
}

string DH_WOSolve::createUpdateStatement(TH_DB* th)
{
  // NB:This implementation assumes only the status has changed. So only the blob and
  // the status field are updated!
  ostringstream q;
  q << "UPDATE bbs3wosolver SET data='";
  th->addDBBlob(this, q);
  q << "', status=" << getStatus()
    <<" WHERE woid=" << getWorkOrderID();
  return q.str();
}


} // namespace LOFAR
