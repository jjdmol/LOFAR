//  PgSQLDatabase.h: Class for accessing a PgSQL database.
//
//  Copyright (C) 2002
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

// Includes
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream.h>
#include <sys/time.h>
#include <pqxx/all.h>
#include <LofarDatabase.h>
#include <PgSQLDatabase.h>

// Forward Declarations
// class xxx;

using namespace pqxx;

//
// Default-constructor: reset all variables.
//
PgSQLDatabase::PgSQLDatabase() 
: LofarDatabase (),
  itsConn		(0),
  itsTrans		(0)
{
}


//
// Constructor with databasename, server, username and password
//
PgSQLDatabase::PgSQLDatabase(const char *database,
				  			 const char *server, 
							 const char *username,
				  			 const char *password)
			  
: LofarDatabase (database, server, username, password),
  itsConn		(0),
  itsTrans		(0)
{
	// Parameters are stored by the base class.
	// just try to connect.
	try2connect();

	printf ("SQL user:%s, password=%s, server=%s\n", 
					LofarDatabase::itsUsername, LofarDatabase::itsPassword, 
					LofarDatabase::itsServer);

}



//
// Destructor
//
PgSQLDatabase::~PgSQLDatabase(void)
{
	printf ("SQL destructor\n");

	// Clear allocated memory.
	if (!itsConn) 
		delete itsConn;
	if (!itsTrans)
		delete itsTrans;

}


//
// Copy constructor
//
PgSQLDatabase& PgSQLDatabase::operator= (const PgSQLDatabase& orgDatabase)
{
	printf ("SQL Copying database connection\n");

	// don't copy myself
	if (&orgDatabase == this) {
		return (*this);
	}

	// Clear allocated memory.
	if (!itsConn)  {
		delete itsConn;
		itsConn = 0;
	}
	if (!itsTrans) {
		delete itsTrans;
		itsTrans = 0;
	}

	// Copy internal parameters.
	if (orgDatabase.itsConn) {
		itsConn = orgDatabase.itsConn;
	}
	if (orgDatabase.itsTrans) {
		itsTrans = orgDatabase.itsTrans;
	}

	return (*this);
}

PgSQLDatabase*	PgSQLDatabase::clone() const
{
	return new PgSQLDatabase (*this);
}


void	PgSQLDatabase::useTable	(const char	*tabelname)
{
	LofarDatabase::useTable(tabelname);

	printf ("using table: %s\n", LofarDatabase::itsTablename);

	try2connect();
}

//
// try2connect
//
// Internal routine that tries to (re)connect to the database.
// 
void	PgSQLDatabase::try2connect ()
{
	if (isConnected()) {						// baseclass knows the state
		return;
	}

	// Allocate connection and try to connect
	char	connectParams [256];
	sprintf (connectParams, "host='%s' user='%s' dbname='%s'", 
											server(), username(), database());
	itsConn = new Connection(connectParams, false);
	
	// Allocate a transaction-object and bind it to this connection.
	itsTrans = new Transaction (*itsConn);

	// Update state
	LofarDatabase::connected(true);

}

int		PgSQLDatabase::add		(const char	*fields,
								 const char	*values)
{
	char	sqlCmd[512];

	// Construct an INSERT statement
	sprintf (sqlCmd, "INSERT INTO %s (%s) VALUES (%s)", tablename(), fields,
																	values);

	// Execute the query
	Result res = itsTrans->Exec(sqlCmd);

	printf ("%d rows were modified by the cmd: %s\n", res.size(), sqlCmd);

	return res.size();
}

int		PgSQLDatabase::remove	(const char	*condition)
{
	char	sqlCmd[512];

	// Construct an DELETE statement
	if (condition && *condition)
		sprintf (sqlCmd, "DELETE FROM %s WHERE %s", tablename(), condition);
	else
		sprintf (sqlCmd, "DELETE FROM %s", tablename());

	// Execute the query
	Result res = itsTrans->Exec(sqlCmd);
	itsTrans->Commit();

	return (0);
}

//
// find
//
// Does a search in the default table for records that meet the given
// condition. From the found records the specified fields are returned.
// The return-value tells how many records were found.
int		PgSQLDatabase::find		(const char	*fields,
								 const char	*condition,
								 char		**result)

{
	char			sqlCmd[512];
	const char		*fieldList = "*";
	struct timeval	t1, t2;
	int				ds, du;

	// Use the fields of the user if he/she has specified them,
	// otherwise return all fields.
	if (fields && *fields) {
		fieldList = fields;
	}

	// Construct an SELECT statement
	if (condition && *condition)
		sprintf (sqlCmd, "SELECT %s FROM %s WHERE %s", fieldList, tablename(), 
																	condition);
	else
		sprintf (sqlCmd, "SELECT %s FROM %s", fieldList, tablename());

	// Execute the query
	gettimeofday(&t1, 0L);
	Result	res = itsTrans->Exec(sqlCmd);
	gettimeofday(&t2, 0L);
	ds = t2.tv_sec - t1.tv_sec;
	du = t2.tv_usec - t1.tv_usec;
	if (t2.tv_usec < t1.tv_usec) {
		ds -= 1;
		du += 1000000;
	}


	// Show some results
	cout << "Query: " << sqlCmd << endl;
	cout << "Found " << res.size() << " records" << endl;
	cout << "time: " << ds << "." << du << endl;

	if (res.size() < 10) {
		// variables for showing the results
		int					i, j;

	// show column names
	//	row = *(res.begin());
	//	for (unsigned int f = 0; f < row.size(); f++)
	//		cout << res.names(f).c_str() << "	";
	//	cout << endl;
		
		// show record contents
		for (i = 0; i != res.size(); i++) {
			for (j = 0; j < res.Columns(); j++) {
				cout << res[i][j] << "	";
			}
			cout << endl;
		}
	}

	*result = (char*) &res;

	return res.size();
}



int		PgSQLDatabase::update	(const char	*condition,
								 const char	*modification)
{
	char	sqlCmd[512];

	// Construct an SELECT statement
	if (condition && *condition)
		sprintf (sqlCmd, "UPDATE %s SET %s WHERE %s", tablename(), 
													modification, condition);
	else
		sprintf (sqlCmd, "UPDATE %s SET %s", tablename(), modification);

	// Execute the query
	Result res = itsTrans->Exec(sqlCmd);

	printf ("%d rows were modified by the cmd: %s\n", res.size(), sqlCmd);

	return res.size();
}


void	PgSQLDatabase::closeDB	()
{
	// When its not disconnected yet
	if (!isConnected()) {
		itsConn->Deactivate();					// close connection

		delete itsConn;							// release all memory
		itsConn = 0;
		delete itsTrans;
		itsTrans = 0;

		LofarDatabase::connected(false);		// update state
	}

}


void	PgSQLDatabase::commit	()
{
	itsTrans->Commit();
}


void	PgSQLDatabase::rollback()
{
	itsTrans->Abort();
}


int		PgSQLDatabase::SQLCmd	(const char	*sqlcommand)
{
	return (0);
}

