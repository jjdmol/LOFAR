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


// Read the contents of a table and print the resulting rows
void StoredProcReadTestParm() {

 DBView<variant_row> view("{? = call TestParm(?)}",
  TestParmBCA());




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
 for (vector<string>::iterator name_it = colNames.begin(); name_it !=
      colNames.end(); ++name_it)
        cout << (*name_it) << " ";
 cout << endl;

 // Print out all rows and columns from our query
 DBView<variant_row>::sql_iterator print_it = view;


 // By default DTL uses server side cursors for SQL Server so that more than
 // one iterator can be active at a time.  This is set in the constructor
 // for DBStmt.
 // Here we require a client side cursor because our stored procedure returns
 // multiple result sets.  Therfore we clear out the setting to use server
 // side cursors in which case SQL server will default to a client side cursor.
 // For details on server side versus client side cursors
 // see "Rowsets and SQL Server cursors, " 
 // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/oledbsql/9_ole_07_212r.asp 
 print_it.GetStmt().ClearStmtAttrs();

 variant_row r = view.GetDataObj();
 r[0] = 0;
 r[1] = 0;

 for (++print_it; print_it != view.end(); ++print_it)
 {
  r = *print_it;
  for (size_t i = 0; i < r.size(); ++i)
    cout << r[i] << " ";
  cout << endl;
 }

 cout << endl;
 cout << "After call to MoreResults(), "
  "SQL-Server gives results in output parameters & return code." << endl;
 print_it.MoreResults();
 r = *print_it;
 for (size_t i = 0; i < r.size(); ++i)
    cout << r[i] << " ";
 cout << endl;

}



