//  testAOD.h: Testprogram for testing the LofarDatabase class and
//			   its derived classes.
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
#include <PgSQLDatabase.h>
#include <MySQLDatabase.h>

// Forward Declarations
// class xxx;

static	bool			hasTable	= false;
static	bool			hasDatabase	= false;
static	char			theTable    [256] = "";
static	char			theDatabase [256] = "";
static	LofarDatabase	*ldb = 0;

static	char			theEngine	[256] = "MySQL";
static	char			theUser		[256] = "root";

void showMenu ()
{
	cout << endl << endl << endl;
	cout << "Commands:" << endl;
	cout << "e	choose Engine(" << theEngine << ")"   << endl;
	cout << "u	set Username (" << theUser << ")"     << endl;
	cout << "d	use Database (" << theDatabase << ")" << endl;
	cout << "t	use Table    (" << theTable << ")"    << endl << endl;
	if (hasTable && hasDatabase) {
		cout << "a	Add a record" << endl;
		cout << "c	Change record(s)" << endl;
		cout << "r	Remove record(s)" << endl;
		cout << "f	Find record(s)" << endl << endl;
		cout << "C	Commit change(s)" << endl;
		cout << "R	Rollback change(s)" << endl << endl;
	}
	cout << "q	Quit program" << endl;
	cout << endl << "Type the letter of the command: ";
}

char getMenuChoice ()
{
	char	choice;

	for (;;) {
		showMenu ();
		cin >> choice;
		switch (choice) {
			case 't':							// always legal
			case 'd':
			case 'e':
			case 'u':
			case 'q':
				return (choice);

			case 'a':							// only legal when connected
			case 'c':
			case 'r':
			case 'f':
			case 'C':
			case 'R':
				if (hasTable && hasDatabase) {
					return (choice);
				}
		}
		cout << "Sorry wrong choice" << endl;
	} // while no legal choice
}

void closeConnection ()
{
	if (ldb) {
		ldb->closeDB();
		delete ldb;
		ldb = 0;
	}
}


void refreshConnection ()
{
	// closes and opens the DB connection

	closeConnection();

	if (!hasDatabase)
		return;

	if (!strcmp(theEngine, "MySQL"))
		ldb = new MySQLDatabase (theDatabase, "dop50", theUser, ""); 
	else
		ldb = new PgSQLDatabase (theDatabase, "dop50", theUser, ""); 

	if (hasTable)
		ldb->useTable(theTable);
}

void getUsername (char	*newName)
{
	if (newName) {
		strcpy (theUser, newName);
	} else {
		cout << "Username (" << theUser << "): ";
		cin >> theUser;
	}

	refreshConnection ();
}


void getDatabaseEngine (char	*newEngine)
{
	char	choice = '\0';

	closeConnection();

	theDatabase[0] = '\0';
	hasDatabase    = false;
	theTable[0]    = '\0';
	hasTable       = false;

	if (!newEngine) {
		cout << "Database Engine is " << theEngine << endl;
		cout <<	"m	MySQL" << endl;
		cout << "p	PostgreSQL" << endl << endl;
		cout << "Choose a new one: ";
	}

	for (;;) {
		if (newEngine && *newEngine) {
			choice = newEngine[0];
			newEngine[0] = '\0';
		} else {
			cin >> choice;
		}
		switch (choice) {
		case 'm':
			strcpy (theEngine, "MySQL");
			strcpy (theUser,   "root");
			return;
		case 'p':
			strcpy (theEngine, "PostgreSQL");
			strcpy (theUser,   "postgres");
			return;
		}
	}
}



void getTablename (char	*newTable)
{
	if (newTable) {
		strcpy (theTable, newTable);
	} else {
		if (hasTable)
			cout << "Tablename (" << theTable << "): ";
		else
			cout << "Tablename: ";
	
		cin >> theTable;
	}

	hasTable = (theTable[0] != '\0');

	if (hasDatabase)
		ldb->useTable(theTable);
}

	

void getDatabasename (char	*newDatabase)
{
	if (newDatabase) {
		strcpy (theDatabase, newDatabase);
	} else {
		if (hasDatabase)
			cout << "Databasename (" << theDatabase << "): ";
		else
			cout << "Databasename: ";

		cin >> theDatabase;
	}

	hasDatabase = (theDatabase[0] != '\0');

	refreshConnection ();

}

