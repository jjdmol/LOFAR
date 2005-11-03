//  DH_ParmSol.cc:
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

#include <BBS3/DH_ParmSol.h>
#include <PL/TPersistentObject.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/LofarLogger.h>
#include <sstream>

namespace LOFAR
{

const unsigned int MaxParmNameLength = 64;

DH_ParmSol::DH_ParmSol (const string& name)
  : DH_PL           (name, "DH_ParmSol", 1),
    itsPODHSOL      (0),
    itsWOID         (0),
    itsParmName     (0),
    itsIter         (0),
    itsFit          (0),
    itsRank         (0),
    itsStartFreq    (0),
    itsEndFreq      (0),
    itsStartTime    (0),
    itsEndTime      (0),
    itsCoeff0       (0),
    itsCoeff1       (0),
    itsCoeff2       (0),
    itsCoeff3       (0)    
{}

DH_ParmSol::DH_ParmSol(const DH_ParmSol& that)
  : DH_PL   (that),
    itsPODHSOL      (0),
    itsWOID         (0),
    itsParmName     (0),
    itsIter         (0),
    itsFit          (0),
    itsRank         (0),
    itsStartFreq    (0),
    itsEndFreq      (0),
    itsStartTime    (0),
    itsEndTime      (0),
    itsCoeff0       (0),
    itsCoeff1       (0),
    itsCoeff2       (0),
    itsCoeff3       (0)    
{}

DH_ParmSol::~DH_ParmSol()
{
  delete itsPODHSOL;
}

DataHolder* DH_ParmSol::clone() const
{
  return new DH_ParmSol(*this);
}

void DH_ParmSol::initPO (const string& tableName)
{                         
  itsPODHSOL = new PO_DH_SOL(*this);
  itsPODHSOL->tableName (tableName);
}

PL::PersistentObject& DH_ParmSol::getPO() const
{
  return *itsPODHSOL;
} 

void DH_ParmSol::init()
{
  // Add the fields to the data definition.
  addField ("WOID", BlobField<int>(1));
  addField ("ParmName", BlobField<char>(1, MaxParmNameLength));
  addField ("Iteration", BlobField<int>(1));
  addField ("Fit", BlobField<double>(1));
  addField ("Rank", BlobField<int>(1));
  addField ("StartFreq", BlobField<double>(1));
  addField ("EndFreq", BlobField<double>(1));
  addField ("StartTime", BlobField<double>(1));
  addField ("EndTime", BlobField<double>(1));
  addField ("Coeff0", BlobField<double>(1));
  addField ("Coeff1", BlobField<double>(1));
  addField ("Coeff2", BlobField<double>(1));
  addField ("Coeff3", BlobField<double>(1));

  // Create the data blob (which calls fillPointers).
  createDataBlock();

  for (unsigned int k=0; k<MaxParmNameLength; k++)
  {
    itsParmName[k] = 0;
  }

  *itsWOID = -1;
  *itsIter = -1;
  *itsFit = 0;
  *itsRank = 0;
  *itsStartFreq = 0;
  *itsEndFreq = 0;
  *itsStartTime = 0;
  *itsEndTime = 0;
  *itsCoeff0 = 0;
  *itsCoeff1 = 0;
  *itsCoeff2 = 0;
  *itsCoeff3 = 0; 
}

void DH_ParmSol::fillDataPointers()
{
  // Fill in the pointers.
  itsWOID = getData<int> ("WOID");
  itsParmName = getData<char>("ParmName");
  itsIter = getData<int>("Iteration");
  itsFit = getData<double> ("Fit");
  itsRank = getData<int> ("Rank");
  itsStartFreq = getData<double> ("StartFreq");
  itsEndFreq = getData<double> ("EndFreq");
  itsStartTime = getData<double> ("StartTime");
  itsEndTime = getData<double> ("EndTime");
  itsCoeff0 = getData<double> ("Coeff0");
  itsCoeff1 = getData<double> ("Coeff1");
  itsCoeff2 = getData<double> ("Coeff2");
  itsCoeff3 = getData<double> ("Coeff3");
}

void DH_ParmSol::setParmName(const string& parmName)
{ 
  ASSERTSTR(parmName.size() < MaxParmNameLength, "Parameter name is too long");
  char* ptr;
  ptr = itsParmName;
  strcpy(ptr, parmName.c_str());
}

void DH_ParmSol::setQuality(const Quality& quality)
{
  *itsFit = quality.itsFit;
  *itsRank = quality.itsRank;
}

void DH_ParmSol::setDomain(double fStart, double fEnd, double tStart, double tEnd)
{
  *itsStartFreq = fStart;
  *itsEndFreq = fEnd;
  *itsStartTime = tStart;
  *itsEndTime = tEnd;
}

void DH_ParmSol::setCoefficients(double c0, double c1, double c2, double c3)
{
  *itsCoeff0 = c0;
  *itsCoeff1 = c1;
  *itsCoeff2 = c2;
  *itsCoeff3 = c3;
}

void DH_ParmSol::clearData()
{
  setWorkOrderID(-1);
  setParmName("");
  setIteration(-1);
  Quality q;
  setQuality(q);
  setDomain(0, 0, 0, 0);
  setCoefficients(0);
}

void DH_ParmSol::dump()
{
  cout << "Workorder id = " << getWorkOrderID() << endl;
  cout << "Parameter name = " << getParmName() << endl;
  cout << "Iteration = " << getIteration() << endl;
  cout << "Fit = " << getFit() << endl;
  cout << "Rank = " << getRank() << endl;
  cout << "Start frequency = " << getStartFreq() << endl;
  cout << "End frequency = " << getEndFreq() << endl;
  cout << "Start time = " << getStartTime() << endl;
  cout << "End time = " << getEndTime() << endl;
  cout << "Coefficient 0 = " << getCoeff0() << endl;
  cout << "Coefficient 1 = " << getCoeff1() << endl;
  cout << "Coefficient 2 = " << getCoeff2() << endl;
  cout << "Coefficient 3 = " << getCoeff3() << endl;
}

namespace PL {

void DBRep<DH_ParmSol>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["WOID"] == itsWOID;
  cols["PARMNAME"] == itsParmName;
  cols["ITERATION"] == itsIter;
  cols["FIT"] == itsFit;
  cols["RANK"] == itsRank;
  cols["STARTFREQ"] == itsStartFreq;
  cols["ENDFREQ"] == itsEndFreq;
  cols["STARTTIME"] == itsStartTime;
  cols["ENDTIME"] == itsEndTime;
  cols["COEFF0"] == itsCoeff0;
  cols["COEFF1"] == itsCoeff1;
  cols["COEFF2"] == itsCoeff2;
  cols["COEFF3"] == itsCoeff3;
}

void DBRep<DH_ParmSol>::toDBRep (const DH_ParmSol& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsWOID = obj.getWorkOrderID();
  itsParmName = obj.getParmName();
  itsIter = obj.getIteration();
  itsFit = obj.getFit();
  itsRank = obj.getRank();
  itsStartFreq = obj.getStartFreq();
  itsEndFreq = obj.getEndFreq();
  itsStartTime = obj.getStartTime();
  itsEndTime = obj.getEndTime();
  itsCoeff0 = obj.getCoeff0();
  itsCoeff1 = obj.getCoeff1();
  itsCoeff2 = obj.getCoeff2();
  itsCoeff3 = obj.getCoeff3();
}

//# Force the instantiation of the templates.
template class TPersistentObject<DH_ParmSol>;

}  // end namespace PL

} // namespace LOFAR
