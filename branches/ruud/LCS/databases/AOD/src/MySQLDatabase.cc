//  MySQLDatabase.h: Class for accessing a MySQL database.
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
#include <sqlplus/sqlplus.hh>
#include <LofarDatabase.h>
#include <MySQLDatabase.h>

// Forward Declarations
// class xxx;

//
// Default-constructor: reset all variables.
//
MySQLDatabase::MySQLDatabase() 
: LofarDatabase(),
  itsConn      (0)
{
	
	printf ("not much SQL to do\n");

}


//
// Constructor with databasename, server, username and password
//
MySQLDatabase::MySQLDatabase(const char *database,
				  			 const char *server, 
							 const char *username,
				  			 const char *password)
			  
: LofarDatabase (database, server, username, password),
  itsConn		(0),
  itsQuery		(0)
{

	printf ("SQL user:%s, password=%s, server=%s\n", 
					LofarDatabase::itsUsername, LofarDatabase::itsPassword, 
					LofarDatabase::itsServer);

	try2connect();
}



//
// Destructor
//
MySQLDatabase::~MySQLDatabase(void)
{
	printf ("SQL destructor\n");

	if (!itsConn) 
		delete itsConn;
	if (!itsQuery)
		delete itsQuery;

}


//
// Copy constructor
//
MySQLDatabase& MySQLDatabase::operator= (const MySQLDatabase& orgDatabase)
{
	printf ("SQL Copying database connection\n");

	return (*this);
}

MySQLDatabase*	MySQLDatabase::clone() const
{
	return new MySQLDatabase (*this);
}


void	MySQLDatabase::useTable	(const char	*tabelname)
{
	LofarDatabase::useTable(tabelname);

	printf ("using table: %s\n", LofarDatabase::itsTablename);

	try2connect();
}

void	MySQLDatabase::try2connect ()
{
	if (isConnected()) {
		return;
	}

	itsConn = new Connection(true);
	itsConn->connect (database(), server(),
					  username(), password());
	
	itsQuery = new Query (itsConn, true);

	LofarDatabase::connected(true);

}

void	MySQLDatabase::add		(const void	*record)
{
}

int		MySQLDatabase::remove	(const char	*condition)
{
}

int		MySQLDatabase::find		(const char	*fields,
								 const char	*condition,
								 char		**result)

{
	char	sqlCmd[512];
	const char	*fieldList = "*";

	if (fields && *fields) {
		fieldList = fields;
	}

	if (condition && *condition)
		sprintf (sqlCmd, "SELECT %s FROM %s WHERE %s", fieldList, tablename(), 
																	condition);
	else
		sprintf (sqlCmd, "SELECT %s FROM %s", fieldList, tablename());

	Result	res = itsQuery->store(sqlCmd);

	cout << "Query: " << sqlCmd << endl;
	cout << "Found " << res.size() << " records" << endl;

	*result = (char*) &res;

	return res.size();
}



int		MySQLDatabase::update	(const char	*condition,
								 const char	*modification)
{
}


void	MySQLDatabase::closeDB	()
{
}


void	MySQLDatabase::commit	()
{
}


void	MySQLDatabase::rollback()
{
}


int		MySQLDatabase::SQLCmd	(const char	*sqlcommand)
{
}



int main (void) {
	
	char	*qry_result;

	printf ("Running\n");

	LofarDatabase	*ldb = new MySQLDatabase ("my_database", "dop50", 
																"root", "");

	ldb->useTable("test_table");

//	ldb->find ("*", "", &qry_result);
	ldb->find ("name", "", &qry_result);


	delete ldb;


}
