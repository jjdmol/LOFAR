// example showing the use of an indexed view in the dynamic case

#include "DynamicIndexedViewExample.h"

const TIMESTAMP_STRUCT then = {2000, 12, 15, 0, 0, 0, 0};

// Dynamic IndexedDBView example
void DynamicIndexedViewExample()
{
  typedef DynamicDBView<ParamObjExample>  DynaDBV;
  typedef DEFAULT_DYNA_VIEW(DynaDBV)      DynaIdxDBV;

  DynaDBV dynamic_view("DB_EXAMPLE",
		       "INT_VALUE, STRING_VALUE, DOUBLE_VALUE, EXAMPLE_LONG,  EXAMPLE_DATE",
		       "WHERE INT_VALUE BETWEEN (?) AND (?) OR "
		       "EXAMPLE_DATE <= (?) OR STRING_VALUE = (?)" + exampleOrderBy,
		       BPAExampleObj());

  // make the functor needed for SetParams out of SetParamsExample() by calling
  // cb_ptr_fun(SetParamsExample)
  DynaIdxDBV indexed_view(dynamic_view, 
						  "PrimaryIndex; STRING_VALUE;"
						  "UNIQUE IndexLongDate; EXAMPLE_LONG, EXAMPLE_DATE",
						  BOUND, USE_ALL_FIELDS, 
						  cb_ptr_fun(SetParamsExample));
  
  // Find the item where the STRING_VALUE matches the string  "Foozle"
  DynaIdxDBV::indexed_iterator  idxview_it = 
	indexed_view.find(string("Foozle"));
  
  
  // Update the item with the key of "Foozle", to read  "Fizzle" instead
  if (idxview_it != indexed_view.end()) {
	variant_row replacement;
	replacement = *idxview_it;
	replacement["STRING_VALUE"] = string("Fizzle");
	TIMESTAMP_STRUCT date = {2003, 3, 3, 0, 0, 0, 0};
	replacement["EXAMPLE_DATE"] = date;
	indexed_view.replace(idxview_it, replacement);
  }
  
  // now find the record with STRING_VALUE "Mirror Image"
  DynaIdxDBV::indexed_iterator idxview_it2 =
	indexed_view.find(string("Mirror Image"));
	
  // Update the item with the key of "Mirror Image", to read "Egami Rorrim",
  // then "Alice in Wonderland" instead (this last one will appear in the TableDiff())
  if (idxview_it2 != indexed_view.end()) {

	// be careful here!  You would think here that you can reuse idxview_it,
	// but the first call to IndexedDBView::replace() invalidates this iterator ...
	// so make sure to grab the iterator to the updated element in the indexed view
	
	variant_row replacement;
	replacement = *idxview_it2;
	replacement["STRING_VALUE"] = string("Egami Rorrim");
	
	pair<DynaIdxDBV::indexed_iterator, bool> 
	  pr = indexed_view.replace(idxview_it2, replacement);
	
	replacement["STRING_VALUE"] = string("Alice in Wonderland");
	
	// indexed_view.replace(idxview_it2, replacement); // NO! NO! NO!
	// idxview_it2 is invalid
	
	// pr.first is an iterator to the updated element, so we may use it
	// for the IndexedDBView::replace() call
	indexed_view.replace(pr.first, replacement);
  }
  
  // Now find a second set of items using AlternateIndex
  // The STL convention for equal_range is to return a pair  consisting of: 
  // 1. an iterator referring to the beginning of the list of found  items
  // 2. an iterator pointing to the end of the list of found items. 
  // We will remove all items in this range.
  const TIMESTAMP_STRUCT date_criteria = {2000, 1, 1, 0, 0, 0, 0};
  long long_criteria = 33;
  DynaIdxDBV::indexed_pair pr =
	indexed_view.equal_range_AK("IndexLongDate", long_criteria, date_criteria);

  idxview_it = pr.first;

  cout << "*** Size before erase calls: " <<  indexed_view.size() << " ***"
	   << endl;

  // Remove all rows that matched the criteria in our  equal_range_AK lookup
  while (idxview_it != pr.second)
  {
	// as iterator is invalidated upon an erase(), use a temporary   iterator
	// to point to DataObj to erase
	// increment idxview_it before we erase so it will still be valid
	// when we erase the DataObj
	DynaIdxDBV::indexed_iterator deleteMe = idxview_it;

	++idxview_it;

	indexed_view.erase(deleteMe);
	
  }
  
  cout << "*** Size after erase calls: " <<  indexed_view.size() << " ***"
	   << endl;


  // Finally, insert a new item into the container
  pair<DynaIdxDBV::iterator, bool> ins_pr;
  variant_row r(indexed_view.GetDataObj());
  r["INT_VALUE"] = 459;
  r["STRING_VALUE"] = string("Unique String  #1");
  r["DOUBLE_VALUE"] = 3.5;
  r["EXAMPLE_LONG"] = NullField();
  r["EXAMPLE_DATE"] = date_criteria;
  ins_pr = indexed_view.insert(r);
  cout << "insertion succeeded = " 
	   <<  (ins_pr.second == true ? "true": "false") 
	   << endl;
  
  cout << "Testing Nullifier for a variant_row" << endl;
  cout << "Using EXAMPLE_DATE on copy of just inserted object as the guinea pig" << endl;
  r["EXAMPLE_DATE"] = NullField();
  cout << r << endl;
}

