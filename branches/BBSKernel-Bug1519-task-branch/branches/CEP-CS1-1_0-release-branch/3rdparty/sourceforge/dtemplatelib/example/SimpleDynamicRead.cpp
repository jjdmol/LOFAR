// example of reading from a dynamic view ... when you don't know the
// columns or their types

#include "SimpleDynamicRead.h"



// Read the contents of a table and print the resulting rows
void SimpleDynamicRead() {
	// Our query will be "SELECT * FROM DB_EXAMPLE"
	DynamicDBView<> view(DynamicDBView<>::Args().tables("DB_EXAMPLE").fields("*").postfix(exampleOrderBy).key_mode(USE_AUTO_KEY));

	// NOTE: We need to construct s from the view itself since we
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

	copy(view.begin(), view.end(), ostream_iterator<variant_row>(cout, "\n"));

#if 0
	// Print out all rows and columns from our query
	DynamicDBView<>::select_iterator print_it = view.begin();
	variant_row r;
	for (print_it = view.begin(); print_it != view.end(); ++print_it)
	{
		 r = *print_it;
		 for (size_t i = 0; i < r.size(); ++i)
		 	cout << r[i] << " ";
		 cout << endl;
	}
#endif

}

// Read the contents of a table and return the resulting rows
vector<variant_row> ReadDynamicData() {

	vector<variant_row> result;

	// Our query will be "SELECT * FROM DB_EXAMPLE"
	DynamicDBView<> view(DynamicDBView<>::Args().tables("DB_EXAMPLE").fields("*").postfix(exampleOrderBy));


	// Print out all rows and columns from our query
	DynamicDBView<>::select_iterator print_it = view.begin();
	variant_row r;
	for (print_it = view.begin(); print_it != view.end(); ++print_it)
	{
		 r = *print_it;
		 result.push_back(r);
	}

	return result;
}

// Execute an arbitrary query and prompt for any parameters
void ExecQuery(string sql)
{
	DynamicDBView<> view(sql, "");
	DynamicDBView<>::sql_iterator sql_it(view);
	
	// Show query
	cout << sql << "\n\n";

	// Prompt user for any parameters
	for (size_t i = 0; i < sql_it.GetBoundParamCount(); i++)
	{
		cout << "Please enter the value for parameter number " << i << "\n";
		string param;
		cin >> param;

		sql_it.Params()[i] = param;

	}

	// Force an execute in case there is no result set, e.g. INSERT, DELETE
	*sql_it = view.GetDataObj();

	// Print results
	while (sql_it != view.end())
	{
		cout << (variant_row)(*sql_it) << "\n";
		++sql_it;
	}
}

void ExecQueryExample() {
	ExecQuery("SELECT * from DB_EXAMPLE WHERE INT_VALUE = (?)");
}


// Read the contents of a table with a dynamic where clause
void SimpleDynamicWhere() {

	DynamicDBView<> view("DB_EXAMPLE", "*", "WHERE INT_VALUE = (?) AND STRING_VALUE = (?)");

	cout << "expected number of params " << view.GetBoundParamCount() << "\n";
	variant_row params(view.GetParamObj());

	params[0] = 1; // In practice you will likely read a string in from the user and just assign it here.  Let the class do the work of converting it to the proper type
	params[1] = string("Find Me");

	cout << "Params: " << endl;
	cout << params << endl;

    DynamicDBView<>::select_iterator sel_it = view.begin();
	sel_it.Params() = params;

	// Print out the column names
	vector<string> colNames = view.GetColNames();
	for (vector<string>::iterator name_it = colNames.begin(); name_it != colNames.end(); ++name_it)
		cout << (*name_it) << " ";
	cout << endl;

	// print select
	copy(sel_it, view.end(), ostream_iterator<variant_row>(cout, "\n"));
}



 template <class InputIterator, class OutputIterator>
  OutputIterator my_copy (InputIterator first, InputIterator last,
                       OutputIterator result)
  {
	 while (first != last) {
		*result = *first;
		++result;
		++first;
	 }
    return result;
  }

// Copy a table from one database to another
void DynamicCopy(tstring table, DBConnection &conn1, DBConnection &conn2) {
	DynamicDBView<>::Args arg1, arg2;
	arg1.tables(table).fields("*").conn(conn1).postfix(exampleOrderBy);
	arg2.tables(table).fields("*").conn(conn2).key_mode(USE_ALL_FIELDS).postfix(exampleOrderBy);

	DynamicDBView<> view1(arg1), view2(arg2);
	
	// Copy all rows
	DynamicDBView<>::insert_iterator write_it = view2;
	copy(view1.begin(), view1.end(), write_it);
}

void TestDynamicCopy() {
	DBConnection conn1;
	conn1.Connect("uid=example;pwd=example;dsn=exampleA;");
	ResetTables(conn1);
	DBStmt("DELETE FROM DB_EXAMPLE").Execute();
	DynamicCopy("DB_EXAMPLE", conn1, DBConnection::GetDefaultConnection());
	DBConnection::GetDefaultConnection().CommitAll();
}



