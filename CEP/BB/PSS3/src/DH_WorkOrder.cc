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
#include "Common/Debug.h"

using namespace LOFAR;

int DH_WorkOrder::theirWriteCount = 0;
int DH_WorkOrder::theirReadCount = 0;

DH_WorkOrder::DH_WorkOrder (const string& name, const string& type)
  : DH_Postgresql(name, "DH_WorkOrder"),
    itsType      (type)
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_WorkOrder::DH_WorkOrder(const DH_WorkOrder& that)
  : DH_Postgresql(that),
    itsType        (that.itsType)
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
  : itsParam1Name("StokesI.CP"),
    itsParam1Value(0),
    itsParam2Name("RA.CP"),
    itsParam2Value(0),
    itsParam3Name("DEC.CP"),
    itsParam3Value(0),
    itsSrcNo(-1),
    itsStatus(New),
    itsKSType("PSS3"),
    itsStrategyNo(0),
    itsArgSize(0)
{
}

void DH_WorkOrder::setArgSize(int size)
{ 
  AssertStr(size > 0, "Size is not greater than zero");
  AssertStr(size <= MAX_STRAT_ARGS_SIZE, "Size is greater than buffer size,increase buffer size");
  itsDataPacket.itsArgSize = size;
}

bool DH_WorkOrder::StoreInDatabase(int, int, char* buf, int)
{
  DataPacket* data = (DataPacket*) buf; // Necessary at the moment, must be changed
                                        // in TH_Database.

  if (itsType == "Control")
  {
    // First create blob:
    int i;
    ostringstream ostr;

    // To do: add blob header

    // Put varArgs in blob
    char hexrep [data->itsArgSize];
    for (i = 0; i < data->itsArgSize; i ++) {  
      sprintf (hexrep, "%02X ", (unsigned char) (data->itsVarArgs[i]));
      ostr << hexrep;
    }

    // Create insertion commands
    ostringstream q1;
    ostringstream q2;

    q1 << "INSERT INTO BBWorkOrders VALUES ("
       << theirWriteCount << ", "
       << data->itsStatus << ", "
       << "'" << data->itsKSType << "', "
       << data->itsStrategyNo << ", "
       << data->itsArgSize    << ", "  
       << "'" << ostr.str() << "');";

    cout << "DH_WorkOrder::StoreInDatabase <<< Insert WorkOrder QUERY: " << q1.str () << endl;

    q2 << "INSERT INTO BBSolutions (id, p1name, p2name, p3name) VALUES ("
       << theirWriteCount << ", "
       << "'" << data->itsParam1Name << "', "
       << "'" << data->itsParam2Name << "', "
       << "'" << data->itsParam3Name << "'); ";

    cout << "DH_WorkOrder::StoreInDatabase <<< Insert Solution QUERY: " << q2.str () << endl;

    ExecuteSQLCommand(q1);
    ExecuteSQLCommand(q2);

    theirWriteCount ++;

    return true; 
  }
  else if (itsType == "KS")
  {
    ostringstream q;
    q << "UPDATE BBSolutions SET "
      << "P1ParamValue = " << "'" << data->itsParam1Value << "', "
      << "P2ParamValue = " << "'" << data->itsParam2Value << "', "
      << "P3ParamValue = " << "'" << data->itsParam3Value << "', "
      << "SrcNo = " << data->itsSolution.itsMu << " "
      << "WHERE ID = " << data->itsID << "); ";

    cout << "DH_WorkOrder::StoreInDatabase <<< Update Solution QUERY: " << q.str () << endl;


    ExecuteSQLCommand(q);

    return true;
  }
  return false;
}

bool DH_WorkOrder::RetrieveFromDatabase(int, int, char* buf, int size)
{
  if (itsType == "KS")
  {
    // Construct workorder query in table BBWorkOrders
    ostringstream q1;

    q1 << "SELECT * FROM BBWorkOrders WHERE "
       << "id = " << theirReadCount << " AND "
       << "status = 0 AND "
       << "kstype = 'PSS3' " << ";";

    cout << "DH_WorkOrder::RetrieveFromDatabase <<< WorkOrder QUERY: " << q1.str () << endl;

    PGresult * res1;

    // Block until a packet appears in the table
    do {
      cerr << '.';
      res1 = PQexec (DH_Postgresql::theirConnection, (q1.str ()).c_str ());
  
      AssertStr (PQresultStatus (res1) == PGRES_TUPLES_OK,
		 "DH_Postgressql::Retrieve (); Select query failed.")
	// TODO: Do a PQclear here?
	} while (PQntuples (res1) == 0);

    int nRows;
    nRows = PQntuples (res1);
    AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
	       << "ERROR: Found less or more than 1 message in database.")
  
       DataPacket* data = (DataPacket*) buf; // Necessary at the moment, must be changed
    // in TH_Database.

    char token[40];
 
    istringstream istr (PQgetvalue (res1, 0, 5));
    for (int k = 0; k < size; k ++) {
      istr >> token;
      data->itsVarArgs[k] = (char) strtoul (token, NULL, 16);
    }

    // Construct query for parameter names in table BBSolutions
    ostringstream q2;

    q2 << "SELECT * FROM BBSolutions WHERE "
       << "id = " << theirReadCount << ";";

    cout << "DH_WorkOrder::RetrieveFromDatabase <<< Params QUERY: " << q2.str () << endl;

    PGresult * res2;

    // Block until a packet appears in the table
    do {
      cerr << '.';
      res2 = PQexec (DH_Postgresql::theirConnection, (q2.str ()).c_str ());
  
      AssertStr (PQresultStatus (res2) == PGRES_TUPLES_OK,
		 "DH_Postgressql::Retrieve (); Select query failed.")
	// TODO: Do a PQclear here?
	} while (PQntuples (res2) == 0);

    nRows = PQntuples (res2);
    AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
	       << "ERROR: Found less or more than 1 message in database.");


    // Update WorkOrder status in table BBWorkOrders
    ostringstream q3;

    q3 << "UPDATE BBWorkOrders SET "
       << "Status = 1" << " "
       << "WHERE id = " << theirReadCount << " AND "
       << "Status = 0" << " ;";

    cout << "DH_WorkOrder::RetrieveFromDatabase <<< UPDATE COMMAND: " << q3.str () << endl;

    ExecuteSQLCommand(q3);


    // Put results in DataPacket
    data->itsStatus = (DH_WorkOrder::woStatus) atoi(PQgetvalue(res1, 0, 1));
    data->itsKSType = PQgetvalue(res1, 0, 2);
    data->itsStrategyNo = atoi(PQgetvalue(res1, 0, 3));
    data->itsArgSize = atoi(PQgetvalue(res1, 0, 4));

    data->itsID = atoi(PQgetvalue(res2, 0, 0));
    data->itsParam1Name = PQgetvalue(res2, 0, 1);
    data->itsParam2Name = PQgetvalue(res2, 0, 2);
    data->itsParam3Name = PQgetvalue(res2, 0, 3);

    PQclear(res1);
    PQclear(res2);

    theirReadCount++;
    return true;
  }
  else if (itsType == "Control")
  {
    // Code to be added.
    return true;
  }

  return false;
}
