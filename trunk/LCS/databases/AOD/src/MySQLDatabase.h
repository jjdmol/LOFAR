//  MySQLDatabase.h: Class for accessing any MySQL database.
//  The class is derived from the LofarDatabase base class.
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

#if !defined(MYSQL_DATABASE_H)
#define MYSQL_DATABASE_H

// Includes
//#include <LofarDatabase.h>
#include <sqlplus/sqlplus.hh>

// Forward Declarations
class LofarDatabase;


//
// Class to connect to any MySQL database.

class MySQLDatabase : public LofarDatabase {
public:
	// Constructors
	MySQLDatabase();
	MySQLDatabase(const char *database,
				  const char *server,
				  const char *username,
				  const char *password);

	// Destructor
	virtual ~MySQLDatabase();

	// Copy constructor
	//MySQLDatabase&	MySQLDatabase(const MySQLDatabase& src);

	// Assignment constructor
	MySQLDatabase&	operator=    (const MySQLDatabase& src);

	/// Makes a copy of the class instance.
	MySQLDatabase*	clone() const;
	// The integer returned by several functions represents the number
	// of records that were affected by the function.

	// select table
	void	useTable(const char	*tabelname);

	// do work
	int		add		(const char	*fields,
					 const char *values);
	int		remove	(const char	*condition);
	int		find	(const char	*fields, 
					 const char *condition, 
					 char **result);
	int		update	(const char	*condition,
					 const char	*modification);

	// finish work
	void	commit	();
	void	rollback();

	// finish the connection
	void	closeDB ();

	// SQL backdoor
	int		SQLCmd		(const char	*sqlcommand);

private:
	Connection	*itsConn;
	Query		*itsQuery;

	void	try2connect ();

};

////////////////////////// inline functions ///////////////////////////////



#endif
