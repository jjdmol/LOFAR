// example illustrating the use of an IndexedDBView for Example objects

#include "IndexedViewExample.h"
#include "vec_multiset.h"

const TIMESTAMP_STRUCT then = {2000, 12, 15, 0, 0, 0, 0};


// Example of using an IndexDBView to read, insert and update records in a container / database
void IndexedViewExample()
{
	typedef DBView<Example, ParamObjExample> DBV;

	DBV view("DB_EXAMPLE", DefaultBCA<Example>(), 
	  "WHERE INT_VALUE BETWEEN (?) AND (?) OR "
	  "EXAMPLE_DATE <= (?) OR STRING_VALUE = (?)" + exampleOrderBy,
	  BPAExampleObj());

	view.set_io_handler(LoggingHandler<Example, ParamObjExample>());

	typedef IndexedDBView<DBV, multiset<Example *, CBFunctor2wRet<const Example *,
			const Example *, bool> >, NO_HASH> IndexedDBV;

	// make the functor needed for SetParams out of SetParamsExample() by calling
	// cb_ptr_fun(SetParamsExample)
	IndexedDBV indexed_view(view, " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE", 
		BOUND, USE_ALL_FIELDS, 
		cb_ptr_fun(SetParamsExample));

	try
	{
	
	cout << "Outputting initial indexed view contents ..." << endl;
	cout << "(Partly to show that overloaded ContainerFactory() works)" << endl;
	cout << "(which uses reverse alphabetical order for STRING_VALUE and GenericCmp() otherwise)" << endl;

	copy(indexed_view.begin(), indexed_view.end(), ostream_iterator<Example>(cout, "\n"));

	// Find the item where the STRING_VALUE matches the string "Foozle"
	IndexedDBV::indexed_iterator idxview_it = indexed_view.find(string("Foozle"));
		

	// Update the item with the key of "Foozle", to read "Fizzle" instead
	if (idxview_it != indexed_view.end()) {
		Example replacement;
		replacement = *idxview_it;
		replacement.SetExampleStr("Fizzle");
		TIMESTAMP_STRUCT date = {2003, 3, 3, 0, 0, 0, 0};
		replacement.SetExampleDate(date);
		indexed_view.replace(idxview_it, replacement);
	}

	// now find the record with STRING_VALUE "Mirror Image"
	IndexedDBV::indexed_iterator idxview_it2 = indexed_view.find(string("Mirror Image"));
	
	// Update the item with the key of "Mirror Image", to read "Egami Rorrim",
	// then "Alice in Wonderland" instead (this last one will appear in the TableDiff())
	if (idxview_it2 != indexed_view.end()) {

		// be careful here!  You would think here that you can reuse idxview_it,
		// but the first call to IndexedDBView::replace() invalidates this iterator ...
		// so make sure to grab the iterator to the updated element in the indexed view

		Example replacement;
		replacement = *idxview_it2;
		replacement.SetExampleStr("Egami Rorrim");

		pair<IndexedDBV::iterator, bool> pr =
			indexed_view.replace(idxview_it2, replacement);



		replacement.SetExampleStr("Alice in Wonderland");
		
		// indexed_view.replace(idxview_it2, replacement); // NO! NO! NO!
														   // idxview_it2 is invalid

		// pr.first is an iterator to the updated element, so we may use it
		// for the IndexedDBView::replace() call
		indexed_view.replace(pr.first, replacement);
	}

	// Now find a second set of items using AlternateIndex
	// The STL convention for equal_range is to return a pair consisting of:  
	// 1. an iterator referring to the beginning of the list of found items
	// 2. an iterator pointing to the end of the list of found items. 
	// We will remove all items in this range.
	const TIMESTAMP_STRUCT ts_date_criteria = {2000, 1, 1, 0, 0, 0, 0};
	jtime_c date_criteria(ts_date_criteria);
	long long_criteria = 33;
	IndexedDBV::indexed_pair pr = indexed_view.equal_range_AK ("AlternateIndex", long_criteria, date_criteria);

	idxview_it = pr.first;

	cout << "*** Size before erase calls: " << indexed_view.size() << " ***" << endl;
		
	// Remove all items that match the criteria in our equal_range_AK lookup
	while (idxview_it != pr.second)
	{
		// As iterator is invalidated upon an erase(), use a
		// temporary iterator to point to DataObj to erase.
		// Increment idxview_it before we erase so it will still be valid
		// when we erase the DataObj.
		IndexedDBV::iterator deleteMe = idxview_it;

		++idxview_it;

		indexed_view.erase(deleteMe);

	}

	cout << "*** Size after erase calls: " << indexed_view.size() << " ***"
		 << endl;


	// Insert a new item into the container
	pair<IndexedDBV::iterator, bool> ins_pr;

    cout << "We will now try to insert three items.  Only the first item will succeed!" << endl;

	ins_pr = indexed_view.insert(Example(459, "Unique String #1", 3.4, 1, date_criteria));

	cout << "insertion succeded = " << (ins_pr.second == true ? "true": "false") << endl;

	ins_pr = indexed_view.insert(Example(311, "", 3.99, 91, then)); // should fail on InsValidate()
   
	cout << "insertion succeded = " << (ins_pr.second == true ? "true": "false") << endl;
	
	ins_pr = indexed_view.insert(Example(222, "Positron", -34.77, 29, then)); // should fail (ditto)
	
	cout << "insertion succeded = " << (ins_pr.second == true ? "true": "false") << endl;
	}
	
	catch (...)
	{

	typedef LoggingHandler<Example, ParamObjExample>::LoggedTriple LoggedTriple;
 
	// retrieve the LoggingHandler object from the IndexedDBView
	LoggingHandler<Example, ParamObjExample> log_handler = 
		indexed_view.get_io_handler((LoggingHandler<Example, ParamObjExample> *) NULL);

    // the log is a vector of (error message, DataObj, ParamObj) triples,
    // (error message, Example object, ParamObjExample object) in this case
    // the error itself along with the relevant DataObj and ParamObj that resulted with
    // the error
    vector<LoggedTriple> error_log = log_handler.GetLog();

    // nothing to do if no errors occurred
    if (error_log.empty())
	    return;

    cout << "*** Error Log in IndexedViewExample(): " << error_log.size() << " errors recorded! ***"
	     << endl;

    // print out the errors
    for (vector<LoggedTriple>::const_iterator log_it = error_log.begin(); 
		log_it != error_log.end(); log_it++)
	{
       cout << "*** Error Log Entry ***" << endl;
	   cout << "* error message *" << endl;
	   cout << (*log_it).errmsg << endl;
	   cout << "* relevant Example object *" << endl;
	   cout << (*log_it).dataObj << endl;
	}

	}	
}

