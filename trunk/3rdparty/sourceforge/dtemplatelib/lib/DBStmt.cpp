/* Copyright � 2000 
Michael Gradman and Corwin Joy 

Permission to use, copy, modify, distribute and sell this software and 
its documentation for any purpose is hereby granted without fee, provided 
that the above copyright notice appears in all copies and that both that 
copyright notice and this permission notice appear in supporting documentation. 
Corwin Joy and Michael Gradman make no representations about the suitability 
of this software for any purpose. 
It is provided "as is" without express or implied warranty.
*/ 
#include "DBStmt.h"
#include "clib_fwd.h"

#ifdef  WIN32
#ifdef WIN32
	#ifndef DTL_USE_MFC
		#include <windows.h>
	#else 
		#include <afx.h>
	#endif
#endif

#endif
#include <sql.h>
#include <sqlext.h>

#include "std_warn_off.h"
#include <algorithm>
#include <cassert>
#include "std_warn_on.h"

#include "string_util.h"


// Edited: 12/19/2000 - MG - added namespaces

BEGIN_DTL_NAMESPACE

#ifdef DTL_MEM_DEBUG

// ************** implementation code for mcheck **********************
unsigned long mcheck::calc_check() {
		BYTE *p;
		size_t i;
		long check = 0;

		for (p = (BYTE *)ptr, i=0; i < size; i++) {
			check = (check + *(p+i))%LONG_MAX;
		}
		return check;
}

void mcheck::write_mem(void) {
		BYTE *p;
		size_t i;
		
		for (p = (BYTE *)ptr, i=0; i < size; i++) {
			*(p+i) = (BYTE)0xFF;
		}
}

mcheck::mcheck(void *TargetValuePtr, size_t BufferLength) : ptr(TargetValuePtr),
		size(BufferLength) , checksum(calc_check())
{}

bool mcheck::validate() {
		bool b_valid  = (checksum == calc_check());
		//write_mem(); // test write to memory
		return b_valid;

}

void mcheck::revalidate() {
		checksum = calc_check();
}


#endif

// ************** Implementation code for DBStmt *****************
HSTMT GetHSTMT(const DBStmt &stmt)
{
   if (stmt.state == DBStmt::STMT_UNALLOCATED)
			throw DBException(_TEXT("DBStmt::GetHSTMT()"),
				_TEXT("Statement is not allocated!"), NULL, NULL);

   return stmt.hstmt;
}

HSTMT DBStmt::GetHSTMT() const
{
   if (state == STMT_UNALLOCATED)
			throw DBException(_TEXT("DBStmt::GetHSTMT()"),
				_TEXT("Statement is not allocated!"), NULL, NULL);

   return hstmt;
}

// errors deemed fatal to a DBStmt on Execute()
// causes the DBStmt to become invalidated
// all other errors do not validate the DBStmt

