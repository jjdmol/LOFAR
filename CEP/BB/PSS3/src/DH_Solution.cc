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
#include <Common/Debug.h>
#include <sstream>

using namespace LOFAR;

DH_Solution::DH_Solution (const string& name, const string& type, int DBid)
  : DH_Postgresql    (name, "DH_Solution"),
    itsType          (type),
    itsDBid          (DBid)  
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_Solution::DH_Solution(const DH_Solution& that)
  : DH_Postgresql(that),
    itsType      (that.itsType),
    itsDBid      (that.itsDBid)
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_Solution::~DH_Solution()
{
}

DataHolder* DH_Solution::clone() const
{
  return new DH_Solution(*this);
}

void DH_Solution::preprocess()
{
}

void DH_Solution::postprocess()
{
}

DH_Solution::DataPacket::DataPacket()
  : itsID(-1),
    itsWOID(-1),
    itsIteration(-1)
{
  for (int i = 0; i < 10; i++)
  {
    itsRAValues[i] = 0;
    itsDECValues[i] = 0;
    itsStokesIValues[i] = 0;
    itsQuality.init();
  }
}

bool DH_Solution::StoreInDatabase(int, int, char*, int)
{
  // Create query
  std::ostringstream q1;
  q1 << "INSERT INTO BBSolutions VALUES ("
     << getID() << ", "
     << getWorkOrderID() << ", ";

  for (int sourceNo = 1; sourceNo <= 10; sourceNo++)
  {
    q1 << getRAValue(sourceNo) << ", "
       << getDECValue(sourceNo) << ", "
       << getStokesIValue(sourceNo) << ", ";
  }
  
  q1 << getIterationNo() << ", "
     << getQuality()->itsFit << ", "
     << getQuality()->itsMu << ", "
     << getQuality()->itsStddev << ", "
     << getQuality()->itsChi << "); ";
  
  TRACER1("DH_Solution::StoreInDatabase <<< Insert Solution QUERY: " 
	  << q1.str ());
  
  std::ostringstream q2;
  q2 << "UPDATE BBWorkOrders SET "
     << "Status = 2" << " "
     << "WHERE WOid = " << getWorkOrderID() << " AND Status = 1 ;";
  
  TRACER1("DH_Solution::StoreInDatabase <<< Update WorkOrder status QUERY: " 
	  << q2.str ());
  
  ExecuteSQLCommand(q1);
  ExecuteSQLCommand(q2);

  TRACER1("------ End of Knowledge Source write to database --------");
    
  return true;
}

bool DH_Solution::RetrieveFromDatabase(int, int, char*, int)
{
  if (itsType == "Control")
  {
    std::ostringstream q1;

    q1 << "SELECT * FROM BBWorkOrders WHERE "
       << "Status = " << 2 << ";";

    TRACER1("DH_Solution::RetrieveFromDatabase <<< WorkOrder QUERY: "
	    << q1.str ());

    PGresult * res1;

    // Block until a packet appears in the table
    do 
      {
	cerr << '.';
	res1 = PQexec (DH_Postgresql::theirConnection, (q1.str ()).c_str ());
    
	AssertStr (PQresultStatus (res1) == PGRES_TUPLES_OK,
		   "DH_Postgressql::Retrieve (); Select query failed.");
      } while (PQntuples (res1) == 0);

    int nRows;
    nRows = PQntuples (res1);
    AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
	       << "ERROR: Message table may not have been cleaned up before starting program.");

    setWorkOrderID(atoi(PQgetvalue(res1, 0, 0)));
 
    PQclear (res1);
  
    std::ostringstream q2;

    q2 << "SELECT * FROM BBSolutions WHERE "
       << "WOID = " << getWorkOrderID() << ";";

    TRACER1("DH_Solution::RetrieveFromDatabase <<< Solution QUERY: " 
	    << q2.str ());

    PGresult * res2;

    // Block until a packet appears in the table
    do 
      {
	cerr << '.';
	res2 = PQexec (DH_Postgresql::theirConnection, (q2.str ()).c_str ());
    
	AssertStr (PQresultStatus (res2) == PGRES_TUPLES_OK,
		   "DH_Postgressql::Retrieve (); Select query failed.");
      } while (PQntuples (res2) == 0);

    nRows = PQntuples (res2);
    AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
	       << "ERROR: Message table may not have been cleaned up before starting program.");

    int base;
    for (int sourceNo = 1; sourceNo <= 10; sourceNo++)
    { 
      base = 3*sourceNo;
      setRAValue(sourceNo, atoi(PQgetvalue(res2, 0, base-1)));
      setDECValue(sourceNo, atoi(PQgetvalue(res2, 0, base)));
      setStokesIValue(sourceNo, atoi(PQgetvalue(res2, 0, base+1)));
    }

     PQclear (res2);

    TRACER1("------ End of Controller read from database --------");
  }

  else if (itsType == "KS")
  {
    std::ostringstream q1;

    q1 << "SELECT * FROM BBSolutions WHERE "
       << "id = " << itsDBid << ";";

    TRACER1("DH_Solution::RetrieveFromDatabase <<< Solution QUERY: " 
	    << q1.str ());

    PGresult * resSol;

    // Block until a packet appears in the table
    do 
      {
	cerr << '.';
	resSol = PQexec (DH_Postgresql::theirConnection, (q1.str ()).c_str ());
    
	AssertStr (PQresultStatus (resSol) == PGRES_TUPLES_OK,
		   "DH_Postgressql::Retrieve (); Select query failed.");
      } while (PQntuples (resSol) == 0);

    int nRows = PQntuples (resSol);
    AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
	       << "ERROR: Message table may not have been cleaned up before starting program.");

    setID(atoi(PQgetvalue(resSol, 0, 0)));
    setWorkOrderID(atoi(PQgetvalue(resSol, 0, 1)));

    int base;
    for (int sourceNo = 1; sourceNo <= 10; sourceNo++)
    { 
      base = 3*sourceNo;
      setRAValue(sourceNo, atoi(PQgetvalue(resSol, 0, base-1)));
      setDECValue(sourceNo, atoi(PQgetvalue(resSol, 0, base)));
      setStokesIValue(sourceNo, atoi(PQgetvalue(resSol, 0, base+1)));
    }

    PQclear (resSol);

    TRACER1("------ End of Controller read from database --------");
  }

  return true;
}