// Example of reading from a IndexedDBView which uses default comparisons
void SimpleReadFromIndexedView()
{
	typedef DBView<Example, ParamsSimpleExample> DBV;
	typedef DEFAULT_INDEX_VIEW(DBV)              IdxDBV;

	DBV view("DB_EXAMPLE", DefaultBCA<Example>(), 
	  "WHERE INT_VALUE BETWEEN (?) AND (?) OR "
	  "EXAMPLE_DATE <= (?) OR STRING_VALUE = (?)" + exampleOrderBy,
	  BPASimpleExampleObj());

	cout << "Reading some records from DB_EXAMPLE and indexing first by"<< endl;
	cout << "single field key STRING_VALUE and then by" << endl;
	cout << "unique two field key EXAMPLE_LONG and EXAMPLE_DATE  ..." << endl;

	IdxDBV indexed_view(
		IdxDBV::Args().view(view).indexes(
		    " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE"
		).bound(BOUND).params(cb_ptr_fun(SetParamsSimpleExample))
	);

	cout << "Outputting indexed view contents ..." << endl;
	cout << "(Partly to show that overloaded ContainerFactory() works)" << endl;
	cout << "(which uses GenericCmp())" << endl;

	copy(indexed_view.begin(), indexed_view.end(), ostream_iterator<Example>(cout, "\n"));
}

