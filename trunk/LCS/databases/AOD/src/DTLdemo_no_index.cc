//  DTLdemo_no_index.cc: example how to use the non-indexed DBView of DTL.
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
#include "RandomDBView.h"
#include <iostream>
using namespace dtl;
using namespace std;

// DTL also works on structs (which is a kind of 'public' class).
// To see this, change the 0 below into a 1
#define USE_STRUCTS		0
//
// construct our DataObject
//
#if USE_STRUCTS
struct AllFlds {
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
};
#else
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
#endif	// USE_STRUCTS

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
		cols["timestamp"] << rowbuf.theDate;	
		cols["f10"]		  << rowbuf.freq10;
		cols["f20"]		  << rowbuf.freq20;
		cols["f30"]		  << rowbuf.freq30;
		cols["f40"]		  << rowbuf.freq40;
		cols["f50"]		  << rowbuf.freq50;
		cols["f60"]		  << rowbuf.freq60;
		cols["f70"]		  << rowbuf.freq70;
		cols["f80"]		  << rowbuf.freq80;
		cols["f90"]		  << rowbuf.freq90;
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

// ---------------- Subroutines for testing the DTL layer ------------------
//
// FillTable
//
// Insert some example records in the database.
//
void FillTable() {
	cout << "Filling table.." << endl;

	DBView<AllFlds>						ins_view("DTL_example", InsertBCA());
	DBView<AllFlds>::insert_iterator	ins_iter = ins_view;

	const	TIMESTAMP_STRUCT			someDate = { 2003, 1, 1, 1, 0, 0, 0 };
#if USE_STRUCTS
	AllFlds								ins_rec = { someDate, 10.1, 20.2, 30.3, 40.4, 50.5, 60.6, 70.7, 80.8, 90.9 };
#else
	AllFlds								ins_rec = AllFlds (someDate, 10.1, 20.2, 30.3, 40.4, 50.5, 60.6, 70.7, 80.8, 90.9876543210123456789 );
#endif
	
	for (int i = 0; i < 10; ++i) {
		*ins_iter = ins_rec;				// INSERT it.
		ins_rec.theDate.hour += 1;			// Prepare the next record.
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
	
	vector<AllFlds>						results;
	DBView<AllFlds>						view("DTL_example", ShowBCA());
	DBView<AllFlds>::select_iterator	read_iter = view.begin();

	for	( ; read_iter != view.end(); ++read_iter) {
		results.push_back(*read_iter);
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
void SetSomeFields() {
	cout << "Modifying data.." << endl;
	
	vector<AllFlds>						results;
	DBView<AllFlds>						view("DTL_example",SetBCA(), 
													"WHERE F10>15");
	DBView<AllFlds>::update_iterator	upd_iter = view;
	AllFlds								row;

	row.freq10 = 100;
	row.freq20 = 200;
	row.freq30 = 300;
	*upd_iter = row;

}

//
// UpdateFunction
//
// Used by UpdateSomeFields
//
class UpdateFunction : public std::unary_function<Flds2Update, Flds2Update> {
public:
	Flds2Update operator() (const Flds2Update &f2uRecord) {
		Flds2Update		row(f2uRecord);
		row.freq10++;
		row.freq20 += 200;
		return row;
	}
};

//
// UpdateSomeFields
//
// Some experiment where the unique OID of the records is used in
// updating the records.
//
void UpdateSomeFields() {
	cout << "Updating data.." << endl;
	
	DBView<Flds2Update>							view("DTL_example",UpdateBCA());
	DBView<Flds2Update>::select_update_iterator	upd_iter = view.begin();
	upd_iter.SetKey("oid");		// @@@

	std::transform(view.begin(), view.end(), upd_iter, UpdateFunction());

}


//
// DeleteSomeRows
//
void DeleteSomeRows() {
	cout << "Deleting data.." << endl;
	
	DBView<AllFlds>						view("DTL_example",DeleteBCA());
	DBView<AllFlds>::delete_iterator		del_iter = view;
	AllFlds								del_cond;

	del_cond.freq10 = 100;				// DTL constructs: WHERE F10=100

	*del_iter = del_cond;				// DELETE them all!
	
	cout << del_iter.GetLastCount() << " rows deleted!" << endl;


}

//
// RandomAccess
//
// Demonstrates the (non)possibilities of a RandomDBView
//

// OSTREAM operator for easy output
//
ostream &operator <<(ostream &s, const AllFlds	&v)
{
	return s << v.theDate << "	" << v.freq10 << "	" << v.freq20 << "	" << v.freq30;
}

void RandomAccess() {
	cout << "Random accessing data.." << endl;
	
	DBView<AllFlds>					base_view("DTL_example",ShowBCA(),"ORDER BY TIMESTAMP");
	RandomDBView<AllFlds>			view(base_view);

	cout << "Show forwards" << endl;
	copy(view.begin(), view.end(), ostream_iterator<AllFlds>(cout, "\n"));

	//
	// Try to modify some records. This doesn't work as it should!
	//
	RandomDBView<AllFlds>::iterator	iter = view.begin();
	int		max = view.size();
	iter.SetKey("timestamp");
	for (int i = 0; i < max; i++) {
		// NOTE: instead of using row = *iter and iter++ we should be able
		// to use row = iter[i] without iter++ but this messes up the database
		// So the surplus value of a RandomDBView is negligible.
//		AllFlds		row(iter[i]);
		AllFlds		row = *iter;		// <<------

		row.freq30 = i;
		row.theDate.minute ++;

//		iter[i] = row;
		*iter = row;					// <<------
		iter++;							// <<------
	}

	view.ReQuery();

	cout << "Show backwards" << endl;
	copy(view.rbegin(), view.rend(), ostream_iterator<AllFlds>(cout, "\n"));
	
	//
	// Reading the record in a random way does work.
	//
	cout << "Show random" << endl;
	int		seq[10] = { 4, 2, 7, 0, 1, 3, 9, 8, 5, 6 };
	iter = view.begin();
	for (int j = 0; j < 10; j++) {
		cout << iter[seq[j]] << endl;
	}

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
		DBStmt("CREATE TABLE dtl_example ( \
				timestamp	TIMESTAMP NOT NULL PRIMARY KEY, 	\
				f10 		FLOAT, 	\
				f20 		FLOAT, 	\
				f30 		FLOAT, 	\
				f40 		FLOAT, 	\
				f50 		FLOAT, 	\
				f60 		FLOAT, 	\
				f70 		FLOAT, 	\
				f80 		FLOAT, 	\
				f90 		FLOAT)").Execute();

		FillTable();
		ShowSomeFields();

		UpdateSomeFields();
		ShowSomeFields();

		SetSomeFields();
		ShowSomeFields();

		RandomAccess();

		DeleteSomeRows();
		ShowSomeFields();
	}
	catch (std::exception &ex)
	{
		cerr << ex.what() << endl;
	}

	return 0;
}