// anything bad due to the SQL Query String (unless if possibly due to
// bad parameters) will be considered fatal
	const STD_::vector<tstring> DBStmt::FatalErrors = DBStmt::BuildFatalErrorsList();

	STD_::vector<tstring> DBStmt::BuildFatalErrorsList()
	{
		STD_::vector<tstring> errors;

	    errors.push_back(_TEXT("07002")); // COUNT field incorrect ... wrong # of parameters to SQLBindParam
	    errors.push_back(_TEXT("08S01")); // Communication link failure (lost connection to driver)
	    errors.push_back(_TEXT("21S02")); // Degree of table error ... number of columns mismatch
	    errors.push_back(_TEXT("42000")); // Syntax error or access violation
	    errors.push_back(_TEXT("HY105")); // Invalid parameter type (input or output)
	    errors.push_back(_TEXT("HYC00")); // Optional feature not implemented
        errors.push_back(_TEXT("HYT00")); // Timeout expired for operation
	    errors.push_back(_TEXT("HYT01")); // Connection timeout expired
	    errors.push_back(_TEXT("IM001")); // Driver does not support this function

		errors.push_back(_TEXT("01006")); // Privilege not revoked
		errors.push_back(_TEXT("01007")); // Privilege not granted
		errors.push_back(_TEXT("40001")); // Serialiazation failure
		errors.push_back(_TEXT("HY000")); // General error / no statement parsed
		errors.push_back(_TEXT("HY010")); // Function sequence error
		errors.push_back(_TEXT("HY109")); // Invalid cursor position
		errors.push_back(_TEXT("42S22")); // Invalid column name
		errors.push_back(_TEXT("07001")); // SQLBindParameter() not called for parameter
		errors.push_back(_TEXT("37000")); // Native error
		errors.push_back(_TEXT("42S02")); // No such table or view exists
		return errors;
	}

	// constructs a DBStmt associated with the connection
	// allocate (and prepare stmt) if alloc_stmt set to true
	DBStmt::DBStmt(const tstring &query, const DBConnection &connection, bool PrepareFlag, bool CaseifyFlag):
		   ValidatedObject(), pConn(NULL),
		   hstmt(SQL_NULL_HSTMT), sqlQuery(query), state(STMT_UNALLOCATED), attrs(), bPrepare(PrepareFlag)
	{ 
	   DBConnection &connxn = const_cast<DBConnection &>(connection);
	   pConn = &connxn; 
	   if (CaseifyFlag)
			sqlQuery = QueryCaseify(sqlQuery, connection.GetQuoteChar()); 


	   // In the case of SQL Server set the cursor type to use optimistic concurrency
	   // Why? This gives us server side cursors rather than the default client
	   // side cursors which then allows us to have more than one statement open
	   // at a time (at a slight performance penalty).
	   //
	   // Here we set server side cursors as a default attribute at construction.
	   // The user can override this by calling ClearStmtAttrs after construction.
	   // For SQL server this should only be necessary when executing SQL that returns
	   // multiple result set accessed via MoreResults.  In this case, a client side
	   // cursor is required and the attribute set below must be cleared.  See the
	   // sql_iterator documentation for an example.
	   if (pConn->GetDBMSEnum() == DBConnection::DB_SQL_SERVER) {
			SetStmtAttr(SQL_ATTR_CONCURRENCY, SQL_CONCUR_ROWVER,
						SQL_IS_INTEGER);
	   }
	}

		   
	// no-throw swap
    void DBStmt::swap(DBStmt &other) {
		STD_::swap(pConn, other.pConn);
		STD_::swap(hstmt, other.hstmt);
		sqlQuery.swap(other.sqlQuery);
		STD_::swap(state, other.state);
		ValidatedObject::swap(other);
		attrs.swap(other.attrs);
		STD_::swap(bPrepare, other.bPrepare);
	}

	// return reference to the connection associated with this stmt.
	DBConnection &DBStmt::GetConnection() const { return *pConn; }

	// set stmt. to refer to a different existing connection
	void DBStmt::SetConnection(const DBConnection &conn)
	{
	   // must destroy old HSTMT as it will now be invalid
	   // due to new connection
       try
	   {
		   Destroy();
	   }
	   catch (...)
	   {

	   }

	   DBConnection &connxn = const_cast<DBConnection &>(conn);
	   pConn = &connxn;
	}

	// Destroy() may have tricky destruction semantics
	DBStmt::~DBStmt()
	{
	   try
	   {
		   Destroy();
	   }
	   catch (...)
	   {

	   }
	}


	// is the stmt. ready for execution?
	bool DBStmt::IsReady() const { return (state == STMT_PREPARED || state == STMT_EXECUTED)
									&& valid(); }

	// does the stmt. refer to an allocated handle?
	bool DBStmt::IsAllocated() const { return (state != STMT_UNALLOCATED) && valid(); }

	bool DBStmt::IsExecuted() const { return state == STMT_EXECUTED;}

    // check the state of the object against what we know about the hstmt
	bool DBStmt::valid() const
	{
		// if we know the object is invalid from a previous error,
		// object is still invalid
		if (!ValidatedObject::valid())
			return false;

		// we know the object is in an invalid state
		// if the hstmt is not null and the state of the object says the statement has
		// not been allocated, then the object is in an inconsistent state
		// the same is true if the hstmt is null and the state of the object says
		// the statement has been allocated
	
		// if the handle and state are in agreement (both are nonzero or both are zero)
		// we're ok
		// so the negation of an XOR will give us whether the object is valid or not
		// as we need a *logical* XOR, we have to emulate it
		bool stmtValid = !((hstmt || state) && !(hstmt && state));

		// don't forget to invalidate the object if the statement is found invalid
		if (!stmtValid)
			const_cast<DBStmt *>(this)->invalidate();

		return stmtValid;
	}

	// get a statement ready for execution
	void DBStmt::Initialize()
	{
	   if (!IsReady() && valid())
	   {
		    // try to allocate stmt.
			RETCODE rc = SQLAllocStmt(pConn->GetHDBC(), &hstmt);

			if (!RC_SUCCESS(rc))
			{
				tstring errmsg;
				errmsg.reserve(1024);
				errmsg += _TEXT("Unable to allocate statement handle for \"");
				errmsg += sqlQuery;
				errmsg += _TEXT("\"");

				invalidate();

				// making sure we're atomic and consistent

				state = STMT_UNALLOCATED;
				hstmt = SQL_NULL_HSTMT;

				throw DBException(_TEXT("DBStmt::Initialize()"), errmsg, pConn, this);
			}

			
			// stmt. successfully allocated ... next try to prepare stmt.
			state = STMT_ALLOCATED;

			// user-set attributes override the defaults we've set above
			LoadAttributes();

			if (bPrepare)
				Prepare();		

			state = STMT_PREPARED;

	   }
	}

	// Prepare a statement for repeated execution
	void DBStmt::Prepare()
	{
			assert(hstmt != SQL_NULL_HSTMT);

			TCHAR *c_str = new TCHAR[sqlQuery.length() + 1];
			RETCODE rc;
			if (c_str != NULL) {
				std_tstrncpy(c_str, sqlQuery.c_str(), sqlQuery.length() + 1);
				rc = SQLPrepare(hstmt, (SQLTCHAR *)c_str, SQL_NTS);
				delete[] c_str;
			}
			else {
				invalidate();
				Destroy();
				throw DBException(_TEXT("DBStmt::Execute()"), _TEXT("Unable to allocate memory for query"), pConn, this);
			}

			if (!RC_SUCCESS(rc))
			{
				tstring errmsg = _TEXT("Unable to prepare statement handle for \"");
				errmsg += sqlQuery;
				errmsg += _TEXT("\"");

				// N.B.!! it is very important to capture the SQLError messages
				// here before we free the handle
				DBException dbe(_TEXT("DBStmt::Initialize()"), errmsg, pConn, this);

				invalidate();

				Destroy();

				throw dbe;
			}
	}

	// Call the ODBC MoreResults() function to see if another set of results exists
	// for this statement.
	// Example uses would be multiple select queries in a single statement e.g.
	// _TEXT("SELECT Col1 from Table1; Select Col2 from Table2")
	// or stored proceedures that return multiple result sets
	// See SQLMoreResults in the ODBC docs for details	
	bool DBStmt::MoreResults() {
		validate();
		
		assert(hstmt != SQL_NULL_HSTMT);

		RETCODE rc = SQLMoreResults(hstmt); 
        if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) // more results available (either stored procedure or batch SQL statement)
			return true;

		// no more results, clear the EXECUTED state
		state = STMT_PREPARED;
  
		if (rc == SQL_ERROR)
		{ 
		  tstring errmsg;
		  errmsg.reserve(1024);
		  errmsg += _TEXT("Error when calling SQLMoreResults() \"");
		  errmsg += sqlQuery;
		  errmsg += _TEXT("\"");

		  const DBException dbe(_TEXT("DBStmt::MoreResults()"), errmsg, pConn, this);

		  // gets the actual ODBC error tstring
		  STD_::pair<tstring, tstring> info = dbe.GetODBCError();
		  const tstring szSQLError = info.first;

		  // if error is fatal, invalidate the DBStmt
		  if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
			  szSQLError) != FatalErrors.end())
			  invalidate();

		  throw dbe;
		}
		

		return false;
	}

	// execute the stmt.
	DBStmt::NeedDataStatus DBStmt::Execute()
	{

	  validate();

      if (!IsReady() && valid())
		Initialize();

#ifdef DTL_MEM_DEBUG
	  STD_::vector<mcheck>::iterator it;
 	  for(it = vp.begin(); it != vp.end(); it++) 
		tcout << it->ptr << _TEXT(", ");
	  tcout << STD_::endl;
#endif

	  RETCODE rc; 

	  assert(hstmt != SQL_NULL_HSTMT);

	  if (!bPrepare) {		
		  TCHAR *c_str = new TCHAR[sqlQuery.length() + 1];
		  if (c_str != NULL) {
			std_tstrncpy(c_str, sqlQuery.c_str(), sqlQuery.length() + 1);
			rc = SQLExecDirect(hstmt, (SQLTCHAR *)c_str, SQL_NTS);
			delete[] c_str;
		  }
		  else {
			invalidate();
			Destroy();
			throw DBException(_TEXT("DBStmt::Execute()"), _TEXT("Unable to allocate memory for query"), pConn, this);
		  }
	  }
	  else
		rc = SQLExecute(hstmt);


	  if (!RC_SUCCESS(rc) && rc != SQL_NO_DATA && rc != SQL_NEED_DATA)
	  { 
		  tstring errmsg;
		  errmsg.reserve(1024);
		  errmsg += _TEXT("Unable to execute statement \"");
		  errmsg += sqlQuery;
		  errmsg += _TEXT("\"");

		  const DBException dbe(_TEXT("DBStmt::Execute()"), errmsg, pConn, this);

		  // gets the actual ODBC error tstring
		  STD_::pair<tstring, tstring> info = dbe.GetODBCError();
		  const tstring szSQLError = info.first;

		  // if error is fatal, invalidate the DBStmt
		  if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
			  szSQLError) != FatalErrors.end())
			  invalidate();

		  throw dbe;
  
		  // destructor should free statement
	  }

  	  state = STMT_EXECUTED;

	  return rc;
	}


        // for SELECT's only ... fetch next row from recordset	
	DBStmt::FetchStatus DBStmt::Fetch()
	{
		validate();
		if (state != STMT_EXECUTED) 
			Execute();

		assert(hstmt != SQL_NULL_HSTMT);
#ifdef DTL_MEM_DEBUG
		STD_::vector<mcheck>::iterator it;
		bool b_check;
		tcout << _TEXT("fetch: "); // comment out for less verbosity
		for(it = vc.begin(); it != vc.end(); it++) {
			b_check = it->validate();
			tcout << it->ptr << _TEXT(" - ") << (b_check ? _TEXT("T") : _TEXT("F")); // comment out for less verbosity
			if (!it->validate())
			{   
				tcout <<_TEXT(" **** potentially corrupted!!!! ****");
			}

			tcout << _TEXT(", ");
		}
		tcout << STD_::endl; // comment out for less verbosity
#endif

		RETCODE rc = SQLFetch(hstmt);

#ifdef DTL_MEM_DEBUG
		for(it = vc.begin(); it != vc.end(); it++) 
			it->revalidate();
#endif

		switch (rc)
		{
		   // fetch went OK
		   case SQL_SUCCESS : case SQL_SUCCESS_WITH_INFO: return ROW_FETCHED;

		   // no data returned, reached end of recordset ... this is also OK
		   case SQL_NO_DATA :
			   return NO_DATA_FETCHED;

		   // all other scenarios - an error occurred
		   default:
			{			 
				
		      tstring errmsg;
			  errmsg.reserve(1024);
			  errmsg += _TEXT("Unable to fetch statment \"");
			  errmsg += sqlQuery;
			  errmsg += _TEXT("\"");

			  const DBException dbe(_TEXT("DBStmt::Fetch()"), errmsg, pConn, this);
			  
			  // gets the actual ODBC error tstring
			  STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			  const tstring szSQLError = info.first;
		  
			  // if error is fatal, invalidate the DBStmt
			  if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			  throw dbe;
			}
		}
		return NO_DATA_FETCHED;
	}

	// for SELECT's only ... fetch next block of rows from recordset	
	DBStmt::FetchStatus DBStmt::FetchScroll(SQLSMALLINT FetchOrientation,
SQLINTEGER FetchOffset)
	{
		validate();
		if (state != STMT_EXECUTED) 
			Execute();

		assert(hstmt != SQL_NULL_HSTMT);
#ifdef DTL_MEM_DEBUG
		STD_::vector<mcheck>::iterator it;
		bool b_check;
		tcout << _TEXT("fetch: "); // comment out for less verbosity
		for(it = vc.begin(); it != vc.end(); it++) {
			b_check = it->validate();
			tcout << it->ptr << _TEXT(" - ") << (b_check ? _TEXT("T") : _TEXT("F")); // comment out for less verbosity
			if (!it->validate())
			{   
				tcout <<_TEXT(" **** potentially corrupted!!!! ****");
			}

			tcout << _TEXT(", ");
		}
		tcout << STD_::endl; // comment out for less verbosity
#endif

		RETCODE rc = SQLFetchScroll(hstmt, FetchOrientation, FetchOffset);

#ifdef DTL_MEM_DEBUG
		for(it = vc.begin(); it != vc.end(); it++) 
			it->revalidate();
#endif

		switch (rc)
		{
		   // fetch went OK
		   case SQL_SUCCESS : case SQL_SUCCESS_WITH_INFO: return ROW_FETCHED;

		   // no data returned, reached end of recordset ... this is also OK
		   case SQL_NO_DATA :
			   return NO_DATA_FETCHED;

		   // all other scenarios - an error occurred
		   default:
			{			 
				
		      tstring errmsg;
			  errmsg.reserve(1024);
			  errmsg += _TEXT("Unable to fetch statement \"");
			  errmsg += sqlQuery;
			  errmsg += _TEXT("\"");

			  const DBException dbe(_TEXT("DBStmt::FetchScroll()"), errmsg, pConn, this);
			  
			  // gets the actual ODBC error tstring
			  STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			  const tstring szSQLError = info.first;
		  
			  // if error is fatal, invalidate the DBStmt
			  if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			  throw dbe;
			}
		}
		return NO_DATA_FETCHED;
	}

	void DBStmt::SetPos(SQLUSMALLINT RowNumber, SQLUSMALLINT Operation, SQLUSMALLINT LockType) {
		assert(hstmt != SQL_NULL_HSTMT);
		RETCODE rc = SQLSetPos(hstmt, RowNumber, Operation, LockType);
		if (!RC_SUCCESS(rc)) {
			  tstring errmsg;
			  errmsg.reserve(1024);
			  errmsg += _TEXT("Unable to execute SQLSetPos \"");
			  errmsg += sqlQuery;
			  errmsg += _TEXT("\"");

			  const DBException dbe(_TEXT("DBStmt::SetPos()"), errmsg, pConn, this);
			  
			  // gets the actual ODBC error tstring
			  STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			  const tstring szSQLError = info.first;
			   			    
			  throw dbe;

		}

	}

	// destroy this hstmt. ... must use if done with the hstmt completely or the
	// underlying SQL tstring is changed
	// (can always reuse the hstmt with the same query)

	// tricky destructor issues here!
	void DBStmt::Destroy()
	{
		   // state may be only partly valid() partial destructions allowed below

	       if (state != STMT_UNALLOCATED && hstmt != SQL_NULL_HSTMT)
		   {	
			 RETCODE rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			 
			 if (!RC_SUCCESS(rc))
			 {
				invalidate();

				tstring errmsg = _TEXT("Unable to free statement handle for \"");
				errmsg += sqlQuery;
				errmsg += _TEXT("\"");

				// these two lines needed to provide local atomicity and
				// consistency for exception safety
				state = STMT_UNALLOCATED;
				hstmt = SQL_NULL_HSTMT;
				
				throw DBException(_TEXT("DBStmt::Destroy()"), errmsg, pConn, this);
			 }
			 else
			 {
				hstmt = SQL_NULL_HSTMT;
			 }
 	       }
		
	       state = STMT_UNALLOCATED;

	}

	// frees current statement handle and allocates and prepares a new one
	void DBStmt::ReInitialize()
	{
	     try
	     {
		   Destroy();
	     }
	     catch (...)
	     {

	     }
	     Initialize();
	}

	// reexecute the statement ... should only be used on a select stmt.
	void DBStmt::ReExecute()
	{
	      Reset();
	      Execute();
	}

	// close the stmt. so we can reexecute it ... use on a SELECT stmt. to reset the
	// cursor to the top of the recordset
	// we require this to be a no-throw operation
	void DBStmt::Reset()
	{
		   if (state == STMT_EXECUTED  && valid()) {
		     /* RETCODE rc = */ SQLCloseCursor(hstmt);
			   state = STMT_PREPARED;
		   }
	}

	void DBStmt::CloseCursor()
	{
		   if (state == STMT_EXECUTED  && valid()) {
			   assert(hstmt != SQL_NULL_HSTMT);
		     /* RETCODE rc = */ SQLCloseCursor(hstmt);
			   state = STMT_PREPARED;
		   }
	}

	// clear paramter bindings
	void DBStmt::UnbindParams()
	{
		validate();

		if (IsAllocated())
		{	
			assert(hstmt != SQL_NULL_HSTMT);
			RETCODE rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
			if (!RC_SUCCESS(rc))
				throw DBException(_TEXT("DBStmt::UnbindParams()"),
					_TEXT("Unable to unbind params for stmt '") + sqlQuery + _TEXT("'!"),
					pConn, this);
			// Destroy();	
		}
	}

	// clear column bindings
	void DBStmt::UnbindCols()
	{
		validate();

		if (IsAllocated())
		{
			assert(hstmt != SQL_NULL_HSTMT);
			RETCODE rc = SQLFreeStmt(hstmt, SQL_UNBIND);
			if (!RC_SUCCESS(rc))
				throw DBException(_TEXT("DBStmt::UnbindParams()"),
					_TEXT("Unable to unbind params for stmt '") + sqlQuery + _TEXT("'!"),
					pConn, this);
			// Destroy();
		}
	}

	// form of reset which accepts a new query tstring for the stmt.
	// must destroy and realloc the hstmt in order to bind
	// query tstring to stmt.
	void DBStmt::ResetWithNewQuery(tstring qry)
	{
	       try
		   {
			   Destroy();
		   }
		   catch (...)
		   {


		   }

	       // set the new query tstring before we reinitialize
	       // as Initialize does the SQLPrepare()
	       sqlQuery = qry; // atomicity not guaranteed (but consistency maintained)
						   // if tstring asssigment fails

	}

	// bind a parameter for the statement
	void DBStmt::BindParam(SDWORD ParameterNumber, SDWORD InputOutputType,
					  SDWORD ValueType,	SDWORD ParameterType,
					  SDWORD ColumnSize, SDWORD DecimalDigits,
					  void *ParameterValuePtr, SDWORD BufferLength,
					  SDWORD *StrLen_or_IndPtr)
	{
		  validate();

	      if (!IsReady() && valid())
	          Initialize();

		  assert(hstmt != SQL_NULL_HSTMT);

		  RETCODE rc = SQLBindParameter((SQLHSTMT) hstmt,
			    (SQLUSMALLINT) ParameterNumber + 1,
			    (SQLSMALLINT) InputOutputType, (SQLSMALLINT) ValueType,
				(SQLSMALLINT) ParameterType, (SQLUINTEGER) ColumnSize,
				(SQLSMALLINT) DecimalDigits, (SQLPOINTER) ParameterValuePtr, 
				(SQLINTEGER) BufferLength, (SQLINTEGER *) StrLen_or_IndPtr);

	      if (!RC_SUCCESS(rc))
		  {
			tstring errmsg = _TEXT("Parameter binding failed for parameter #");

			// get parameter number
			tostringstream errstr;
			errstr << errmsg;

			errstr << ParameterNumber << _TEXT(" in statement \"") << sqlQuery << _TEXT("\"!  ");
			errstr << STD_::endl << _TEXT("(You may want to double-check that you have numbered ")
				_TEXT("your parameters correctly - depending on the SQL error you are getting.)") << STD_::ends;
			const DBException dbe(_TEXT("DBStmt::BindParam()"), errstr.str(), pConn, this);


			// errstr.freeze(false);

			// gets the actual ODBC error tstring
			STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			const tstring szSQLError = info.first;
		  
			// if error is fatal, invalidate the DBStmt
			if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			throw dbe;
		  }

#ifdef DTL_MEM_DEBUG	  
		  vp.push_back(mcheck(ParameterValuePtr, BufferLength));
		  vc.push_back(mcheck(StrLen_or_IndPtr, sizeof(SDWORD)));
#endif
	}

	// bind a column for the statement
	void DBStmt::BindCol(SDWORD ColumnNumber, SDWORD TargetType,
					void *TargetValuePtr, SDWORD BufferLength,
					SDWORD *StrLen_or_IndPtr)
	{
		  validate();

	      if (!IsReady())
			Initialize();

//		  tcout << _TEXT("Binding column ") << ColumnNumber << STD_::endl;

		  assert(hstmt != SQL_NULL_HSTMT);

	      RETCODE rc = SQLBindCol((SQLHSTMT) hstmt,
			    (SQLUSMALLINT) ColumnNumber + 1,
			    (SQLSMALLINT) TargetType, (SQLPOINTER) TargetValuePtr, 
				(SQLINTEGER) BufferLength, (SQLINTEGER *) StrLen_or_IndPtr);

	      if (!RC_SUCCESS(rc))
		  {
			tstring errmsg = _TEXT("Column binding failed for parameter #");

			// get parameter number
			tostringstream errstr;
			errstr << errmsg;

			errstr << ColumnNumber << _TEXT(" in statement \"") << sqlQuery << _TEXT("\"!  ");
			errstr << STD_::endl << _TEXT("(Remember: Column numbers are zero-index based!)");
			errstr << _TEXT("DB expected type incompatible with C++ type!") << STD_::ends;

			const DBException dbe(_TEXT("DBStmt::BindCol()"), errstr.str(), pConn, this);

			// errstr.freeze(false);

			// gets the actual ODBC error tstring
			STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			const tstring szSQLError = info.first;
		  
			// if error is fatal, invalidate the DBStmt
			if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			throw dbe;
		  }
		  
#ifdef DTL_MEM_DEBUG	  
		  vc.push_back(mcheck(TargetValuePtr, BufferLength));
		  vc.push_back(mcheck(StrLen_or_IndPtr, sizeof(SDWORD)));		  
#endif
	}

	// get data for a field
	DBStmt::FetchStatus DBStmt::GetData(SDWORD ColumnNumber, SDWORD TargetType,
					void *TargetValuePtr, SDWORD BufferLength,
					SDWORD *StrLen_or_IndPtr)
	{
		  validate();

	      if (!IsReady())
			Initialize();

//		  tcout << _TEXT("GetData() for column ") << ColumnNumber << STD_::endl;

		  assert(hstmt != SQL_NULL_HSTMT);

	      RETCODE rc = SQLGetData((SQLHSTMT) hstmt,
			    (SQLUSMALLINT) ColumnNumber + 1,
			    (SQLSMALLINT) TargetType, (SQLPOINTER) TargetValuePtr, 
				(SQLINTEGER) BufferLength, (SQLINTEGER *) StrLen_or_IndPtr);

	      if (!RC_SUCCESS(rc)  && rc != SQL_NO_DATA)
		  {
			tstring errmsg = _TEXT("GetData failed for column #");

			// get parameter number
			tostringstream errstr;
			errstr << errmsg;

			errstr << ColumnNumber << _TEXT(" in statement \"") << sqlQuery << _TEXT("\"!  ");
			errstr << STD_::endl << _TEXT("(Remember: Column numbers are zero-index based!)");
			errstr << _TEXT("DB expected type incompatible with C++ type!") << STD_::ends;

			const DBException dbe(_TEXT("DBStmt::GetData()"), errstr.str(), pConn, this);

			// errstr.freeze(false);

			// gets the actual ODBC error tstring
			STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			const tstring szSQLError = info.first;
		  
			// if error is fatal, invalidate the DBStmt
			if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			throw dbe;
		  }
		  
		if (rc == SQL_NO_DATA || *StrLen_or_IndPtr < 1)
			return DBStmt::NO_DATA_FETCHED;
		else
			return DBStmt::ROW_FETCHED;
	}

	// put data for a field
	void DBStmt::PutData(void *DataPtr, SDWORD StrLen_or_Ind)
	{
		validate();

		if (!IsReady())
			Initialize();

		assert(hstmt != SQL_NULL_HSTMT);

		RETCODE rc = SQLPutData((SQLHSTMT) hstmt, (SQLPOINTER) DataPtr, 
						(SQLINTEGER) StrLen_or_Ind);

		if (!RC_SUCCESS(rc))
		{
			tstring errmsg = _TEXT("SQLPutData() call failed for DataPtr ");

			// get exception info.
			tostringstream errstr;
			errstr << errmsg;

			errstr << DataPtr << _TEXT(" in statement \"") << sqlQuery << _TEXT("\"!  ");
			errstr << STD_::endl << _TEXT("(You may want to double-check that you have numbered ")
				_TEXT("your parameters correctly - depending on the SQL error you are getting.)") << STD_::ends;
			const DBException dbe(_TEXT("DBStmt::PutData()"), errstr.str(), pConn, this);


			// errstr.freeze(false);

			// gets the actual ODBC error tstring
			STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			const tstring szSQLError = info.first;
		  
			// if error is fatal, invalidate the DBStmt
			if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			throw dbe;		

		}

	}

	// see which parameter that system is requesting next for PutData()
	DBStmt::NeedDataStatus DBStmt::ParamData(void **DataPtrPtr)
	{
		validate();

		if (!IsReady())
			Initialize();

		assert(hstmt != SQL_NULL_HSTMT);

		RETCODE rc = SQLParamData((SQLHSTMT) hstmt, (SQLPOINTER *) DataPtrPtr);

		if (!RC_SUCCESS(rc) && (rc != SQL_NEED_DATA) && (rc != SQL_NO_DATA))
		{
			tstring errmsg = _TEXT("SQLParamData() call failed");

			// get exception info.
			tostringstream errstr;
			errstr << errmsg;

			errstr << _TEXT(" in statement \"") << sqlQuery << _TEXT("\"!  ");
			errstr << STD_::endl << _TEXT("(You may want to double-check that you have numbered ")
				_TEXT("your parameters correctly - depending on the SQL error you are getting.)") << STD_::ends;
			const DBException dbe(_TEXT("DBStmt::ParamData()"), errstr.str(), pConn, this);


			// errstr.freeze(false);

			// gets the actual ODBC error tstring
			STD_::pair<tstring, tstring> info = dbe.GetODBCError();
			const tstring szSQLError = info.first;
		  
			// if error is fatal, invalidate the DBStmt
			if (STD_::find(FatalErrors.begin(), FatalErrors.end(),
				  szSQLError) != FatalErrors.end())
				  invalidate();
			   			    
			throw dbe;		

		}

		return (rc);
	}

	// return number of affected rows ... used by UPDATE and DELETE
	long DBStmt::RowCount() const
	{
		validate();
		if (!IsReady() && valid())
			const_cast<DBStmt *>(this)->Initialize();

		SQLINTEGER result;

		assert(hstmt != SQL_NULL_HSTMT);

		RETCODE rc = SQLRowCount(hstmt, &result);
		
		if (!RC_SUCCESS(rc))
		{
		   const_cast<DBStmt *>(this)->invalidate();

		   tstring errmsg;
		   errmsg.reserve(1024);
		   errmsg += _TEXT("Unable to get row count for statement \"");
		   errmsg += sqlQuery;
		   errmsg += _TEXT("\"");

		   throw DBException(_TEXT("DBStmt::RowCount()"),
							 errmsg, pConn, this);
		}

		return result;
	}

	// get the query tstring for this statement
	tstring DBStmt::GetQuery() const
	{
	     return sqlQuery;
	}

	void DBStmt::SetQuery(const tstring &s) {
		sqlQuery = s;
	}

	void DBStmt::BulkOperations(SQLUSMALLINT Operation){

	   assert(hstmt != SQL_NULL_HSTMT);
	   
	   RETCODE rc = SQLBulkOperations(hstmt, Operation);

	   if (!RC_SUCCESS(rc))
		   throw DBException(_TEXT("DBStmt::BulkOperations()"),
							 _TEXT("Unable to perform bulk operation for statement \"") + sqlQuery +
							 _TEXT("\"!"), pConn, this);
	}

	void DBStmt::GetStmtAttr(SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER BufferLength, 
		SQLINTEGER* StringLengthPointer)  {

		RETCODE rc;
		rc = SQLGetStmtAttr(hstmt, Attribute, ValuePtr, BufferLength, StringLengthPointer);
			
		if (!RC_SUCCESS(rc))
		{
                tostringstream tostr;
                tostr << _TEXT("Attribute: ") << Attribute 
					  << _TEXT(" Value: ") << ValuePtr;

				tstring toString = tostr.str();

				throw DBException(_TEXT("DBStmt::GetStmtAttr()"),
							 _TEXT("Unable to get statement attribute for ") +
                                                         toString + _TEXT(" statement \"") + sqlQuery +
							 _TEXT("\"!"), pConn, this);
		}

	}

	// set statement attributes ... will override the default behavior in DTL

	// must cache the attributes we want to set because the hstmt must already exist
	// for us to set statement attributes, which only occurs when the statement gets initialized
	void DBStmt::SetStmtAttr(SQLINTEGER Attribute, SDWORD Value, SQLINTEGER StringLength)
	{
	   Attr attr(Attribute, Value, StringLength);

	   // erase the old value of the attribute if it was previously set
	   STD_::set<Attr>::iterator find_it = attrs.find(attr);

	   if (find_it != attrs.end())
	   {
			attrs.erase(find_it);  
	   }
	   
	   // now insert the current value
	   attrs.insert(attr);
	}		

	void DBStmt::ClearStmtAttrs()
	{
	   attrs.clear();
	}

	// sets the attributes in ODBC that the user set in the DBStmt object
	void DBStmt::LoadAttributes()
	{
		RETCODE rc = SQL_SUCCESS;

		assert(hstmt != SQL_NULL_HSTMT);

		for (STD_::set<Attr>::iterator attr_it = attrs.begin();
					attr_it != attrs.end(); ++attr_it)
		{
			rc = SQLSetStmtAttr(hstmt, attr_it->attr, (SQLPOINTER) attr_it->value, attr_it->length);
			
			if (!RC_SUCCESS(rc))
			{
                                tostringstream tostr;
                                tostr << _TEXT("Attribute: ") << attr_it->attr 
									  << _TEXT(" Value: ") << attr_it->value;

								tstring toString = tostr.str();

				throw DBException(_TEXT("DBStmt::LoadAttributes()"),
							 _TEXT("Unable to set statement attribute for ") +
                                                         toString + _TEXT(" statement \"") + sqlQuery +
							 _TEXT("\"!"), pConn, this);
			}
		}
	}

	// get primary keys for the table passed in
	STD_::vector<tstring> DBStmt::GetPrimaryKeys(const tstring &tableNm, 
		DBConnection &conn)
	{
		STD_::vector<tstring> primary_keys;

		tstring tableNmNarrow = 
			tstring_cast((tstring *) NULL, tableNm);     /* Table to display   */

		TCHAR *szTable;

		// Unused variable:
		// const size_t TAB_LEN = SQL_MAX_TABLE_NAME_LEN + 1;
		const size_t COL_LEN = SQL_MAX_COLUMN_NAME_LEN + 1;

		TCHAR   szPkCol[COL_LEN];  /* Primary key column   */
		
		SQLHSTMT         hstmt;
		SQLINTEGER      cbPkCol;
		
		SQLRETURN         retcode;

		retcode = SQLAllocStmt(conn.GetHDBC(), &hstmt);

		if (!RC_SUCCESS(retcode))
			return primary_keys;

	    /* Find the column names that can best give a unique identifier for the row */
#ifdef  _UNICODE
	     retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szPkCol, COL_LEN, &cbPkCol);