// Even simpler example ... query with no postfix clause ...
void VerySimpleReadFromIndexedView()
{
	typedef DBView<Example>             DBV;
	typedef DEFAULT_INDEX_VIEW(DBV)     IdxDBV;

	DBV view(DBV::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));

    cout << "Connected to: " << view.GetConnection().GetDBMSName()
		 << " Version: " << view.GetConnection().GetDBMSVersion() << endl;
	cout << "Reading all records from DB_EXAMPLE and indexing first by"<< endl;
	cout << "single field key STRING_VALUE and then by" << endl;
	cout << "unique two field key EXAMPLE_LONG and EXAMPLE_DATE  ..." << endl;

	IdxDBV indexed_view(view, " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE");

	cout << "Outputting indexed view contents ..." << endl;
	cout << "(Partly to show that overloaded ContainerFactory() works)" << endl;
	cout << "(which uses GenericCmp())" << endl;

	copy(indexed_view.begin(), indexed_view.end(), ostream_iterator<Example>(cout, "\n"));
}

#ifdef __SGI_STL_PORT
// Even simpler example ... query with no postfix clause ...
void VerySimpleReadFromHashedIndexedView()
{
	typedef DBView<Example> DBV;

	typedef eq_functor<Example> EqualsType;
	typedef hash_functor<Example> HashType;
	typedef hash_multiset<Example *, HashType, EqualsType> HMSType; 
	
	DBV view(DBV::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));

    cout << "Connected to: " << view.GetConnection().GetDBMSName()
		   << " Version: " << view.GetConnection().GetDBMSVersion() << endl;
	cout << "Reading all records from DB_EXAMPLE and indexing first by"<< endl;
	cout << "single field key STRING_VALUE and then by" << endl;
	cout << "unique two field key EXAMPLE_LONG and EXAMPLE_DATE  ..." << endl;

	IndexedDBView<DBV, HMSType, HASH> 
		indexed_view(view, " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE");

	cout << "Outputting indexed view contents ..." << endl;
	cout << "(Partly to show that overloaded ContainerFactory() works)" << endl;
	cout << "(which uses GenericHash() and GenericCmp())" << endl;

	copy(indexed_view.begin(), indexed_view.end(), ostream_iterator<Example>(cout, "\n"));
}

// same example, but one that should call our specialized version
// of hashed ContainerFactory, and uses postfix clause
void SimpleReadFromHashedIndexedView()
{
	typedef DBView<Example, ParamObjExample> DBV;

	typedef CBFunctor2wRet<const Example *, const Example *, bool> EqualsType;
	typedef CBFunctor1wRet<const Example *, size_t> HashType;
	typedef hash_multiset<Example *, HashType, EqualsType> HMSType; 

	DBV view("DB_EXAMPLE", DefaultBCA<Example>(), 
	  "WHERE INT_VALUE BETWEEN (?) AND (?) OR "
	  "EXAMPLE_DATE <= (?) OR STRING_VALUE = (?)" + exampleOrderBy,
	  BPAExampleObj());

	cout << "Reading some records from DB_EXAMPLE and indexing first by"<< endl;
	cout << "single field key STRING_VALUE and then by" << endl;
	cout << "unique two field key EXAMPLE_LONG and EXAMPLE_DATE  ..." << endl;

	IndexedDBView<DBV, HMSType, HASH> 
		indexed_view(view, " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE", 
		BOUND, USE_ALL_FIELDS, 
		cb_ptr_fun(SetParamsExample));

	cout << "Outputting indexed view contents ..." << endl;
	cout << "(Partly to show that overloaded ContainerFactory() works)" << endl;
	cout << "(which uses alternative hasher my_hash_strings() for exampleStr" << endl;
	cout << " and GenericHash() and GenericCmp() in all other cases)" << endl;

	copy(indexed_view.begin(), indexed_view.end(), ostream_iterator<Example>(cout, "\n"));
}
#endif

