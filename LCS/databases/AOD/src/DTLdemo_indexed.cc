//  DTLdemo_indexed.cc: Example how to use the indexed views from DTL.
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include "DTL.h"
#include <iostream>
using namespace dtl;
using namespace std;

//
// construct our DataObject
//
class AllFlds {
public:
	TIMESTAMP_STRUCT		theDate;
	double					freq10;
	double					freq20;
	double					freq30;
	double					freq40;
	double					freq50;
	double					freq60;
	double					freq70;
	double					freq80;
	double					freq90;

	// Define a constructor with all database related fields
	// This constructor is not necessary but (in this demo) it 
	// simplifies the implementation of inserting records 
	AllFlds(const TIMESTAMP_STRUCT	&exDate,
			double					exf10,
			double					exf20,
			double					exf30,
			double					exf40,
			double					exf50,
			double					exf60,
			double					exf70,
			double					exf80,
			double					exf90) :
		theDate(exDate), freq10(exf10), freq20(exf20), freq30(exf30),
		freq40(exf40), freq50(exf50), freq60(exf60),
		freq70(exf70), freq80(exf80), freq90(exf90)
	{ }

	// Define the standard constructor
	AllFlds() :
		freq10(0), freq20(0), freq30(0),
		freq40(0), freq50(0), freq60(0),
		freq70(0), freq80(0), freq90(0)
	{
		cout << "##constructor##" << endl;
	}

};

// In indexed-views the underlying DBView is used as a parameter.
// Defining a type for it simplifies the implementation.
// Note: in the 2nd typedef the AllFlds class is also used as ParamObj
//		 which is no problem because the BPA determines which fields
//		 are used.
typedef DBView<AllFlds>					DBV_AF;
typedef DBView<AllFlds, AllFlds>		DBV_AFAF;


BEGIN_DTL_NAMESPACE
//
// Define BCA's for insertion, modifications, readings, etc.
// We use different BCA because we dont want to use the same
// fields every time.
//
class InsertBCA {
public:
	void operator() (BoundIOs 			&cols, 
					 AllFlds			&rowbuf) {
		cout << "##bind InsertBCA##" << endl;
		cols["timestamp"] == rowbuf.theDate;	
		cols["f10"]		  == rowbuf.freq10;
		cols["f20"]		  == rowbuf.freq20;
		cols["f30"]		  == rowbuf.freq30;
		cols["f40"]		  == rowbuf.freq40;
		cols["f50"]		  == rowbuf.freq50;
		cols["f60"]		  == rowbuf.freq60;
		cols["f70"]		  == rowbuf.freq70;
		cols["f80"]		  == rowbuf.freq80;
		cols["f90"]		  == rowbuf.freq90;
	}
};

class SetBCA {
public:
	void operator() (BoundIOs 			&cols, 
					 AllFlds			&rowbuf) {
		cout << "##bind SetBCA##" << endl;
		cols["f10"]		  == rowbuf.freq10;
		cols["f20"]		  == rowbuf.freq20;
		cols["f30"]		  == rowbuf.freq30;
	}
};

class SetBPA {
public:
	void operator() (BoundIOs 			&parpos, 
					 AllFlds			&paramobj) {
		cout << "##bind SetBPA##" << endl;
		parpos[0]  == paramobj.freq10;
	}
};

class ShowBCA {
public:
	void operator() (BoundIOs 			&cols, 
					 AllFlds			&rowbuf) {
		cout << "##bind ShowBCA##" << endl;
		cols["timestamp"] >> rowbuf.theDate;	
		cols["f10"]		  >> rowbuf.freq10;
		cols["f20"]		  >> rowbuf.freq20;
		cols["f30"]		  >> rowbuf.freq30;
	}
};

class DeleteBCA {
public:
	void operator() (BoundIOs 			&cols, 
					 AllFlds			&rowbuf) {
		cout << "##bind DeleteBCA##" << endl;
		cols["f10"]		  << rowbuf.freq10;
	}
};

//
// The database uses a unique ObjectID field for each item in a table.
// When we want to use this field is must be added to the DATAOBJECT
// we use.
struct Flds2Update {
	long					oid;
	double					freq10;
	double					freq20;
	double					freq30;
	// skip the other fields, its just a test
};

