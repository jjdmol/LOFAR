#include "ReadData.h"

#include "table.h"

#include "RandomDBView.h"


// Read the contents of the DB_EXAMPLE table and return a vector of the
// resulting rows
// The "Example" class and binding to database are defined in example_core.h
vector<Example> ReadData()
{
	 vector<Example> results;
	 DBView<Example> view(DBView<Example>::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));

	 copy(view.begin(), view.end(), back_inserter(results));

	 return results;
}



typedef tcstring<50> tcstring50_t;

class ExampleCharred
{
  public:							// tablename.columnname:
	int exampleInt;	    			// DB_EXAMPLE.INT_VALUE
	char exampleStr[100];			// DB_EXAMPLE.STRING_VALUE
	double exampleDouble;			// DB_EXAMPLE.DOUBLE_VALUE
	long exampleLong;				// DB_EXAMPLE.EXAMPLE_LONG
	TIMESTAMP_STRUCT exampleDate;	// DB_EXAMPLE.EXAMPLE_DATE
	
	ExampleCharred() : exampleInt(0), exampleDouble(0.0), exampleLong(0) 
	{ 
		exampleStr[0] = 0;
		TIMESTAMP_STRUCT exDate = {2000, 6, 6, 13, 13, 0, 0};
		exampleDate = exDate;
	}

	ExampleCharred(int exInt, const char *exStr, double exDouble, long exLong, const TIMESTAMP_STRUCT &exDate) :
	   exampleInt(exInt), exampleDouble(exDouble), exampleLong(exLong),
	   exampleDate(exDate)
	{ 
		std_strncpy(exampleStr, exStr, 99);   
	}

    ExampleCharred(const ExampleCharred &other) : 
		exampleInt(other.exampleInt), exampleDouble(other.exampleDouble),
		exampleLong(other.exampleLong), exampleDate(other.exampleDate)
	{
		std_strcpy(exampleStr, other.exampleStr);
	}

	ExampleCharred(const Example &other) :
		exampleInt(other.GetExampleInt()), exampleDouble(other.GetExampleDouble()),
		exampleLong(other.GetExampleLong()), exampleDate(other.GetExampleDate())
	{
		std_strcpy(exampleStr, other.GetExampleStr().c_str());
	}
	
	ExampleCharred &operator=(const ExampleCharred &other)
	{
		if (this != &other)
		{
			exampleInt = other.exampleInt;
			exampleDouble = other.exampleDouble;
			exampleLong = other.exampleLong;
			exampleDate = other.exampleDate;
			std_strcpy(exampleStr, other.exampleStr);
		}

		return *this;

	}

	friend bool operator==(const ExampleCharred &ex1, const ExampleCharred &ex2)
	{
	  return (ex1.exampleInt == ex2.exampleInt) &&
			 (std_tstrcmp(ex1.exampleStr, ex2.exampleStr) == 0) &&
			 (ex1.exampleDouble == ex2.exampleDouble) &&
			 (ex1.exampleLong == ex2.exampleLong) &&
			 (ex1.exampleDate == ex2.exampleDate);
	}