// same example yet again, but used the custom associative container
// that is a sorted vector that emulates a multiset
void SimpleReadFromCustomIndexedView()
{
	typedef DBView<Example, ParamObjExample> DBV;

	typedef lt_functor<Example> LtType;

	typedef vec_multiset<Example *, LtType> MSType; 

	DBV view("DB_EXAMPLE", DefaultBCA<Example>(), 
	  "WHERE INT_VALUE BETWEEN (?) AND (?) OR "
	  "EXAMPLE_DATE <= (?) OR STRING_VALUE = (?)" + exampleOrderBy,
	  BPAExampleObj());

	cout << "Reading some records from DB_EXAMPLE and indexing first by"<< endl;
	cout << "single field key STRING_VALUE and then by" << endl;
	cout << "unique two field key EXAMPLE_LONG and EXAMPLE_DATE  ..." << endl;

	IndexedDBView<DBV, MSType, NO_HASH> 
		indexed_view(view, " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE", 
		BOUND, USE_ALL_FIELDS, 
		cb_ptr_fun(SetParamsExample));

	cout << "Outputting indexed view contents ..." << endl;
	cout << "(Partly to show that overloaded ContainerFactory() works)" << endl;
	cout << "(which uses GenericCmp() in all cases)" << endl;

	copy(indexed_view.begin(), indexed_view.end(), ostream_iterator<Example>(cout, "\n"));
}

