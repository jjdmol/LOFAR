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


#include "PSS3/DH_WorkOrder.h"
#include <Common/Debug.h>
#include <sstream>
#include <unistd.h> 

using namespace LOFAR;

int DH_WorkOrder::theirWriteCount = 0;

DH_WorkOrder::DH_WorkOrder (const string& name)
  : DH_Postgresql(name, "DH_WorkOrder")
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_WorkOrder::DH_WorkOrder(const DH_WorkOrder& that)
  : DH_Postgresql(that)
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_WorkOrder::~DH_WorkOrder()
{
  TRACER4("DH_WorkOrder destructor");
}

DataHolder* DH_WorkOrder::clone() const
{
  return new DH_WorkOrder(*this);
}

void DH_WorkOrder::preprocess()
{
}

void DH_WorkOrder::postprocess()
{
}

DH_WorkOrder::DataPacket::DataPacket()
  : itsWOID(-1),
    itsStatus(New),
    itsKSType("PSS3"),
    itsStrategyNo(0),
    itsArgSize(0),
    itsParam1Name("StokesI"),
    itsParam2Name("RA"),
    itsParam3Name("DEC"),
    itsStartSolution(-1)
{
}

void DH_WorkOrder::setArgSize(int size)
{ 
  AssertStr(size > 0, "Size is not greater than zero");
  AssertStr(size <= MAX_STRAT_ARGS_SIZE, "Size is greater than buffer size,increase buffer size (MAX_STRAT_ARGS_SIZE)");
  itsDataPacket.itsArgSize = size;
}

bool DH_WorkOrder::StoreInDatabase(int, int, char*, int)
{
    // First create blob:
    int i;
    std::ostringstream ostr;

    // Put varArgs in blob
    char hexrep [getArgSize()];
    for (i = 0; i < getArgSize(); i ++) {  
      sprintf (hexrep, "%02X ", itsDataPacket.itsVarArgs[i]);
      ostr << hexrep;
    }

  // Create insertion commands
  std::ostringstream q1;
  
  setWorkOrderID(theirWriteCount);

  q1 << "INSERT INTO BBWorkOrders VALUES ("
     << getWorkOrderID() << ", "
     << getStatus() << ", "
     << "'" << getKSType() << "', "
     << getStrategyNo() << ", "
     << getArgSize()    << ", "  
     << "'" << ostr.str() << "', "
     << "'" << getParam1Name() << "', "
     << "'" << getParam2Name() << "', "
     << "'" << getParam3Name() << "', "
     << getSolutionNumber() << ");";

  TRACER1("DH_WorkOrder::StoreInDatabase <<< Insert WorkOrder QUERY: " << q1.str ());

  ExecuteSQLCommand(q1);
  theirWriteCount ++;

  TRACER1("------ End of Controller write to database --------");
  
  return true; 
}

bool DH_WorkOrder::RetrieveFromDatabase(int, int, char*, int)
{
  bool selectedWorkOrder = false;
  PGresult* resWO;
  PGresult* resUpd;
  while (!selectedWorkOrder)   // Do this until a valid (single) workorder is found
  {

    // Construct workorder query in table BBWorkOrders
    std::ostringstream q1;

    q1 << "SELECT * FROM BBWorkOrders WHERE "      // Look for own name and general workOrders
       << "status = 0 AND "
       << "kstype = 'KS' OR " 
       << "kstype = '" << getName() << "' ORDER BY kstype DESC, woid ASC;";      

    TRACER1("DH_WorkOrder::RetrieveFromDatabase <<< WorkOrder QUERY: " << q1.str ());

    resWO = PQexec (DH_Postgresql::theirConnection, (q1.str ()).c_str ());
    // Block until a packet appears in the table
    while (PQntuples (resWO) == 0)
    {
      cout << ".";
      //     usleep(1000);
      resWO = PQexec (DH_Postgresql::theirConnection, (q1.str ()).c_str ());
      AssertStr (PQresultStatus (resWO) == PGRES_TUPLES_OK,
		 "DH_Postgressql::Retrieve (); Select query failed.");
    }

    int identifier = atoi(PQgetvalue(resWO, 0, 0));    


    // Update WorkOrder status in table BBWorkOrders
    std::ostringstream q3;
      
    q3 << "UPDATE BBWorkOrders SET "
       << "Status = 1" << " "
       << "WHERE WOid = " << identifier << " AND "
       << "Status = 0" << " ;";

    TRACER1("DH_WorkOrder::RetrieveFromDatabase <<< UPDATE COMMAND: " << q3.str ());

    resUpd = PQexec (DH_Postgresql::theirConnection, ((q3.str ()).c_str ()));

    AssertStr (PQresultStatus (resUpd) == PGRES_COMMAND_OK,
	       "ERROR: ExecuteCommand () Failed (" 
	       << PQresStatus (PQresultStatus (resUpd)) 
	       << "): " << q3.str ());
	
    
    if (atoi(PQcmdTuples(resUpd)) == 1)
    {
      selectedWorkOrder = true;                      // Found a valid entry in database
      TRACER1("Found WorkOrder with id " << identifier);
    }
    else
    {
      TRACER1("Update of WorkOrder failed. Number of rows affected by " 
	      << q3.str () << " is " << PQcmdTuples(resUpd) << ". Trying again all queries.");
      PQclear(resWO);
    }
    PQclear (resUpd);

  }

  // Put results in DataPacket
  setWorkOrderID(atoi(PQgetvalue(resWO, 0, 0)));
  setStatus((DH_WorkOrder::woStatus) atoi(PQgetvalue(resWO, 0, 1)));
  setKSType(PQgetvalue(resWO, 0, 2));
  setStrategyNo(atoi(PQgetvalue(resWO, 0, 3)));
  setArgSize(atoi(PQgetvalue(resWO, 0, 4)));
  setParam1Name(PQgetvalue(resWO, 0, 6));
  setParam2Name(PQgetvalue(resWO, 0, 7));
  setParam3Name(PQgetvalue(resWO, 0, 8));
  useSolutionNumber(atoi(PQgetvalue(resWO, 0, 9)));

  char token[getArgSize()];
     
  std::istringstream istr (PQgetvalue (resWO, 0, 5));
  for (int k = 0; k < getArgSize(); k ++) {
    istr >> token;
    itsDataPacket.itsVarArgs[k] = (char) strtoul (token, NULL, 16);
  }

  PQclear(resWO);

  TRACER1("------ End of Knowledge Source read from database --------");
    
  return true;

}