	friend bool operator<(const ExampleCharred &ex1, const ExampleCharred &ex2)
	{
		if (ex1.exampleInt < ex2.exampleInt)
			return true;
		if (ex1.exampleInt > ex2.exampleInt)
			return false;

		int strResult = std_strcmp(ex1.exampleStr, ex2.exampleStr);

		if (strResult < 0)
			return true;
		if (strResult > 0)
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


	friend tostream& operator<<(tostream &o, const ExampleCharred &ex)
	{
		o << _TEXT("ExampleCharred(") << ex.exampleInt << _TEXT(", \"") << ex.exampleStr << _TEXT("\", ");
		o << ex.exampleDouble << _TEXT(", ") << ex.exampleLong << _TEXT(", ");
		o << ex.exampleDate << _TEXT(")");
		return o;
	}
};

void CharredBCA(BoundIOs &boundIOs, ExampleCharred &rowbuf);

BEGIN_DTL_NAMESPACE
template<> class DefaultBCA<ExampleCharred>
{
public:
	void operator()(BoundIOs &boundIOs, ExampleCharred &rowbuf)
    {
	  boundIOs[_TEXT("INT_VALUE")] 	 == rowbuf.exampleInt;
	  boundIOs[_TEXT("STRING_VALUE")]   == 
		  DTL_CHAR_FIELD(rowbuf.exampleStr);
      boundIOs[_TEXT("DOUBLE_VALUE")]   == rowbuf.exampleDouble;
	  boundIOs[_TEXT("EXAMPLE_LONG")]   == rowbuf.exampleLong;
	  boundIOs[_TEXT("EXAMPLE_DATE")]   == rowbuf.exampleDate;
	}
};
END_DTL_NAMESPACE

class BulkSelVal : public binary_function<BoundIOs &, ExampleCharred &, bool>
{
public:
	bool operator()(BoundIOs &boundIOs, ExampleCharred &rowbuf);
};


// bulk copy example
void BulkCopyExample()
{
	 // note: while bulk_copy() can be used on DataObj's which contain std::string fields,
	 // using std::string will make bulk_copy() run *much* slower
	 // as these fields must then be copied internally in DTL using GetData() and PutData()
	 // as char arrays bind directly to the DB, you should use char arrays
	 // instead of std::string for the best performance with bulk_copy()

	 vector<ExampleCharred> results;

	 typedef DBView<ExampleCharred> DBV;

	 DBV view(DBV::Args().tables("DB_EXAMPLE").SelValidate(
		 BulkSelVal()
		).handler(
		   BulkFetchHandler<ExampleCharred>()
		).postfix(exampleOrderBy)
	 );

	 // show use of bulk_fetch_helper()
	 DBV::select_iterator beg(view.begin());
	 cout << "***" << endl;
	 cout << "Examples read using bulk_fetch_helper()" << endl;
	 cout << endl;
	 bulk_fetch_helper(beg, 100, back_inserter(results));
	 copy(results.begin(), results.end(), ostream_iterator<ExampleCharred>(cout, "\n"));

	 jtime_c jtime;
	 DBView<jtime_c> jt_view("DB_EXAMPLE", BCA(jtime, COLS["EXAMPLE_DATE"] == jtime),
		 "ORDER BY EXAMPLE_DATE");
	 DBView<jtime_c>::select_iterator jt_beg(jt_view.begin());
	 cout << "***" << endl;
	 cout << "Examples read using bulk_fetch_helper() for jtimes" << endl;
	 cout << endl;
	 bulk_fetch_helper(jt_beg, 100, ostream_iterator<jtime_c>(cout, "\n"));


	 int myInt;
	 DBView<int> int_view("DB_EXAMPLE", BCA(myInt, COLS["INT_VALUE"] == myInt),
		 "ORDER BY INT_VALUE");
	 DBView<int>::select_iterator int_beg(int_view.begin());
	 cout << "***" << endl;
	 cout << "Examples read using bulk_fetch_helper() for ints" << endl;
	 cout << endl;
	 bulk_fetch_helper(int_beg, 100, ostream_iterator<int>(cout, "\n"));

     DBStmt stmt("DELETE FROM DB_EXAMPLE");
     stmt.Execute();


	 cout << "***" << endl;

	 cout << "*** From here on out ... don't care about validation any more ***" << endl;

	 cout << "Now inserting 10 Examples using bulk_copy() ..." << endl;
	 cout << "(this will failover to regular copy() for databases like MS Access that don't support bulk insert)" << endl;

     // ExampleCharred output[10];
	 list<ExampleCharred> output;
     DBView<ExampleCharred> view2(DBView<ExampleCharred>::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));
   
     copy(&results[0], &results[10], back_inserter(output));
 
     DBView<ExampleCharred>::insert_iterator iit;
     iit = view2;
   
