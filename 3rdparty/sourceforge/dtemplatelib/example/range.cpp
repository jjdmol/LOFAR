#include "range.h"

const TIMESTAMP_STRUCT chrysalis = {2002, 4, 3, 0, 0, 0, 0};
const TIMESTAMP_STRUCT mikero = {2001, 11, 2, 0, 0, 0, 0};
const TIMESTAMP_STRUCT victory = {2001, 3, 10, 0, 0, 0, 0};

// this example shows range insert transactions in action
void RangeInsertExample()
{
	DBConnection conn;
    conn.Connect(conn.GetDefaultConnection().GetDSN());

	typedef DBView<Example> DBV;

	DBV view(DBV::Args().tables("DB_EXAMPLE").handler(AlwaysThrowsHandler<Example>()).conn(conn).postfix(exampleOrderBy));

	vector<Example> read_from_DB_before;

	copy(view.begin(), view.end(), back_inserter(read_from_DB_before));

	// examples that we want to insert into the DB ...
	// we want an all or nothing on these guys!
	vector<Example> all_or_nothing_examples;

	// third element will fail to be inserted, should force rollback
	all_or_nothing_examples.push_back(Example(79, "FUBAR", 2.2, 99, mikero));
	all_or_nothing_examples.push_back(Example(81, "All Messed Up", 21.09, 75, chrysalis));
	all_or_nothing_examples.push_back(Example(85, "Bad Boy", -21.22, 11, victory));
	all_or_nothing_examples.push_back(Example(99, "Good One", 77.99, 41, victory));
	
	// must write all the elements to succeed in the transaction
	// else we rollback
	try { 
      
	  DBV::insert_iterator write_it = view;

	  for (vector<Example>::iterator ins_it = all_or_nothing_examples.begin(); 
	      ins_it != all_or_nothing_examples.end(); ins_it++, write_it++)
		  {	  
			 *write_it = *ins_it;
		  }

      conn.CommitAll(); // we assume commit and rollback must always succeed to avoid two-phase commit type logic
	}
    catch(RootException &ex) 
	{ 
	  cout << ex << endl;
      conn.RollbackAll(); 
	}

	vector<Example> read_from_DB_after;

	copy(view.begin(), view.end(), back_inserter(read_from_DB_after));

	cout << "Changes resulting from attempted range insert:" << endl;
   
  TableDiff(cout, read_from_DB_before, read_from_DB_after);
} 

// same, but for an indexed view
void RangeIndexInsertExample()
{
	DBConnection conn;
    conn.Connect(conn.GetDefaultConnection().GetDSN());

	typedef DBView<Example>         DBV;
	typedef DEFAULT_INDEX_VIEW(DBV) IdxDBV;

	DBV view(DBV::Args().tables("DB_EXAMPLE").handler(AlwaysThrowsHandler<Example>()).conn(conn).postfix(exampleOrderBy));

	IdxDBV idxview(view, 
		"PrimaryIndex; STRING_VALUE; UNIQUE AlternateIndex; EXAMPLE_LONG, EXAMPLE_DATE",
		BOUND);

	vector<Example> read_from_DB_before;

	copy(idxview.begin(), idxview.end(), back_inserter(read_from_DB_before));

	// examples that we want to insert into the DB ...
	// we want an all or nothing on these guys!
	vector<Example> all_or_nothing_examples;

	// third element will fail to be inserted, should force rollback
	all_or_nothing_examples.push_back(Example(79, "FUBAR", 2.2, 99, mikero));
	all_or_nothing_examples.push_back(Example(81, "All Messed Up", 21.09, 75, chrysalis));
	all_or_nothing_examples.push_back(Example(85, "Bad Boy", -21.22, 99, victory));
	all_or_nothing_examples.push_back(Example(99, "Good One", 77.99, 41, victory));
	
	// must write all the elements to succeed in the transaction
	// else we rollback

	IdxDBV tmp(idxview); // make copy so we can rollback to idxview on failure

	try { 
	  for (vector<Example>::iterator ins_it = all_or_nothing_examples.begin(); 
	      ins_it != all_or_nothing_examples.end(); ins_it++)
		  {	  
			 idxview.insert(*ins_it); // work with tmp
		  }

      conn.CommitAll(); // we assume commit and rollback must always succeed to avoid two-phase commit type logic
	}
    catch(RootException &ex) 
	{ 
	  cout << ex << endl;
	  idxview.swap(tmp); // this will rollback to original results in memory
      conn.RollbackAll(); 
	}

	vector<Example> read_from_DB_after;

	copy(idxview.begin(), idxview.end(), back_inserter(read_from_DB_after));

	cout << "Local Changes (to IndexedDBView) resulting from attempted range insert:" << endl;
   
  TableDiff(cout, read_from_DB_before, read_from_DB_after);

	cout << endl;

  read_from_DB_after.clear();

  copy(view.begin(), view.end(), back_inserter(read_from_DB_after));

	cout << "Remote Changes (to Database) resulting from attempted range insert:" << endl;

  TableDiff(cout, read_from_DB_before, read_from_DB_after);  
}

