#include "CStringExample.h"
#include <exception>

void CStringExample()
{
	PrintHeader(cout, "CStringExample()");

	// insert a example with null date into the DB
	DynamicDBView<> add_null_view("DB_EXAMPLE", "*");
	DynamicDBView<>::insert_iterator null_ins_it(add_null_view);

	variant_row vr(add_null_view.GetDataObj());

    vr["INT_VALUE"] = 4443;
	vr["STRING_VALUE"] = string("Null Me");
	vr["DOUBLE_VALUE"] = 33.3;
	vr["EXAMPLE_LONG"] = 444;
	vr["EXAMPLE_DATE"] = NullField();

	cout << "Inserting following Example into DB" << endl;
	cout << vr << endl;

	*null_ins_it = vr;
	++null_ins_it;

	DBConnection::GetDefaultConnection().CommitAll();

	cout << "*** regular fetch ***" << endl;
	DBView<ExampleCStr> view(DBView<ExampleCStr>::Args().tables("DB_EXAMPLE").SelValidate(SelValNullData()).postfix(exampleOrderBy));
	copy(view.begin(), view.end(), ostream_iterator<ExampleCStr>(cout, "\n"));

	cout << "*** bulk fetch ***" << endl;
	vector<ExampleCStr> examples;

	DBView<ExampleCStr>::select_iterator sel_it(view.begin());
	bulk_fetch_helper(sel_it, 100, back_inserter(examples));

	copy(examples.begin(), examples.end(), ostream_iterator<ExampleCStr>(cout, "\n"));

	cout << "----" << endl;

	cout << "*** Regular insert ***" << endl;

	DBView<ExampleCStr>::insert_iterator ins_it(view);

	copy(&examples[0], &examples[3], ins_it);

	copy(view.begin(), view.end(), ostream_iterator<ExampleCStr>(cout, "\n"));
	
	cout << "---" << endl;

	cout << "*** Bulk insert ***" << endl;
#if !defined (__SUNPRO_CC)
	try {
		bulk_insert_helper(&examples[3], &examples[13], 10, ins_it);
	}
	catch (std::exception &ex) {
		cout << ex.what() << endl;
		cout << "Error. Bulk insert / row-wise arrays of parameters is not supported by this ODBC driver" << endl;
	}
#else
	cout << "Bulk insert gives core dump under sun workshop?? Disabled for now." << endl;
#endif

	cout << "---" << endl;

	copy(view.begin(), view.end(), ostream_iterator<ExampleCStr>(cout, "\n"));

	cout << "*** indexed view fetch ***" << endl;

	typedef DEFAULT_INDEX_VIEW(DBView<ExampleCStr>) IdxDBV;

	IdxDBV idx_view(
		IdxDBV::Args().view(view).indexes( 
		   " PrimaryIndex ; STRING_VALUE ; UNIQUE AlternateIndex ; EXAMPLE_LONG , EXAMPLE_DATE"
		)
	);

	copy(idx_view.begin(), idx_view.end(), ostream_iterator<ExampleCStr>(cout, "\n"));

	cout << "*** indexed view equal_range ***" << endl;
	pair<IdxDBV::iterator, IdxDBV::iterator> pr =
		idx_view.equal_range(tcstring50_t("Find Me"));

	copy(pr.first, pr.second, ostream_iterator<ExampleCStr>(cout, "\n"));
	
	
	PrintSeparator(cout);
}
