// special query example

#ifndef _SPECIALQRYEXAMPLE_H
#define _SPECIALQRYEXAMPLE_H

#include "example_core.h"


// Define an object to hold our row data -- used here for testing custom query
class DistinctExample
{
private:                       // tablename.columnname:
 int exampleInt;               // DB_EXAMPLE.INT_VALUE
 string exampleStr;            // DB_EXAMPLE.STRING_VALUE
 double exampleDouble;         // DB_EXAMPLE.DOUBLE_VALUE
 long exampleLong;             // DB_EXAMPLE.EXAMPLE_LONG
 jtime_c exampleDate;		   // DB_EXAMPLE.EXAMPLE_DATE

public: 

 DistinctExample() : exampleInt(0), exampleStr(""), exampleDouble(0.0),
	 exampleLong(0), exampleDate() { }

 DistinctExample(int exInt, const string &exStr, double exDouble, 
  long exLong, const TIMESTAMP_STRUCT &exDate) :
    exampleInt(exInt), exampleStr(exStr), exampleDouble(exDouble), 
	exampleLong(exLong), exampleDate(exDate){ }

 // only accessors needed by our examples
 void SetExampleStr(const string &str)
 {
   exampleStr = str;
 }

 string GetExampleStr() const
 {
    return exampleStr;
 }

 jtime_c GetExampleDate() const
 {
	return exampleDate;
 }

 void SetExampleDate(const jtime_c &jtime)
 {
	exampleDate = jtime;
 }

 friend class dtl::DefaultBCA<DistinctExample>;

 friend class dtl::DefaultInsValidate<DistinctExample>;

 friend ostream& operator<<(ostream &o, const DistinctExample &ex)
 {
    o << "DistinctExample(" << ex.exampleInt << ", \"" << ex.exampleStr << "\", ";
	o << ex.exampleDouble << ", " << ex.exampleLong << ", ";
	o << ex.exampleDate << ")";

	return o;
 }

 friend bool operator<(const DistinctExample &ex1, const DistinctExample &ex2)
 {
	if (ex1.exampleInt < ex2.exampleInt)
		return true;
	if (ex1.exampleInt > ex2.exampleInt)
		return false;

	if (ex1.exampleStr < ex2.exampleStr)
		return true;
	if (ex1.exampleStr > ex2.exampleStr)
		return false;

	if (ex1.exampleDouble < ex2.exampleDouble)
		return true;
	if (ex1.exampleDouble > ex2.exampleDouble)
		return false;

	if (ex1.exampleLong < ex2.exampleLong)
		return true;
	if (ex1.exampleLong > ex2.exampleLong)
		return false;

	return (ex1.exampleDate < ex2.exampleDate);

 }
};

BEGIN_DTL_NAMESPACE

template<> class DefaultBCA<DistinctExample>
{
public:
 void operator()(BoundIOs &cols, DistinctExample &row)
 {
  cols["INT_VALUE"]    == row.exampleInt;
  cols["STRING_VALUE"] == row.exampleStr;
  cols["DOUBLE_VALUE"] == row.exampleDouble;
  cols["EXAMPLE_LONG"] == row.exampleLong;
  cols["EXAMPLE_DATE"] == row.exampleDate;
 }
};

// Specialization of DefaultInsValidate for Example
// This defines a business rule we wish to enforce for all 
// Example objects before they are allowed to be inserted into the database
template<> class DefaultInsValidate<DistinctExample>
{
public:

	bool operator()(BoundIOs &cols, DistinctExample &rowbuf) {	
		// data is valid if rowbuf.exampleStr is nonempty and
		// rowbuf.exampleDouble is 
		// between 0 and 100 (like a percentage)
		return (rowbuf.exampleStr.length() > 0 &&  rowbuf.exampleDouble >= 0.0 
			&& rowbuf.exampleDouble  <= 100.0);
	}
};
END_DTL_NAMESPACE

BEGIN_DTL_NAMESPACE
// special query builder for DBView<DistinctExample>
template<> class BuildSpecialQry<DistinctExample>
{
public:
  // build the special query
  string operator()(const DBView<DistinctExample> &view, SQLQueryType qryType, 
	  const tstring &QuoteChar)
  {
	switch (qryType)
	{
		case SELECT:
		{
		  string Query;

		  // get the necessary info. we need from the view
		  string postfixClause = view.GetPostfixClause();
		  string rawSQL = view.GetRawSQL();
		  STD_::set<string> tableNames = view.GetTableNames();
	      STD_::vector<string> colNames = view.GetColNames();
		 
		  // SELECT colNames FROM tableNames postfixClause
		  if (tableNames.empty())
			throw DBException("DBView::BuildQry()",
				   "SELECT : must select from at least one table", NULL, NULL);

		  // Catch this error in MakeBindings()
		  // Need to leave this test out so that sql_iterator can work
		  //
		  //if (colNames.empty())
		  //	throw DBException("DBView::BuildQry()",
		  //		   "SELECT : must select from at least one column", NULL, NULL);

		  // build SELECT stmt into Query
		  Query += "SELECT DISTINCT ";

		  // build comma-separated list of columns and tack on to the query
		  Query += MakeDelimitedList(colNames, _TEXT(""), _TEXT(""), _TEXT(", "));

		  Query += " FROM ";

		  // now add a comma-separated list of the tables
		  Query += MakeDelimitedList(tableNames, _TEXT(""), _TEXT(""), _TEXT(", "));

		  // tack on postfix clause
		  if (postfixClause.length() > 0)
		  {
			  Query += " ";
		      Query += postfixClause;
		  }
		  
		  // conceptually read the following as:
		  //		  return Query;
		  // databases have different rules involving cases, so apply it below

		  return QueryCaseify(Query, QuoteChar);

		}
		default:
			return BuildDefaultQry(view, qryType, QuoteChar);
	} // end switch
  } // end operator()

}; // end BuildSpecialQry<DistinctExample>
END_DTL_NAMESPACE

// example for building special queries
void SpecialQryExample();
#endif
