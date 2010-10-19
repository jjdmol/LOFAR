#include "example_core.h"

// prints out asterisks between each example function
void PrintSeparator(ostream &o)
{
   o << endl;
   o << "********************************" << endl;
   o << endl;
}

// prints a header with title for a particular example
void PrintHeader(ostream &o, const string &title)
{
   o << "!!!!!!!!!!!!!!!!!!!!! Begin Example " << title << " !!!!!!!!!!!!!!!!!!!! ";
   o << endl;
}


// Set parameters function for Example ... used by IndexedDBView<Example> to set dynamic query parameters
// Dynamic query parameters are indicated by (?) in our query string for the IndexedDBView
void SetParamsExample(ParamObjExample &params)
{
	// set parameter values
	params.lowIntValue = 2;
	params.highIntValue = 8;
	params.strValue = "Example";
	
	TIMESTAMP_STRUCT paramDate = {2000, 1, 1, 0, 0, 0, 0};
	params.dateValue = paramDate;
}

void SetParamsSimpleExample(ParamsSimpleExample &params)
{
	// set parameter values
	params.lowIntValue = 2;
	params.highIntValue = 8;
	params.strValue = "Example";
	
	TIMESTAMP_STRUCT paramDate = {2000, 1, 1, 0, 0, 0, 0};
	params.dateValue = paramDate;
}

// this function is used to reset the Example table in the database
void ResetTables(DBConnection &conn)
{
	DBStmt("DELETE FROM DB_EXAMPLE", conn).Execute();
	DBStmt("INSERT INTO DB_EXAMPLE SELECT * FROM DB_EXAMPLE_BACKUP", conn).Execute();
}

// test out TableDiff() using old and new views
void TestTableDiff()
{
  typedef DBView<Example>         DBV;
  typedef DEFAULT_INDEX_VIEW(DBV) idxDBV;

  // first for DBView's
  DBV new_table(DBV::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));
  DBV old_table(DBV::Args().tables("DB_EXAMPLE_BACKUP").postfix(exampleOrderBy));
  TableDiff(cout, old_table, new_table);

  cout << "--- should be same for IndexedDBViews --- " << endl;

  // now do the same thing for an IndexedDBView

  idxDBV  new_idx_table(new_table, "PrimaryIndex; STRING_VALUE");
  idxDBV  old_idx_table(old_table, "PrimaryIndex; STRING_VALUE");
  TableDiff(cout, old_idx_table, new_idx_table);
}

// reverse comparison on exampleStr
bool reverse_compare_strings(const Example *pData1, const Example *pData2)
{
	return pData1->GetExampleStr() > pData2->GetExampleStr();
}

// alternative hashing scheme on exampleStr
size_t my_hash_strings(const Example *pData1)
{
   string str = pData1->GetExampleStr();

   size_t sum = 0;

   // sum all the per character hash values together
   // a character c's hash value = 5*c + 13
   for (size_t i = 0; i < str.length(); i++)
		sum += (5 * str[i] + 13);

   return sum;
}
