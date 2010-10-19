// special query example

#include "SpecialQryExample.h"

const TIMESTAMP_STRUCT specialDate = {2001, 7, 12, 0, 0, 0, 0};

 template <class InputIterator, class OutputIterator>
  OutputIterator my_copy (InputIterator first, InputIterator last,
                       OutputIterator result)
  {
	 while (first != last) {
		 *result = *first;
		 result++; first++;
	 }
    return result;
  };

void SpecialQryExample()
{
	DBView<DistinctExample> view(DBView<DistinctExample>::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));

	// first print out the records from this view
	cout << "Query built by DBView<DistinctExample>::BuildQry() for select_iterator: " << endl;
	cout <<	view.begin().GetQuery() << endl;

	cout << "Selecting all records ..." << endl;
	
	vector<DistinctExample> origResults;


	my_copy(view.begin(), view.end(), back_inserter(origResults));
	copy(origResults.begin(), origResults.end(),
		ostream_iterator<DistinctExample>(cout, "\n"));

	// now insert a record
	DistinctExample insertMe = DistinctExample(712, "Distinct Example", 7.12, 7120, specialDate);
		
	cout << "Now inserting " << insertMe << " into the view!" << endl;

	DBView<DistinctExample>::insert_iterator ins_it = view;

	*ins_it = insertMe;

	ins_it++;

	cout << "Query built by DBView<DistinctExample> for insert_iterator: " << endl;
	cout << ins_it.GetQuery() << endl;

	cout << "View now has: " << endl;
	copy(view.begin(), view.end(), ostream_iterator<DistinctExample>(cout, "\n"));

	cout << "TableDiff() should show exactly one record inserted!" << endl;

	TableDiff(cout, origResults, view);
}


