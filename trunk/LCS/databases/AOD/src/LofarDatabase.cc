//  LofarDatabase.cc: Class for accessing a Lofar database.
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
//
//  NOTE: only the constructors and destructors are implemented, all other
//		  functions are data-access functions which must be implemented in 
//		  the derived classes.

// Includes
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <LofarDatabase.h>

// Forward Declarations
// class xxx;

//
// Default-constructor: reset all variables.
//
LofarDatabase::LofarDatabase()
: itsDatabase  (0),
  itsServer    (0),
  itsUsername  (0),
  itsPassword  (0),
  itsTablename (0),
  itsConnected (false)
{
	printf ("not much to do\n");
}


//
// Constructor with databasename, server, username and password
//
LofarDatabase::LofarDatabase(const char *database,
				  			 const char *server,
							 const char *username,
				  			 const char *password)
: itsTablename (0),
  itsConnected (false)
{
	// Copy the parameter to local storage.
	itsDatabase = new char[strlen(database)+1];
	strcpy (itsDatabase, database);
	itsServer = new char[strlen(server)+1];
	strcpy (itsServer, server);
	itsUsername = new char[strlen(username)+1];
	strcpy (itsUsername, username);
	itsPassword = new char[strlen(password)+1];
	strcpy (itsPassword, password);

	printf ("user:%s, password=%s, server=%s\n", itsUsername, 
														itsPassword, itsServer);

}


//
// Destructor
//
LofarDatabase::~LofarDatabase()
{
	if (itsUsername)
		printf ("Destructing:%s, %s\n", itsUsername, itsPassword);
	else
		printf ("Destructing unknown\n");

	// Release all allocated memory.
	if (itsDatabase)
		delete[] itsDatabase;
	if (itsServer)
		delete[] itsServer;
	if (itsUsername)
		delete[] itsUsername;
	if (itsPassword)
		delete[] itsPassword;
	if (itsTablename)
		delete[] itsTablename;

}


//
// Copy constructor
//
LofarDatabase& LofarDatabase::operator= (const LofarDatabase& orgDatabase)
{
	printf ("Copying database connection\n");

	// don't copy myself
	if (this == &orgDatabase) {
		return (*this);
	}

	printf ("...deleting internal parameters\n");

	// release allocated memory
	if (itsDatabase) {
		delete[] itsDatabase;
		itsDatabase = 0;
	}
	if (itsServer) {
		delete[] itsServer;
		itsServer = 0;
	}
	if (itsUsername) {
		delete[] itsUsername;
		itsUsername = 0;
	}
	if (itsPassword) {
		delete[] itsPassword;
		itsPassword = 0;
	}
	if (itsTablename) {
		delete[] itsTablename;
		itsTablename = 0;
	}

	printf ("...copying internal parameters\n");
	
	// Copy setting from original object.
	if (orgDatabase.itsDatabase) {
		itsServer = new char[strlen(orgDatabase.itsDatabase)+1];
		strcpy (itsDatabase, orgDatabase.itsDatabase);
	}
	if (orgDatabase.itsServer) {
		itsServer = new char[strlen(orgDatabase.itsServer)+1];
		strcpy (itsServer, orgDatabase.itsServer);
	}
	if (orgDatabase.itsUsername) {
		itsUsername = new char[strlen(orgDatabase.itsUsername)+1];
		strcpy (itsUsername, orgDatabase.itsUsername);
	}
	if (orgDatabase.itsPassword) {
		itsPassword = new char[strlen(orgDatabase.itsPassword)+1];
		strcpy (itsPassword, orgDatabase.itsPassword);
	}
	if (orgDatabase.itsTablename) {
		itsTablename = new char[strlen(orgDatabase.itsTablename)+1];
		strcpy (itsTablename, orgDatabase.itsTablename);
	}

	itsConnected = orgDatabase.itsConnected;

	return (*this);
}

LofarDatabase*	LofarDatabase::clone() const
{
	return new LofarDatabase (*this);
}

void	LofarDatabase::useTable	(const char	*tabelname)
{
	// Remove old value if any.
	if (itsTablename) {
		delete[] itsTablename;
	}

	itsTablename = new char[strlen(tabelname)+1];
	strcpy (itsTablename, tabelname);

}

//
// DATA-ACCESS FUNCTIONS: to be implemented in the derived classes
//
int		LofarDatabase::add	(const char	*fields,
							 const char	*values)
{ 
}

int		LofarDatabase::remove(const char	*condition)
{ 
}

int		LofarDatabase::find	(const char *fields,
							 const char	*condition, 
							 char **result)
{ return 0; }


int		LofarDatabase::update(const char	*condition,
								 const char	*modification)
{ return 0; }


void	LofarDatabase::closeDB()
{
}


void	LofarDatabase::commit ()
{
}


void	LofarDatabase::rollback()
{
}


int		LofarDatabase::SQLCmd	(const char	*sqlcommand)
{ return 0; }