class UpdateBCA {
public:
	void operator() (BoundIOs 			&cols, 
					 Flds2Update		&rowbuf) {
		cout << "##bind UpdateBCA##" << endl;
		cols["oid"] 	  == rowbuf.oid;
		cols["f10"]		  == rowbuf.freq10;
		cols["f20"]		  == rowbuf.freq20;
		cols["f30"]		  == rowbuf.freq30;
	}
};
END_DTL_NAMESPACE
// Also define a type for a DBView based on this class.
typedef DBView<Flds2Update>				DBV_UF;

// ---------------- Subroutines for testing the DTL layer ------------------
//
// FillTable
//
// Insert some example records in the database.
//
void FillTable() {
	cout << "Filling table.." << endl;

	DBV_AF							ins_view("DTL_example", InsertBCA());
	IndexedDBView<DBV_AF>			indexed_view (ins_view, 
										 "UNIQUE PrimaryIndex; timestamp",
										 BOUND,
										 USE_PK_FIELDS_ONLY);
	IndexedDBView<DBV_AF>::iterator	iter;
	pair<IndexedDBView<DBV_AF>::iterator, bool>	ins_pair;

	const	TIMESTAMP_STRUCT		someDate = { 2003, 1, 1, 1, 0, 0, 0 };

	// NOTE: Here we use the extra constructor off the AllFlds class.
	AllFlds							ins_rec = AllFlds (someDate, 10.1, 20.2, 30.3, 40.4, 50.5, 60.6, 70.7, 80.8, 90.9 );

	for (int i = 0; i < 10; ++i) {
		ins_pair = indexed_view.insert(ins_rec);	// INSERT it.
		ins_rec.theDate.hour += 1;					// Prepare the next record
		ins_rec.freq10  += 1;
		ins_rec.freq20  += 1;
		ins_rec.freq30  += 1;
		ins_rec.freq40  += 1;
		ins_rec.freq50  += 1;
		ins_rec.freq60  += 1;
		ins_rec.freq70  += 1;
		ins_rec.freq80  += 1;
		ins_rec.freq90  += 1;
	}
}

//
// ReadSomeFields
//
// Fetches some fields of all records of the database and returns them
// in a vector.
//
// WARNING: the vector contains AllFlds records but only the
// fields mentioned in the ShowBCA class are filled!
//
vector<AllFlds> ReadSomeFields() {
	cout << "Fetching data.." << endl;
	
	DBV_AF							view("DTL_example", ShowBCA());
	IndexedDBView<DBV_AF>				indexed_view (view, 
										 "PrimaryIndex; f10",
										 BOUND,
										 USE_PK_FIELDS_ONLY);
	IndexedDBView<DBV_AF>::iterator	iter;
	vector<AllFlds>					results;

	iter = indexed_view.begin();
	for	( ; iter != indexed_view.end(); ++iter) {
		results.push_back(*iter);
	}
	return results;
}

//
// Show some fields
//
void ShowSomeFields() {
	// show table contents
	cout << "Showing table.." << endl;

	vector<AllFlds>				theContents = ReadSomeFields();
	vector<AllFlds>::iterator	iter;

	cout << "Found " << theContents.size() << " records" << endl;
	for (iter =theContents.begin(); iter != theContents.end(); ++iter) {
		// Remember: ReadSomeFields uses the ShowBCA class to fetch
		// the records, so only these fields are available.
		cout <<  iter->theDate << "	";
		cout <<  iter->freq10  << "	";
		cout <<  iter->freq20  << "	";
		cout <<  iter->freq30  << endl;
	}
}


//
// SetSomeFields
//
// Makes a selection in the database and sets the contents of 
// some fields to a constant value.
void SetParamsSetBPA (AllFlds &sel_crit)
{
	cout << "##setparams##" << endl;
	sel_crit.freq10 = 15;
}

void SetSomeFields() {
	cout << "Modifying data.." << endl;
	
	DBV_AFAF							mod_view("DTL_example", SetBCA(),
											"WHERE F10>(?)", SetBPA());
	IndexedDBView<DBV_AFAF>				indexed_view (mod_view, 
											 "PrimaryIndex; f10",
											 BOUND,
											 USE_PK_FIELDS_ONLY,
											 cb_ptr_fun(SetParamsSetBPA));
	IndexedDBView<DBV_AFAF>::iterator	iter = indexed_view.begin();
	// Replace() returns a 'pair' to reflect the result.
	pair<IndexedDBView<DBV_AFAF>::iterator, bool>	upd_pair;	

	cout << "nr of records to change: " << indexed_view.size() << endl;

	AllFlds		row;
	while (iter != indexed_view.end()) {	// while any records left
		row		   = *iter;					// make local copy
		row.freq10 = 100;					// modify copy
		row.freq20 = 200;
		row.freq30 = 300;

		// NOTE: iterator MUST be postfix incremented because
		//       the iterator is destroyed in the replace !!!!
		upd_pair = indexed_view.replace(iter++, row);	// update record

		cout << (upd_pair.second == true ? "s" : "uns") << "uccesfull" <<  endl;
	}
}

