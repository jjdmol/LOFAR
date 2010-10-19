// example for reading data from joined tables

#include "ReadJoinedData.h"

const string joinOrderBy =
	" ORDER BY INT_VALUE, STRING_VALUE, DOUBLE_VALUE, SAMPLE_LONG, EXTRA_FLOAT";

#define USE_SQL_IT_ON_JOIN

// Read JoinExample objects from the database using a query that
// joins the DB_EXAMPLE and DB_SAMPLE tables
vector<JoinExample> ReadJoinedData()
{
 vector<JoinExample> results;

 // construct view
 // note here that we use a custom parameter class for JoinExample
 // rather than DefaultParamObj<JoinExample>

#if 0
 DBView<JoinExample, JoinParamObj> view("DB_EXAMPLE,  DB_SAMPLE", 
    BCAJoinExample(), "WHERE (INT_VALUE = (?) AND STRING_VALUE = (?)) AND "
    "(SAMPLE_INT = (?) OR SAMPLE_STR = (?)) " + joinOrderBy
    BPAJoinParamObj());

  int exampleInt;           // DB_EXAMPLE.INT_VALUE
 string exampleStr;        // DB_EXAMPLE.STRING_VALUE
 double exampleDouble;     // DB_EXAMPLE.DOUBLE_VALUE
 unsigned long sampleLong; // DB_SAMPLE.SAMPLE_LONG
 double extraDouble;       // DB_SAMPLE.EXTRA_FLOAT

#endif

#ifdef USE_SQL_IT_ON_JOIN
	DBView<JoinExample, JoinParamObj> view("SELECT INT_VALUE, DOUBLE_VALUE, "
	"SAMPLE_LONG, EXTRA_FLOAT, STRING_VALUE FROM DB_EXAMPLE, DB_SAMPLE WHERE (INT_VALUE = (?) AND STRING_VALUE = (?)) AND "
    "(SAMPLE_INT = (?) OR SAMPLE_STR = (?)) " + joinOrderBy, BCAJoinExample(), "",
    BPAJoinParamObj());


 // loop through query results and add them to our vector
 DBView<JoinExample, JoinParamObj>::sql_iterator read_it  = view.begin();
#else
 DBView<JoinExample, JoinParamObj> view("DB_EXAMPLE, DB_SAMPLE",
	BCAJoinExample(), "WHERE (INT_VALUE = (?) AND STRING_VALUE = (?)) AND "
    "(SAMPLE_INT = (?) OR SAMPLE_STR = (?)) " + joinOrderBy,
    BPAJoinParamObj());

 // loop through query results and add them to our vector
 DBView<JoinExample, JoinParamObj>::select_iterator read_it  = view.begin();
#endif

 // assign paramteter values as represented by the (?) placeholders
 // in the where clause for our view
 read_it.Params().intValue = 3;
 read_it.Params().strValue = "Join Example";
 read_it.Params().sampleInt = 1;
 read_it.Params().sampleStr = "Joined Tables";

 for ( ; read_it != view.end(); ++read_it)
 { 
  results.push_back(*read_it);
 }

 return results;
}