	 try {
		 bulk_insert_helper(output.begin(), output.end(), 100, iit);
	 } 
	 catch(std::exception &ex) {
		 cout << ex.what() << endl;
		 cout << "Error. This ODBC driver does not support bulk insert / row-wise arrays of parameters." << endl;
		 cout << "Using regular copy() function." << endl;
		 iit = view2; // restore the iterator
		 copy(output.begin(), output.end(), iit);

	 }

     DBConnection::GetDefaultConnection().CommitAll();

	 cout << "Records inserted as read from database ... " << endl;
	 copy(view2.begin(), view2.end(), ostream_iterator<ExampleCharred>(cout, "\n"));
}

///////////////////////////////////////////////
// Test support for long strings
//////////////////////////////////////////////
struct Test {
	string str;
};

class BCAAccess : public binary_function<BoundIOs &, Test &, void>
{
public:
	void operator()(BoundIOs &boundIOs, Test &rowbuf)
	{
		boundIOs["STRING_VALUE"] == rowbuf.str;
		boundIOs["STRING_VALUE"].SetSQLType(SQL_LONGVARCHAR);
		boundIOs["STRING_VALUE"].SetColumnSize(500);
	}
};

void InsLong() {
	PrintHeader(cout, "InsLong()");
	Test rowbuf, rowbuf_read;
	DBStmt stmt("DELETE FROM LONG_EXAMPLE");
	stmt.Execute();
	DBConnection::GetDefaultConnection().CommitAll();
	
	DBView<Test> view;
	if (DBConnection::GetDefaultConnection().GetDBMSEnum() !=
		DBConnection::DB_ACCESS)
	{
		view = DBView<Test>("LONG_EXAMPLE", BCA(rowbuf, COLS["STRING_VALUE"] == rowbuf.str),
			"ORDER BY STRING_VALUE");
	}
	else
	{
		view = DBView<Test>("LONG_EXAMPLE", BCAAccess(),
			"ORDER BY STRING_VALUE");
	}

	DBView<Test>::insert_iterator it = view;

	for (int i=0 ; i < 500; ++i)// Varchar in oracle can only be up to 4000 bytes
		rowbuf.str += char('0' + i%10 - 1);


	*it = rowbuf;
	++it;

	 DBConnection::GetDefaultConnection().CommitAll();

	 DBView<Test>::select_iterator read_it = view.begin();

	 rowbuf_read = *read_it;

	 if (rowbuf.str != rowbuf_read.str)
	 {
		 cout << "<<<<< Error with long strings! >>>>>" << endl;
		 cout << "String written ..." << endl;
		 cout << rowbuf.str << endl;
		 cout << "String read ... " << endl;
		 cout << rowbuf_read.str << endl;
		 rowbuf_read.str = "error";
	 }
	 else
	 {
		 cout << "Long string test successful!" << endl;
	 }

	 PrintSeparator(cout);
}

bool BulkSelVal::operator()(BoundIOs &boundIOs, ExampleCharred &rowbuf)
{
	// fail example objects with exampleStr == "Find Me"
	if (strcmp(rowbuf.exampleStr, "Find Me") == 0)
		return false;

	return true;
}



// DTL_TABLE3 generates: db_example_row, DefaultBCA<db_example_row>,
// db_example_view, and instantiates a db_example_view named db_example 
// Note: macro must be invoked at namespace scope because
// DTL uses templates to implement the macro and
// template parameters cannot have local linkage (local structs not allowed)

DTL_TABLE3(db_example,
   int, int_value,
   string, string_value,
   double, double_value
);


// note that the field names in the table are the field names in the
// generated object
void TableStructExample()
{
   PrintHeader(cout, "TableStructExample()");

   // only using an IndexedDBView here to gain consistent ordering for regression tests
   DEFAULT_INDEX_VIEW(db_example_view) idx_view(db_example,
	   "PrimaryKey; int_value, string_value, double_value");

   cout << "Objects read from DB:" << endl;

   for (DEFAULT_INDEX_VIEW(db_example_view)::iterator it = idx_view.begin();
          it != idx_view.end(); ++it)
   {
       cout << it->int_value << " " << it->string_value
                 << " " << it->double_value << endl;
   }

  
   PrintSeparator(cout);
}

