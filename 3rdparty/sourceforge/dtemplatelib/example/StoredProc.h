// stored proc examples
#ifndef _STORED_PROC_H
#define _STORED_PROC_H

#include "example_core.h"

#if 0
class StoredProcBCA {
public:
 void operator()(BoundIOs &cols, variant_row &row)
 {
  cols[0] >> row._int();
  cols[1] >> row._string();
  cols[2] >> row._string();
  cols.BindVariantRow(row);
 }
};

#endif

class GetEmployeeDetailsBCA {
public:
 void operator()(BoundIOs &cols, variant_row &row)
 {
  cols[0] << row._int();
  cols[1] >> row._string();
  cols[2] >> row._string();
  cols[3] >> row._double();
  cols[4] >> row._timestamp();
  cols.BindVariantRow(row);
 }
};

#if 0
PROCEDURE GetEmployeeDetails(
	i_emID			IN		employee.em_id%TYPE,
    	o_FirstName 	        OUT 	        employee.em_first_name%TYPE,
    	o_LastName		OUT		employee.em_last_name%TYPE,
    	o_Salary		OUT		employee.em_salary%TYPE,
    	o_StartDate		OUT		employee.em_start_date%TYPE)
#endif


void StoredProcRead();

class ProcBCA {
public:
 void operator()(BoundIOs &cols, variant_row &row)
 {

  cols["INT_VALUE"] >> row._int();
  cols["STRING_VALUE"] >> row._string();
  
  cols.BindVariantRow(row);
 }
};

class ProcParams {
public:
	long long_criteria;

};

class ProcBPA {
public:
 void operator()(BoundIOs &cols, ProcParams &row)
 {
  cols[0] << row.long_criteria;
 }
};

// Read the contents of a table and print the resulting rows
// *** you must have Oracle ODBC driver version 8.1.5.3.0 for this to work ***
void StoredProcRead2();

// stuff for StoredProcRead3()
class EmptyDataObj
{

};

class ProcOutBCA
{
public:
   void operator()(BoundIOs &boundIOs, EmptyDataObj &rowbuf)
   {

   }
};

class ProcOutParams {
public:
	long long_criteria;
	int numRecords;
	
	friend ostream &operator<<(ostream &o, const ProcOutParams &params)
	{
       cout << "ProcOutParams(" << params.long_criteria << ", " << params.numRecords << ")";
	   return o;
	}
};

class ProcOutBPA {
public:
 void operator()(BoundIOs &cols, ProcOutParams &params)
 {
  cols[0] << params.long_criteria;
  cols[1] >> params.numRecords;
 }
};

// simply does a select count(*) from db_example where example_long = 22
// version that will test output params
// *** you must have Oracle ODBC driver version 8.1.5.3.0 for this to work ***
void StoredProcRead3();

void StoredProcReadTestParm();

#endif
