//  LofarDatabase.h: Base class for accessing any Lofar database.
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

#if !defined(LOFAR_DATABASE_H)
#define LOFAR_DATABASE_H

//# Includes
// include <otherpackage/file.h>

//# Forward Declarations
// class xxx;

// LofarDatabase
//
/// The LofarDatabase class is used as an abstract base class for communicating 
/// with any kind of database engine used in the LOFAR project. 
/// The interface of the class is not based on SQL commands. This simplifies
/// the use of the database(s) and makes it possible to use also non-SQL
/// databases.
///
/// Several of the data-access functions return a integer-value as a result. 
/// This represents the number of records that were affected by the function.
///
/// \todo How should the records (= collections of fields) be passed in the 
/// interface?
class LofarDatabase {
public:
	//  CONSTRUCTOR
	/// When allocating a LofarDatabase instance you must specify which
	/// database on which server you want to use the instance for.
	/// You should also always specify a username for accessing this database.
	/// Most database systems like you to specify a password, but sometimes
	/// it is enough to pass an empty string ("").
	LofarDatabase(const char *database,
				  const char *server,
				  const char *username,
				  const char *password);

	//  DESTRUCTOR
	/// Deallocates the stored connection parameters.
	virtual ~LofarDatabase();

	//  SELECT TABLE
	/// Before using any of the data-access functions specify the table-name
	/// on which the functions will work.
	virtual	void	useTable	(const char	*tabelname);

	//  DATA-ACCESS FUNCTIONS

	/// Add ...depends of how to handle fields...
	virtual	int		add		(const char	*fields,
							 const char	*values);

	/// Searches the database for records that meet the given condition.
	/// All found records are deleted from the database.
	virtual	int		remove 	(const char	*condition);

	/// Searches the database for records that meet the given condition. Of
	/// the found records the specified fields are returned.
	///
	/// Fields: 
	/// \n The field should be specified by name in a comma-seperated list. To
	/// retrieve all fields an asterix should be used.
	///
	/// Condition:
	/// \n The syntax of the condition is like:
	///		 <fieldname> <oper> <value> [ AND|OR <fieldname> <oper> <value> ]
	///
	/// Result:
	/// \n The result of the database-search is formatted in a fresh allocated
	/// buffer and contains the field-values of a record seperated by a comma.
	/// The records are seperated by a linefeed.
	/// The user should take care of the deallocation of the result-buffer.
	virtual	int		find		(const char	*fields, 
								 const char *condition, 
								 char **result);

	/// Update ...depends on how to handle fields...
	virtual	int		update	(const char	*condition,
							 const char	*modification);

	//  FINISH WORK
	/// Commits the changes to the database. The Changes can \b not be
	/// undone anymore.
	/// \todo Can these command be implemented in a non-SQL database?
	virtual	void	commit		();

	/// Cancels the changes to the database. 
	/// \todo Can these command be implemented in a non-SQL database?
	virtual	void	rollback	();

	//  FINISH THE CONNECTION
	/// Closes the connection with the database. When the class-instance is
	/// not deleted but used again, the instance will reconnect to the database
	/// during the first function-call.
	virtual	void	closeDB		();

	//  SQL BACKDOOR
	/// Only use this function when complex table operations are neccesary
	/// that are not supported by the other data-access functions.
	/// It is \b not guaranteed that the sqlcommand will work in the future.
	//  TO DO:
	/// \todo Should this backdoor be implemented?
	virtual	int		SQLCmd		(const char	*sqlcommand);

protected:
	/// Returns the value of the connection-state to the friend-classes.
	inline bool		isConnected	() const;

	/// Function for the friend-classes to change the connnection-state.
	inline void		connected	(bool newState);

	/// Returns the databasename to friend-classes.
	inline char*	database	() const;

	/// Returns the servername to friend-classes.
	inline char*	server		() const;

	/// Returns the username to friend-classes.
	inline char*	username	() const;

	/// Returns the password to friend-classes.
	inline char*	password	() const;

	/// Returns the tablename to friend-classes.
	inline char*	tablename	() const;

private:
	char	*itsDatabase;
	char	*itsServer;
	char	*itsUsername;
	char	*itsPassword;
	char	*itsTablename;
	bool	itsConnected;

	/// The constructor is private to prevent creating raw baseclass objects
	LofarDatabase();

	/// Copying the abstract base-class is not allowed.
	//LofarDatabase&	LofarDatabase(const LofarDatabase& src);

	/// Copying the abstract base-class is not allowed.
	virtual LofarDatabase&	operator=    (const LofarDatabase& src);

	/// Only derived classes may make a copy of an instance.
	virtual LofarDatabase*	clone() const;

	friend class MySQLDatabase;
	friend class PgSQLDatabase;
};

////////////////////////// inline functions ///////////////////////////////

inline bool	LofarDatabase::isConnected	() const
	{ return itsConnected; }

inline void	LofarDatabase::connected	(bool newState)
	{  itsConnected = newState; }

inline char* LofarDatabase::database	() const
	{ return itsDatabase; }

inline char* LofarDatabase::server		() const
	{ return itsServer; }

inline char* LofarDatabase::username	() const
	{ return itsUsername; }

inline char* LofarDatabase::password	() const
	{ return itsPassword; }

inline char* LofarDatabase::tablename	() const
	{ return itsTablename; }


#endif