// Bulk Fetch using bulk_fetch_helper versus a DynamicDBView
void DynamicBulkFetch()
{
	PrintHeader(cout, "DynamicBulkFetch()");
	DynamicDBView<> view("DB_EXAMPLE", "*");
	vector<variant_row> results;

	bulk_fetch_helper(view.begin(), 32, back_inserter<vector<variant_row> >(results));
	copy(results.begin(), results.end(), ostream_iterator<variant_row>(cout, "\n"));

}

// Random access DBView built from a DynamicDBView
void RandomDynamicDBView()
{
	PrintHeader(cout, "RandomDynamicDBView()");
	DynamicDBView<> dynamic_view("DB_EXAMPLE", "*");
	RandomDBView<variant_row> random_view(dynamic_view);

	cout << "Items from DB:" << endl;
	copy(random_view.begin(), random_view.end(), ostream_iterator<variant_row>(cout, "\n"));
	cout << "\n\n";

	cout << "Items from DB in reverse order:" << endl;
	copy(random_view.rbegin(), random_view.rend(), ostream_iterator<variant_row>(cout, "\n"));

	// insert and delete rows
	variant_row row_insert(random_view[2]);
	row_insert["INT_VALUE"] = 666;
	random_view.insert(row_insert);
	random_view.erase(random_view.begin()+(ptrdiff_t)2);
	cout << "Show result set with inserted/deleted row:" << endl;
	random_view.ReQuery();
    copy(random_view.begin(), random_view.end(), ostream_iterator<variant_row>(cout, "\n"));

	PrintSeparator(cout);
}

// Random Access container example
void RandomDBViewExample()
{
   PrintHeader(cout, "RandomDBViewExample()");

   RandomDBView<db_example_row> view(db_example);

   cout << "Objects read from DB:" << endl;
   copy(view.begin(), view.end(), ostream_iterator<db_example_row>(cout, "\n"));

   // modify the third row in the table
   RandomDBView<db_example_row>::iterator it = view.begin();
   it += 2;
   db_example_row row(*it);
   row.int_value++;
   *it = row;
   
   cout << "\nElements in reverse order:" << std::endl;
   cout << "NOTE THAT MODIFIED ROW WILL NOT SHOW UP BECAUSE WE HAVE NOT CALLED ReQuery()" << std::endl;
   
   copy(view.rbegin(), view.rend(), ostream_iterator<db_example_row>(cout, "\n"));

   // Note that there is no guarantee that the third row will show the updated record,
   // since other users may have modified the database or the DB may return records in a different
   // order.  See the documentation on the ReQuery() function for details.
   std::cout << "Show updated result row:" << std::endl;
   view.ReQuery();
   it = view.begin();
   std::cout << it[2] << std::endl;
 
   cout << "Distance from first to last: " << view.end() - view.begin() << std::endl;
   cout << "Container size: " << view.size() << std::endl;

   // insert and delete rows
   db_example_row row_insert(it[2]);
   view.erase(it+(ptrdiff_t)2);
   row_insert.int_value = 666;
   view.insert(row_insert);
   std::cout << "Show result set with inserted/deleted row:" << std::endl;
   view.ReQuery();
   std::copy(view.begin(), view.end(), ostream_iterator<db_example_row>(std::cout, "\n"));

   // show container comparison operators
   RandomDBView<db_example_row> view2(view);
#ifndef __GNUC__  // gcc 2.96 does not support boolalpha format flag
   cout << boolalpha;
#endif
   cout << "view == view2 : " << (view == view2) << std::endl;
   cout << "view < view2 : " << (view < view2) << std::endl;

   PrintSeparator(cout);
}