#else
	     retcode = SQLBindCol(hstmt, 2, SQL_C_CHAR, szPkCol, COL_LEN, &cbPkCol);
#endif		
		if (!RC_SUCCESS(retcode))
		{
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return primary_keys;
		}

		szTable = (TCHAR *) tableNmNarrow.c_str();

		/* Get the names of the columns in the primary key.      */


		// Minimum ROWID, key may become invalid when we move off the current row
		// NB!! When we go to scrollable recordsets we *cannot* depend on this rowid
		// staying valid when move to another row.
		// Also note that key fields may hold NULL values
		retcode = SQLSpecialColumns(hstmt,
			SQL_BEST_ROWID, NULL, 0,     /* Catalog name */
			NULL, 0,     /* Schema name   */
			(SQLTCHAR *)szTable, SQL_NTS, SQL_SCOPE_CURROW, SQL_NULLABLE); /* Table name    */

		if (!RC_SUCCESS(retcode)) {
			SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
			return primary_keys;
		}

		retcode = SQLFetch(hstmt);
		while (RC_SUCCESS(retcode)) {				
			tstring keyNm = (const TCHAR *) szPkCol;
			primary_keys.push_back(tstring_cast((tstring *) NULL, keyNm));
				
			/* Fetch and display the result set. This will be a list of the */
			/* columns in the primary key of the table.   */

			retcode = SQLFetch(hstmt);
		}

		// SQLFreeStmt(hstmt, SQL_CLOSE);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

		return primary_keys;
}

END_DTL_NAMESPACE
