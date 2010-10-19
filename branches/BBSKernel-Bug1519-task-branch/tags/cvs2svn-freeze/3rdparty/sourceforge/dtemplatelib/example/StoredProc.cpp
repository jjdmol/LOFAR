#include "StoredProc.h"


// Read the contents of a table and print the resulting rows
void StoredProcRead() {

	DBView<variant_row> view("{call Employee_Pkg.GetEmployeeDetails(?, ?, ?, ?, ?)}", GetEmployeeDetailsBCA(), exampleOrderBy);



	// NOTE: We need to construct r from the view itself since we
	// don't know what fields the table will contain.
	// We therefore make a call to the DataObj() function to have the
	// table return us a template row with the correct number of fields
	// and field types.
	// We use this construction since we can't be guaranteed that the table
	// is non-empty & we want to still display column names in this case.
	variant_row s(view.GetDataObj());

	// Print out the column names
	vector<string> colNames = s.GetNames();
	for (vector<string>::iterator name_it = colNames.begin(); name_it != colNames.end(); ++name_it)
		cout << (*name_it) << " ";
	cout << endl;

	// Print out all rows and columns from our query
	DBView<variant_row>::sql_iterator print_it = view.begin();

	static_cast<variant_row>(*print_it)[0] = 1;
	for (++print_it; print_it != view.end(); ++print_it) {
		for (size_t i = 0; i < static_cast<variant_row>(*print_it).size(); ++i)
		 		cout << static_cast<variant_row>(*print_it)[i] << " ";
		cout << endl;
	}
	
}




// Oracle stored procedure we wish to test
#if 0
 Create or replace package ExampleInfo as

   Type ExampleRec is record

   (
   INT_VALUE     integer,

   STRING_VALUE   varchar2(50)

   );

   Type ExampleCursor is ref cursor return 

   ExampleRec;

   End ExampleInfo;

   

   Create or replace procedure ExampleInfoProc

   (LONG_CRITERIA IN integer, empcursor IN OUT 

   ExampleInfo.ExampleCursor)

   As

   Begin

   Open empcursor For

   select INT_VALUE, STRING_VALUE 

   from db_example

   where EXAMPLE_LONG = LONG_CRITERIA;

   End;
#endif

// Read the contents of a table and print the resulting rows
// *** you must have Oracle ODBC driver version 8.1.5.3.0 for this to work ***
void StoredProcRead2() {

#if 1
	DBView<variant_row, ProcParams> view("{call ExampleInfoProc(?)}", 
		ProcBCA(), "ORDER BY INT_VALUE, STRING_VALUE", ProcBPA());
#endif

#if 0
	DynamicDBView<> view(
		"{call Employee_Pkg.GetEmployeeList({resultset 100, o_emID, o_FirstName, o_LastName})}", "");
#endif

	// NOTE: We need to construct r from the view itself since we
	// don't know what fields the table will contain.
	// We therefore make a call to the DataObj() function to have the
	// table return us a template row with the correct number of fields
	// and field types.
	// We use this construction since we can't be guaranteed that the table
	// is non-empty & we want to still display column names in this case.
	#if 0
	variant_row s(view.GetDataObj());


	// Print out the column names
	vector<string> colNames = s.GetNames();
	for (vector<string>::iterator name_it = colNames.begin(); name_it != colNames.end(); ++name_it)
		cout << (*name_it) << " ";
	cout << endl;

#endif

	// Print out all rows and columns from our query
	DBView<variant_row, ProcParams>::sql_iterator print_it = view.begin();
	print_it.Params().long_criteria = 22;

	cout << "Params:" << print_it.Params().long_criteria << endl;

	for (; print_it != view.end(); ++print_it)
	{
		 variant_row r = *print_it;
		 for (size_t i = 0; i < r.size(); ++i)
		 	cout << r[i] << " ";
		 cout << endl;
	}
}

// Oracle stored procedure we wish to test
#if 0
   Create or replace procedure ExampleInfoProcOutParams
   (LONG_CRITERIA IN integer, NUM_RECORDS OUT integer)

   As

   Begin

   select count(*)

   into NUM_RECORDS

   from db_example

   where EXAMPLE_LONG = LONG_CRITERIA;

   End;
#endif

