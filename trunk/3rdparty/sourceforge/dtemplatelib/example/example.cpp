#include "example.h"

const TIMESTAMP_STRUCT now = {2000, 12, 15, 0, 0, 0, 0};


// function which calls all example functions in turn
void CallAllExamples(const string &DSN_str)
{
   try
   {
   DBConnection::GetDefaultConnection().Connect(DSN_str);

   // to auto commit, just uncomment the following line
   // DBConnection::GetDefaultConnection().SetAutoCommit(true);

   ResetTables();

   // example of using the DTL_TABLE macro to quickly create a row structure
   // and bind it to a database table
   TableStructExample();

   DBConnection::GetDefaultConnection().CommitAll();

   CStringExample();

   ResetTables();

   // basic table read against a user defined structure
   ReadData();

   // simple example of how to read a set of rows from a table
   // when the columns are not known until runtime
   // (or, if we don't want to create a special struct to hold the rows)
   PrintHeader(std::cout, "SimpleDynamicRead()");

   cout << "Reading objects from a dynamic view:" << endl;

   SimpleDynamicRead();

   DBConnection::GetDefaultConnection().CommitAll();

   PrintSeparator(cout);

   // simple example of how to read/write a set of rows in a table
   // using a random access container
   RandomDBViewExample();
   ResetTables();

   // simple example of how to read/write a set of rows in a table
   // using a random access container versus a DynamicDBView
   RandomDynamicDBView();

   // use a indexed view for the dynamic case
   // as we are working with DB_EXAMPLE, TableDiff() can still
   // crunch on Example objects
   ResetTables();

   PrintHeader(cout, "DynamicIndexedViewExample()");

   vector<variant_row> old_data = ReadDynamicData();
 
   DynamicIndexedViewExample();
   
   cout << "Changes after operating on dynamic indexed view:" << endl;
   
   vector<variant_row> new_data = ReadDynamicData();

   TableDiff(cout, old_data, new_data);

   PrintSeparator(cout);

   // Read a table into a fixed Example class structure
   PrintHeader(cout, "ReadData()");
   ResetTables();

   cout << "Initial Examples read from database:" << endl;
   vector<Example> examples = ReadData();
   copy(examples.begin(), examples.end(), ostream_iterator<Example>(cout, "\n"));

   PrintSeparator(cout);

   // now insert Example objects into the database
   PrintHeader(cout, "WriteData()");

   cout << "Trying to insert the following Example objects into DB" << endl;

   vector<Example> ins;
   
   ins.push_back(Example(555, "Arthur", 1.1, 1, now));
   ins.push_back(Example(311, "", 3.99, 91, now)); // should fail on InsValidate()
   ins.push_back(Example(666, "Langham", 2.2, 2, now));
   ins.push_back(Example(222, "Positron", -34.77, 29, now)); // should fail (ditto)
   ins.push_back(Example(777, "Lala", 3.3, 3, now));
   ins.push_back(Example(911, "The Club", 102.32, 67, now)); // should fail (ditto)

   copy(ins.begin(), ins.end(), ostream_iterator<Example>(cout, "\n"));

   cout << "Only three items should succeed on insert!  The rest represent 'bad' "
      "data we are using to test the InsValidate function!" << endl;

   WriteData(ins);

   vector<Example> updated_examples = ReadData();

   cout << "Example objects inserted into DB according to TableDiff():" << endl;
   TableDiff(cout, examples, updated_examples);

   PrintSeparator(cout);

   // see if table diff results are the same with using the view of the original
   // table versus that of the current one
   PrintHeader(cout, "TestTableDiff()");

   cout << "TableDiff() results should match ... we get:" << endl;
   
   TestTableDiff();

   PrintSeparator(cout);

   // Read a joined view
   
   ResetTables();
   
   PrintHeader(cout, "ReadJoinedData()");

   vector<JoinExample> join_examples = ReadJoinedData();

   cout << "Read these JoinExamples from the DB:" << endl;
   copy(join_examples.begin(), join_examples.end(),
	    ostream_iterator<JoinExample>(cout, "\n"));

   PrintSeparator(cout);

   // now call an example which uses an indexed view with Examples
   // and qualified by a postfix clause and uses specialized comparisons for the keys
   ResetTables();

   PrintHeader(cout, "IndexedViewExample()");
   
   vector<Example> old_examples = ReadData();

   IndexedViewExample();

   vector<Example> after_idx_examples = ReadData();

   cout << "Changes resulting from call to indexed view example:" << endl;
   
   TableDiff(cout, old_examples, after_idx_examples);

   PrintSeparator(cout);

   ResetTables();

   
    // Stored procedure examples for Oracle
    if (DBConnection::GetDefaultConnection().GetDBMSEnum() == DBConnection::DB_ORACLE)
    {
         PrintHeader(cout, "Notice: Stored Proc examples will only work with "
 				"Oracle ODBC driver version 8.1.5.3.0 or higher!");
    	    PrintHeader(cout, "StoredProcRead3()");
    		StoredProcRead3();
    		PrintSeparator(cout);

    		ResetTables();


			try {
				PrintHeader(cout, "StoredProcRead2()");
				cout << "This procedure uses Oracle ODBC driver specific syntax and will not work with Merant drivers." << endl;
    			StoredProcRead2();
    			PrintSeparator(cout);
			}
			catch (RootException &ex)
			{
			  cout << "Exception thrown" << endl;
			  cout << ex << endl;
			}
    }


	// Stored procedure examples for SQL Server
	if (DBConnection::GetDefaultConnection().GetDBMSEnum() == DBConnection::DB_SQL_SERVER) 
	{
		StoredProcReadTestParm();
	}
   // ----------------------------------------------------------------------
   // BEGIN examples of overriding indexing & container methods in IndexDBView


   // show the ability to customize how IndexDBView indexes
   // records and to even control what type of container
   // is used to index those records

   // first warm-up with a simple read of an indexed view
   // of Examples which just reads in all of DB_EXAMPLE
   // and indexes on certain fields
   ResetTables();
   PrintHeader(cout, "VerySimpleReadFromIndexedView()");

   VerySimpleReadFromIndexedView();

   PrintSeparator(cout);

   // another run at this, but with a postfix clause
   ResetTables();

   PrintHeader(cout, "SimpleReadFromIndexedView()");

   SimpleReadFromIndexedView();

   PrintSeparator(cout);


#ifdef __SGI_STL_PORT  // Native VC 6.0 does not have hash_set or hash_multiset

   // now play with indexed views that use hashed containers
   // first a simple case with no postfix clause ... uses default container and semantics
   ResetTables();
   
   PrintHeader(cout, "VerySimpleReadFromHashedIndexedView()");

   VerySimpleReadFromHashedIndexedView();

   PrintSeparator(cout);

   // now do one that has a postfix clause and has a specialized container factory
   ResetTables();

   PrintHeader(cout, "SimpleReadFromHashedIndexedView()");

   SimpleReadFromHashedIndexedView();

   PrintSeparator(cout);

#endif // end hashed container example


   // now for a indexed view with a custom container
   ResetTables();

   PrintHeader(cout, "SimpleReadFromCustomIndexedView()");

   SimpleReadFromCustomIndexedView();

   PrintSeparator(cout);

   ResetTables();

   PrintHeader(cout, "CustomIndexedViewExample()");

   vector<Example> old_custom_examples = ReadData();

   CustomIndexedViewExample();

   vector<Example> after_custom_idx_examples = ReadData();

   cout << "Changes resulting from call to indexed view example:" << endl;
   
   TableDiff(cout, old_custom_examples, after_custom_idx_examples);

   PrintSeparator(cout);

   // Example showing Sharing of DB connections
   ResetTables();

   PrintHeader(cout, "SharedConnectionRead()");

   SharedConnectionRead(DSN_str);

   PrintSeparator(cout);

   // Examples to show exception saftety over a set of
   // transactions

   ResetTables();

   PrintHeader(cout, "RangeInsertExample()");

   // must release this connection so that other connections may be made
   // without being locked out
   DBConnection::GetDefaultConnection().Release();

   // test range insert transactions using a view
   // transaction will fail, so should not change DB
   RangeInsertExample();

   PrintSeparator(cout);

   // need to reset just in-case above example does not rollback properly
   DBConnection::GetDefaultConnection().Connect();
	 ResetTables();
   DBConnection::GetDefaultConnection().Release();


   PrintHeader(cout, "RangeIndexInsertExample()");

   // test range insert transactions using an indexed view
   // transaction will fail, so should not change DB
   RangeIndexInsertExample();

   PrintSeparator(cout);

   // need to reset just in-case above example does not rollback properly
   DBConnection::GetDefaultConnection().Connect();
   ResetTables();
   DBConnection::GetDefaultConnection().Release();


   PrintHeader(cout, "RangeIndexUpdateExampleFailure()");

   // test range update transactions using an indexed view
   // transaction will fail, so should not change DB
   RangeIndexUpdateExampleFailure();

   PrintSeparator(cout);

   DBConnection::GetDefaultConnection().Connect();
   ResetTables();
   DBConnection::GetDefaultConnection().Release();

   PrintHeader(cout, "RangeIndexUpdateExampleSuccess()");

   // test range update transactions using an indexed view
   // transaction will succeed, so should change DB as well as indexed view
   RangeIndexUpdateExampleSuccess();

   PrintSeparator(cout);

   DBConnection::GetDefaultConnection().Connect();

   // use a view that builds a custom query ... simply a SELECT distinct in this case

   ResetTables();

   PrintHeader(cout, "SpecialQryExample()");
   SpecialQryExample();	
   PrintSeparator(cout);

   // Test Long Strings
   InsLong();

   // Test bulk copy
   ResetTables();
   
   PrintHeader(cout, "BulkCopyExample()");
   BulkCopyExample();
   PrintSeparator(cout);

   ResetTables();
   }

   catch (RootException &ex)
   {
	cout << "Exception thrown" << endl;
	cout << ex << endl;
   }
}