void	doChangeCmd (LofarDatabase	*ldb, 
					 char			*theModification, 
					 char		  	*theCondition)
{
	char	modification [4096];
	char	condition	 [4096];

	if (theModification && *theModification) {
		strcpy (modification, theModification);
	} else {
		cout << "Enter modification like: name='klaas' and 'id=id+1'" << endl;
		cout << "Modification: ";
		cin >> modification;
	}

	if (theCondition && *theCondition) {
		strcpy (condition, theCondition);
	} else {
		cout << "Enter condition like: name='piet' and lastname='hein'" << endl;
		cout << " or type '*' to get all records." << endl;
		cout << "Condition: ";
		cin >> condition;
	}

	if (condition[0] == '*')
		condition[0] = '\0';

	cout << ldb->update (condition, modification) << " records updated" << endl;

}
void	doRemoveCmd (LofarDatabase *ldb)
{
	char	condition [4096];

	cout << "Enter condition like: name='piet' and lastname='hein'" << endl;
	cout << " or type '*' to remove all records." << endl;
	cout << "Condition: ";
	cin >> condition;
	if (condition[0] == '*')
		condition[0] = '\0';

	cout << ldb->remove (condition) << " records removed" << endl;

}
	
void	doFindCmd (LofarDatabase *ldb, char	*theFields, char *theCondition)
{
	char	*qryResult;
	char	fields 	  [4096];
	char	condition [4096];

	if (theFields && *theFields) {
		strcpy (fields, theFields);
	} else {
		cout << "Enter comma-separated list of fieldnames or '*'" << endl;
		cout << "Fields: ";
		cin >> fields;
	}

	if (theCondition && *theCondition) {
		strcpy (condition, theCondition);
	} else {
		cout << "Enter condition like: name='piet' and lastname='hein'" << endl;
		cout << " or type '*' to get all records." << endl;
		cout << "Condition: ";
		cin >> condition;
	}

	if (condition[0] == '*')
		condition[0] = '\0';

	cout << ldb->find (fields, condition, &qryResult) << " records found" << endl;
}

char parse_args(int	argc, char	*argv[])
{
	char	c;

	opterr = 0;
	while ((c = getopt(argc, argv, "e:u:d:t:c:f:CRq")) > 0) {
		switch (c) {
		case 'd':						// use Database
			getDatabasename(optarg);
			break;

		case 't':						// use Table
			getTablename(optarg);
			break;

		case 'u':						// set Username
			getUsername(optarg);
			break;

		case 'e':						// choose Engine
			getDatabaseEngine(optarg);
			break;

		case 'c':
			{
				char	*modification, *condition;
				modification = strtok (optarg, "|");
				condition    = strtok (0L, "|");
				doChangeCmd (ldb, modification, condition);
			}
			break;

		case 'f':
			{
				char	*fields, *condition;
				fields = strtok (optarg, "|");
				condition = strtok (0L, "|");
				doFindCmd (ldb, fields, condition);
			}
			break;

		case 'C':
			ldb->commit();
			break;

		case 'R':
			ldb->rollback();
			break;

		case 'q':
			return ('q');

		default:
			cout << "Unknown option [" << (char)optopt << "]" << endl;
			cout << "Usage: " << basename(argv[0]) << " [options]" << endl;
			cout << "  -e m|p (MySQL|PostgreSQL)" << endl;
			cout << "  -u username" << endl;
			cout << "  -d database" << endl;
			cout << "  -t table" << endl;
			cout << "  -c 'modification|condition'" << endl;
			cout << "  -f 'fieldlist|condition' or '*|*'" << endl;
			cout << "  -C (commit)" << endl;
			cout << "  -R (rollback)" << endl;
			cout << "  -q " << endl;
			cout << "All options except -q are repeatable." << endl;
			exit (1);
		}
	
	}
	return ('\0');

}

int main (int argc, char *argv[]) 
{

	char	theChoice = parse_args(argc, argv);

	while (theChoice != 'q') {
		switch (theChoice = getMenuChoice ()) {
		case 'd':						// use Database
			getDatabasename(0L);
			break;

		case 't':						// use Table
			getTablename(0L);
			break;

		case 'u':						// set Username
			getUsername(0L);
			break;

		case 'e':						// choose Engine
			getDatabaseEngine(0L);
			break;

		case 'a':						// Add record
			break;

		case 'c':						// Change record(s)
			doChangeCmd (ldb, 0L, 0L);
			break;

		case 'r':						// Remove record(s)
			doRemoveCmd (ldb);
			break;

		case 'f':						// Find record(s)
			doFindCmd (ldb, 0L, 0L);
			break;

		case 'C':
			ldb->commit();
			break;

		case 'R':
			ldb->rollback();
			break;

		case 'q':						// Quit
			break;
		}
	}

	if (ldb)
		delete ldb;

}