// simply does a select count(*) from db_example where example_long = 22
// version that will test output params
// *** you must have Oracle ODBC driver version 8.1.5.3.0 for this to work ***
void StoredProcRead3() {

	DBView<EmptyDataObj, ProcOutParams> view("{call ExampleInfoProcOutParams(?, ?)}", 
		ProcOutBCA(), "", ProcOutBPA());

	// execute our stored procedure
	DBView<EmptyDataObj, ProcOutParams>::sql_iterator print_it = view.begin();
	
	print_it.Params().long_criteria = 22;

	*print_it = EmptyDataObj(); // must assign to *print_it to force the statement to execute
								// and get print_it.Params() to contain the expected result

	++print_it; 

	cout << "number of records with EXAMPLE_LONG = 22 is " 
		 << print_it.Params().numRecords << endl;
}


#if 0
DROP PROCEDURE TestParm 

-- Example procedure returns three things:
-- 1. A set of records from the select statement: "SELECT STRING_VALUE FROM DB_EXAMPLE"
-- After all records have been retrieved, output paramenters are returned:
-- 2. OutParm
-- 3. Return value for function
CREATE PROCEDURE TestParm @OutParm int OUTPUT AS
SELECT STRING_VALUE FROM db_example
SELECT @OutParm = 66
RETURN 99


DECLARE @RetVal INT

DECLARE @Param INT

  

-- Execute the procedure, which returns

-- the result set from the first SELECT.

EXEC @RetVal = TestParm @OutParm = @Param OUTPUT

  

-- Use the return code and output parameter.

PRINT 'The return value from the procedure was: ' +

                 CONVERT(CHAR(6), @RetVal)

PRINT 'The value of the output parameter in the procedure was: ' +

                 CONVERT(CHAR(6), @Param)

#endif


void TestParmCreate() 
{
	try{
		DBStmt("DROP PROCEDURE TestParm").Execute();
	}
	catch (std::exception)
	{
	}

	DBStmt(
		"CREATE PROCEDURE TestParm @OutParm int OUTPUT AS "
	    "SELECT STRING_VALUE FROM db_example "
		"SELECT @OutParm = 66 "
		"RETURN 99"
	).Execute();

}

class TestParmBCA {
public:
 void operator()(BoundIOs &cols, variant_row &row)
 {
  cols["STRING_VALUE"] == row._string();
  cols[0] >> row._int();
  cols[1] == row._int();

  cols.BindVariantRow(row);
 }
};


#if 0

class TestParmBPA {
public:
	void operator()(dtl::BoundIOs &cols, dtl::variant_row &row)
	{
		cols[0] >> row._int();  //ERROR?  This specifies an output parameter.  Do you want an output parameter, an input parameter, or an i/o parameter
		cols[1] >> row._int();
		cols.BindVariantRow(row);
	}
};
  

void TestMe()
{

DynamicDBView<> view("{? = call TestParm(?)}", "");
// DynamicDBView is really just an alias for DBView<variant_row, variant_row> where the BCA and BPA are built automatically from meta-data.


view.SetBPA(TestParmBPA());
// You will need to add a method to DBView here.  See below

DynamicDBView<>::sql_iterator print_it = view.begin();
DynamicDBView<>::sql_iterator end_it = view.end();

// set params
print_it.Params()[0] =0;
print_it.Params()[1] =0;


// output rows
copy(print_it, end_it, ostream_iterator<variant_row>(cout, "\n"));

// get return values
print_it.MoreResults();
cout <<print_it.Params() << endl;
	
}
#endif



// Read the contents of a table and print the resulting rows
void StoredProcReadTestParm() {
	cout << "SQL Server stored procedure test" << endl;

	TestParmCreate() ;
	DBView<variant_row> view("{? = call TestParm(?)}", TestParmBCA());


	// Print out the column names
	cout << "column names" << endl;
	vector<string> colNames = view.GetColNames();
	for (vector<string>::iterator name_it = colNames.begin(); name_it !=
	  colNames.end(); ++name_it)
		cout << (*name_it) << " ";
	cout << endl;

	// Print out all rows and columns from our query
	DBView<variant_row>::sql_iterator print_it = view;

	variant_row r(view.GetDataObj());
	r["0"] = 0;
	r["1"] = 0;

	*print_it = r; // assign parameters to execute

	for (++print_it; print_it != view.end(); ++print_it)
	{
		cout << (variant_row) *print_it << endl;
	}

	cout << endl;
	cout << "After call to MoreResults(), "
	"SQL-Server gives results in output parameters & return code." << endl;
	print_it.MoreResults();

	r = *print_it;
	cout << "param 0 = " << r["0"] << endl;
	cout << "param 1 = " << r["1"] << endl;

	cout << endl;

}