// Example of using an IndexDBView to read, insert and update records in a container / database
void CustomIndexedViewExample()
{
	typedef DBView<Example, ParamObjExample> DBV;

	DBV view("DB_EXAMPLE", DefaultBCA<Example>(), 
	  "WHERE INT_VALUE BETWEEN (?) AND (?) OR "
	  "EXAMPLE_DATE <= (?) OR STRING_VALUE = (?)" + exampleOrderBy,
	  BPAExampleObj());

	view.set_io_handler(LoggingHandler<Example, ParamObjExample>());

	typedef IndexedDBView<DBV, vec_multiset<Example *, lt_functor<Example> >, NO_HASH> IndexedDBV;

	// make the functor needed for SetParams out of SetParamsExample() by calling
	// cb_ptr_fun(SetParamsExample)
	IndexedDBV indexed_view(view, " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE", 
		BOUND, USE_ALL_FIELDS, 
		cb_ptr_fun(SetParamsExample));

	// debug_examples used for debugging
	vector<Example> debug_examples;

	copy(indexed_view.begin(), indexed_view.end(), back_insert_iterator<vector<Example> >(debug_examples));

	try
	{
	
	// Find the item where the STRING_VALUE matches the string "Foozle"
	IndexedDBV::indexed_iterator idxview_it = indexed_view.find(string("Foozle"));
		

	// Update the item with the key of "Foozle", to read "Fizzle" instead
	if (idxview_it != indexed_view.end()) {
		Example replacement;
		replacement = *idxview_it;
		replacement.SetExampleStr("Fizzle");
		TIMESTAMP_STRUCT date = {2003, 3, 3, 0, 0, 0, 0};
		replacement.SetExampleDate(date);
		pair<IndexedDBV::indexed_iterator, bool> result =
			indexed_view.replace(idxview_it, replacement);
	}

	// now find the record with STRING_VALUE "Mirror Image"
	IndexedDBV::indexed_iterator idxview_it2 = indexed_view.find(string("Mirror Image"));
	
	// Update the item with the key of "Mirror Image", to read "Egami Rorrim",
	// then "Alice in Wonderland" instead (this last one will appear in the TableDiff())
	if (idxview_it2 != indexed_view.end()) {

		// be careful here!  You would think here that you can reuse idxview_it,
		// but the first call to IndexedDBView::replace() invalidates this iterator ...
		// so make sure to grab the iterator to the updated element in the indexed view

		Example replacement;
		replacement = *idxview_it2;
		replacement.SetExampleStr("Egami Rorrim");

		pair<IndexedDBV::iterator, bool> pr =
			indexed_view.replace(idxview_it2, replacement);

		Example result_example = *pr.first;

		replacement.SetExampleStr("Alice in Wonderland");
		
		// indexed_view.replace(idxview_it2, replacement); // NO! NO! NO!
														   // idxview_it2 is invalid

		// pr.first is an iterator to the updated element, so we may use it
		// for the IndexedDBView::replace() call
		pr = indexed_view.replace(pr.first, replacement);
	}

	// Now find a second set of items using AlternateIndex
	// The STL convention for equal_range is to return a pair consisting of:  
	// 1. an iterator referring to the beginning of the list of found items
	// 2. an iterator pointing to the end of the list of found items. 
	// We will remove all items in this range.
	const TIMESTAMP_STRUCT ts_date_criteria = {2000, 1, 1, 0, 0, 0, 0};
	jtime_c date_criteria(ts_date_criteria);
	long long_criteria = 33;
	IndexedDBV::indexed_pair pr = indexed_view.equal_range_AK ("AlternateIndex", long_criteria, date_criteria);

	idxview_it = pr.first;

	cout << "*** Size before erase calls: " << indexed_view.size() << " ***" << endl;
		
	// Remove all items that match the criteria in our equal_range_AK lookup
	while (idxview_it != pr.second)
	{
		// As iterator is invalidated upon an erase(), use a
		// temporary iterator to point to DataObj to erase.
		// Increment idxview_it before we erase so it will still be valid
		// when we erase the DataObj.
		IndexedDBV::iterator deleteMe = idxview_it;

		++idxview_it;

		indexed_view.erase(deleteMe);
	}

	cout << "*** Size after erase calls: " << indexed_view.size() << " ***"
		 << endl;


	// Insert a new item into the container
	pair<IndexedDBV::iterator, bool> ins_pr;

    cout << "We will now try to insert three items.  Only the first item will succeed!" << endl;

	ins_pr = indexed_view.insert(Example(459, "Unique String #1", 3.4, 1, date_criteria));

	cout << "insertion succeded = " << (ins_pr.second == true ? "true": "false") << endl;

	ins_pr = indexed_view.insert(Example(311, "", 3.99, 91, then)); // should fail on InsValidate()
   
	cout << "insertion succeded = " << (ins_pr.second == true ? "true": "false") << endl;
	
	ins_pr = indexed_view.insert(Example(222, "Positron", -34.77, 29, then)); // should fail (ditto)
	
	cout << "insertion succeded = " << (ins_pr.second == true ? "true": "false") << endl;
	}
	
	catch (...)
	{

	typedef LoggingHandler<Example, ParamObjExample>::LoggedTriple LoggedTriple;
 
	// retrieve the LoggingHandler object from the IndexedDBView
	LoggingHandler<Example, ParamObjExample> log_handler = 
		indexed_view.get_io_handler((LoggingHandler<Example, ParamObjExample> *) NULL);

    // the log is a vector of (error message, DataObj, ParamObj) triples,
    // (error message, Example object, ParamObjExample object) in this case
    // the error itself along with the relevant DataObj and ParamObj that resulted with
    // the error
    vector<LoggedTriple> error_log = log_handler.GetLog();

    // nothing to do if no errors occurred
    if (error_log.empty())
	    return;

    cout << "*** Error Log in IndexedViewExample(): " << error_log.size() << " errors recorded! ***"
	     << endl;

    // print out the errors
    for (vector<LoggedTriple>::const_iterator log_it = error_log.begin(); 
		log_it != error_log.end(); log_it++)
	{
       cout << "*** Error Log Entry ***" << endl;
	   cout << "* error message *" << endl;
	   cout << (*log_it).errmsg << endl;
	   cout << "* relevant Example object *" << endl;
	   cout << (*log_it).dataObj << endl;
	}

	}	
}