//
// UpdateSomeFields
//
// Some experiment where the unique OID of the records is used in
// updating the records.
//
void UpdateSomeFields() {
	cout << "Updating data.." << endl;
	
	DBV_UF							upd_view("DTL_example", UpdateBCA());
	IndexedDBView<DBV_UF>			indexed_view (upd_view, 
											 "PrimaryIndex; oid",
											 BOUND,
											 USE_PK_FIELDS_ONLY);
	IndexedDBView<DBV_UF>::iterator		iter = indexed_view.begin();;	
	pair<IndexedDBView<DBV_UF>::iterator, bool>	upd_pair;	

	cout << "nr of records to change: " << indexed_view.size() << endl;

	Flds2Update		row;
	while (iter != indexed_view.end()) {	// while any records left
		row		   = *iter;					// make local copy
		row.freq10++;						// modify copy
		row.freq20 += 200;

		// NOTE: iterator MUST be postfix incremented because
		//       the iterator is destroyed in the replace !!!!
		upd_pair = indexed_view.replace(iter++, row);	// update record

		cout << (upd_pair.second == true ? "s" : "uns") << "uccesfull" <<  endl;
	}

	// When auto-commit is off uncomment the following line
//	DBConnection::GetDefaultConnection().CommitAll();
}


//
// DeleteSomeRows
//
void DeleteSomeRows() {
	cout << "Deleting data.." << endl;
	
	DBV_UF							view("DTL_example", UpdateBCA(),
											"WHERE F10=100");
	IndexedDBView<DBV_UF>			indexed_view (view, 
										 "PrimaryIndex; oid",
										 BOUND,
										 USE_PK_FIELDS_ONLY);
	IndexedDBView<DBV_UF>::iterator	iter = indexed_view.begin();

	// NOTE: The WHERE clause of the view is only used for making a
	// 		 selection of the records which must be deleted.
	//		 Within this selection there must be a uniq field for
	//		 deleting the records, we use OID for this.

	cout << indexed_view.size() << " rows to delete!" << endl;

	// Three ways to get rid of the records:
#if 0
	// This works but is a bit clumsy
	while (iter != indexed_view.end()) {
		// Iterator is destoyed here also
		IndexedDBView<DBV_UF>::iterator	tmp_iter;
		tmp_iter = iter++;
		indexed_view.erase(tmp_iter);		// DELETE them one by one
	}
#endif

	// This is much nicer and work also: DELETE the whole range
	indexed_view.erase(indexed_view.begin(), indexed_view.end());

#if 0
	// Too bad this does not work! (No SQL statment is generated).
	indexed_view.clear();					// DELETE them all at once
#endif
}


//
// MAIN
//
int main ()
{
	try {
		cout << "Connecting to database.." << endl;
		DBConnection::GetDefaultConnection().Connect("UID=postgres;DSN=pg50");

		cout << "Dropping old table.." << endl;
		DBStmt("DROP TABLE dtl_example").Execute();

		cout << "Creating new table.." << endl;
		DBStmt("CREATE TABLE dtl_example (
				timestamp	TIMESTAMP NOT NULL PRIMARY KEY, 
				f10 		FLOAT, 
				f20 		FLOAT, 
				f30 		FLOAT, 
				f40 		FLOAT, 
				f50 		FLOAT, 
				f60 		FLOAT, 
				f70 		FLOAT, 
				f80 		FLOAT, 
				f90 		FLOAT)").Execute();

		FillTable();
		ShowSomeFields();

		UpdateSomeFields();
		ShowSomeFields();

		SetSomeFields();
		ShowSomeFields();

		DeleteSomeRows();
		ShowSomeFields();
	}
	catch (std::exception &ex)
	{
		cerr << ex.what() << endl;
	}

	return 0;
}