// range update (fail)
void RangeIndexUpdateExampleFailure()
{
	DBConnection conn;
    conn.Connect(conn.GetDefaultConnection().GetDSN());

	typedef DBView<Example>          DBV;
	typedef DEFAULT_INDEX_VIEW(DBV)  IdxDBV;

	DBV view(DBV::Args().tables("DB_EXAMPLE").handler(AlwaysThrowsHandler<Example>()).conn(conn).postfix(exampleOrderBy));

	IdxDBV idxview(view, 
				   "PrimaryIndex; STRING_VALUE; UNIQUE AlternateIndex; EXAMPLE_LONG, EXAMPLE_DATE",
				   BOUND);

	vector<Example> read_from_DB_before;

	copy(idxview.begin(), idxview.end(), back_inserter(read_from_DB_before));

	// examples that we want to insert into the DB ...
	// we want an all or nothing on these guys!
	// string indicates key of element to replace, Example is object to replace the object
	// to replace with
	map<string, Example> all_or_nothing_examples;

	// third element will fail to be updated, should force rollback
	all_or_nothing_examples["Bedazzled"] = Example(79, "FUBAR", 2.2, 99, mikero);
	all_or_nothing_examples["Corwin"] = Example(81, "All Messed Up", 21.09, 75, chrysalis);
	all_or_nothing_examples["Jordan"] = Example(85, "Bad Boy", -21.22, 99, victory);
	all_or_nothing_examples["Mirror Image"] = Example(99, "Good One", 77.99, 41, victory);
	
	// must update all the elements to succeed in the transaction
	// else we rollback

	IdxDBV tmp(idxview); // make copy so we can rollback to idxview on failure

	// march through vector and replace elements appropriately
	try {
	  map<string, Example>::iterator map_it;
	  
	  for (map_it = all_or_nothing_examples.begin(); 
	       map_it != all_or_nothing_examples.end(); 
		     map_it++)
		  {	  
		     IdxDBV::iterator find_it = idxview.find((*map_it).first);

			   if (find_it != idxview.end())
				    idxview.replace(find_it, (*map_it).second);
		  }

      conn.CommitAll(); // we assume commit and rollback must always succeed to avoid two-phase commit type logic
	}
    catch(RootException &ex) 
	{ 
	  cout << ex << endl;
	  idxview.swap(tmp); // this will rollback to original results in memory
      conn.RollbackAll(); 
	}

	vector<Example> read_from_DB_after;

	copy(idxview.begin(), idxview.end(), back_inserter(read_from_DB_after));

	cout << "Local changes (to IndexedDBView) resulting from attempted range update:" << endl;
   
    TableDiff(cout, read_from_DB_before, read_from_DB_after);

	cout << endl;

  read_from_DB_after.clear();

  copy(view.begin(), view.end(), back_inserter(read_from_DB_after));

  cout << "Remote changes (to Database) resulting from attempted range update:" << endl;

  TableDiff(cout, read_from_DB_before, read_from_DB_after);
}

// range update (successful)
void RangeIndexUpdateExampleSuccess()
{
  	DBConnection conn;
    conn.Connect(conn.GetDefaultConnection().GetDSN());

	typedef DBView<Example>          DBV;
	typedef DEFAULT_INDEX_VIEW(DBV)  IdxDBV;

#if 0
	DBV view("DB_EXAMPLE", DefaultBCA<Example>(), 

	   exampleOrderBy, DefaultBPA<DefaultParamObj<Example> >(), DefaultSelValidate<Example>(),
	   DefaultInsValidate<Example>(), AlwaysThrowsHandler<Example>(), conn);
#endif

	DBV view(DBV::Args().tables("DB_EXAMPLE").handler(AlwaysThrowsHandler<Example>()).conn(conn).postfix(exampleOrderBy));

	IdxDBV idxview(view, 
		"PrimaryIndex; STRING_VALUE; UNIQUE AlternateIndex; EXAMPLE_LONG, EXAMPLE_DATE",
		BOUND);

	vector<Example> read_from_DB_before;

	copy(idxview.begin(), idxview.end(), back_inserter(read_from_DB_before));

	// examples that we want to insert into the DB ...
	// we want an all or nothing on these guys!
	// string indicates key of element to replace, Example is object to replace the object
	// to replace with
	map<string, Example> all_or_nothing_examples;

	// should succeed
	all_or_nothing_examples["Bedazzled"] = Example(79, "FUBAR", 2.2, 99, mikero);
	all_or_nothing_examples["Corwin"] = Example(81, "All Messed Up", 21.09, 75, chrysalis);
	all_or_nothing_examples["Mirror Image"] = Example(99, "Good One", 77.99, 41, victory);
	
	// must update all the elements to succeed in the transaction
	// else we rollback

	IdxDBV tmp(idxview); // make copy so we can rollback to idxview on failure

	// march through vector and replace elements appropriately
	try {
	  map<string, Example>::iterator map_it;
	  
	  for (map_it = all_or_nothing_examples.begin(); 
	       map_it != all_or_nothing_examples.end(); 
		   map_it++)
		  {	  
		     IdxDBV::iterator find_it = idxview.find((*map_it).first);

			 if (find_it != idxview.end())
				 idxview.replace(find_it, (*map_it).second);
		  }

      conn.CommitAll(); // we assume commit and rollback must always succeed to avoid two-phase commit type logic
	}
    catch(RootException &ex) 
	{ 
	  cout << ex << endl;
	  idxview.swap(tmp); // this will rollback to original results in memory
      conn.RollbackAll(); 
	}

  vector<Example> read_from_DB_after;

  copy(idxview.begin(), idxview.end(), back_inserter(read_from_DB_after));

  cout << "Local changes (to IndexedDBView) resulting from attempted range update:" << endl;
   
  TableDiff(cout, read_from_DB_before, read_from_DB_after);

  cout << endl;

  read_from_DB_after.clear();

  copy(view.begin(), view.end(), back_inserter(read_from_DB_after));

  cout << "Remote changes (to Database) resulting from attempted range update:" << endl;

  TableDiff(cout, read_from_DB_before, read_from_DB_after);
}
