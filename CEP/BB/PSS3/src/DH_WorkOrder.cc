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

bool DH_WorkOrder::StoreInDatabase(int, int, char*, int)
{
  if (itsType == "Control")
  {
    // First create blob:
    int i;
    ostringstream ostr;

    // To do: add blob header

    // Put varArgs in blob
    char hexrep [getArgSize()];
    for (i = 0; i < getArgSize(); i ++) {  
      sprintf (hexrep, "%02X ", (unsigned char) (itsDataPacket.itsVarArgs[i]));
      ostr << hexrep;
    }

    // Create insertion commands
    ostringstream q1;
    ostringstream q2;

    q1 << "INSERT INTO BBWorkOrders VALUES ("
       << theirWriteCount << ", "
       << getStatus() << ", "
       << "'" << getKSType() << "', "
       << getStrategyNo() << ", "
       << getArgSize()    << ", "  
       << "'" << ostr.str() << "');";

    cout << "DH_WorkOrder::StoreInDatabase <<< Insert WorkOrder QUERY: " << q1.str () << endl;

    q2 << "INSERT INTO BBSolutions (id, p1name, p2name, p3name) VALUES ("
       << theirWriteCount << ", "
       << "'" << getParam1Name() << "', "
       << "'" << getParam2Name() << "', "
       << "'" << getParam3Name() << "'); ";

    cout << "DH_WorkOrder::StoreInDatabase <<< Insert Solution QUERY: " << q2.str () << endl;

    ExecuteSQLCommand(q1);
    ExecuteSQLCommand(q2);

    theirWriteCount ++;

    cout << "------ End of Controller write to database --------" << endl;
    cout << endl;

    return true; 
  }
  else if (itsType == "KS")
  {
    ostringstream q1;
    q1 << "UPDATE BBSolutions SET "
       << "P1Value = " << "'" << getParam1Value() << "', "
       << "P2Value = " << "'" << getParam2Value() << "', "
       << "P3Value = " << "'" << getParam3Value() << "', "
       << "SrcNo = " << getSourceNo() << " , " 
       << "SolQual = " << getSolution()->itsMu << " "
       << "WHERE ID = " << getID() << " ;";

    cout << "DH_WorkOrder::StoreInDatabase <<< Update Solution QUERY: " << q1.str () << endl;

    ostringstream q2;
    q2 << "UPDATE BBWorkOrders SET "
       << "Status = " << getStatus() << " "
       << "WHERE ID = " << getID() << " ;";

    cout << "DH_WorkOrder::StoreInDatabase <<< Update WorkOrder status QUERY: " << q2.str () << endl;
    
 
    ExecuteSQLCommand(q1);
    ExecuteSQLCommand(q2);

    cout << "------ End of Knowledge Source write to database --------" << endl;    cout << endl;

    return true;
  }
  return false;
}

bool DH_WorkOrder::RetrieveFromDatabase(int, int, char*, int)
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
    do
    {
      cerr << '.';
      res1 = PQexec (DH_Postgresql::theirConnection, (q1.str ()).c_str ());
  
      AssertStr (PQresultStatus (res1) == PGRES_TUPLES_OK,
		 "DH_Postgressql::Retrieve (); Select query failed.")
    } while (PQntuples (res1) == 0);

    int nRows;
    nRows = PQntuples (res1);
    AssertStr (nRows == 1, "DH_Postgresql::Retrieve ();"
	       << "ERROR: Found less or more than 1 message in database.")
 
    // Put results in DataPacket
    setStatus((DH_WorkOrder::woStatus) atoi(PQgetvalue(res1, 0, 1)));
    setKSType(PQgetvalue(res1, 0, 2));
    setStrategyNo(atoi(PQgetvalue(res1, 0, 3)));
    setArgSize(atoi(PQgetvalue(res1, 0, 4)));

    char token[getArgSize()];
     
    istringstream istr (PQgetvalue (res1, 0, 5));
    for (int k = 0; k < getArgSize(); k ++) {
      istr >> token;
      itsDataPacket.itsVarArgs[k] = (char) strtoul (token, NULL, 16);
    }

    PQclear(res1);

    // Construct query for parameter names in table BBSolutions
    ostringstream q2;

    q2 << "SELECT * FROM BBSolutions WHERE "
       << "id = " << theirReadCount << ";";

    cout << "DH_WorkOrder::RetrieveFromDatabase <<< Params QUERY: " << q2.str () << endl;

    PGresult * res2;

    // Block until a packet appears in the table
    do 
    {
      cerr << '.';
      res2 = PQexec (DH_Postgresql::theirConnection, (q2.str ()).c_str ());
  
      AssertStr (PQresultStatus (res2) == PGRES_TUPLES_OK,
		 "DH_Postgressql::Retrieve (); Select query failed.")
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

    setID(atoi(PQgetvalue(res2, 0, 0)));
    setParam1Name(PQgetvalue(res2, 0, 1));
    setParam2Name(PQgetvalue(res2, 0, 2));
    setParam3Name(PQgetvalue(res2, 0, 3));

    PQclear(res2);

    theirReadCount++;

    cout << "------ End of Knowledge Source read from database --------" << endl;
    cout << endl;

    return true;
  }

  else if (itsType == "Control")
  {
    ostringstream q1;

    q1 << "SELECT * FROM BBWorkOrders WHERE "
       << "Status = " << 2 << ";";

    cout << "DH_WorkOrder::RetrieveFromDatabase <<< Controller WorkOrder QUERY: " << q1.str () << endl;

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

    setID(atoi(PQgetvalue(res1, 0, 0)));
 
    PQclear (res1);


    ostringstream q2;

    q2 << "SELECT * FROM BBSolutions WHERE "
       << "ID = " << getID() << ";";

    cout << "DH_WorkOrder::RetrieveFromDatabase <<< Controller Solution QUERY: " << q2.str () << endl;

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

    setParam1Value(atoi(PQgetvalue(res1, 0, 4)));
    setParam2Value(atoi(PQgetvalue(res1, 0, 5)));
    setParam3Value(atoi(PQgetvalue(res1, 0, 6)));

    PQclear (res2);

    cout << "------ End of Controller read from database --------" << endl;
    cout << endl;

    return true;

  }

  return false;
}
