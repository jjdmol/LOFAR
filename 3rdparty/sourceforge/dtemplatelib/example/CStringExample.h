// example for Binkley's cstring class

#ifndef _CSTRINGEXAMPLE_H
#define _CSTRINGEXAMPLE_H

#include "example_core.h"

// Define an object to hold our row data -- used by all single table examples
class ExampleCStr
{
private:                       // tablename.columnname:
 int exampleInt;               // DB_EXAMPLE.INT_VALUE
 tcstring50_t exampleStr;       // DB_EXAMPLE.STRING_VALUE
 double exampleDouble;         // DB_EXAMPLE.DOUBLE_VALUE
 long exampleLong;             // DB_EXAMPLE.EXAMPLE_LONG
 jtime_c exampleDate;          // DB_EXAMPLE.EXAMPLE_DATE

public: 

 ExampleCStr() : exampleInt(0), exampleStr(), exampleDouble(0.0),
	 exampleLong(0), exampleDate() { }

 ExampleCStr(int exInt, const tcstring50_t &exStr, double exDouble, 
  long exLong, const TIMESTAMP_STRUCT &exDate) :
    exampleInt(exInt), exampleStr(exStr), exampleDouble(exDouble), 
	exampleLong(exLong), exampleDate(exDate) { }

 // only accessors needed by our examples
 void SetExampleStr(const tcstring50_t &str)
 {
   exampleStr = str;
 }

 tcstring50_t GetExampleStr() const
 {
    return exampleStr;
 }

 jtime_c GetExampleDate() const
 {
	return exampleDate;
 }

 void SetExampleDate(const jtime_c &jtime)
 {
	exampleDate = jtime;
 }

 int GetExampleInt() const {
	 return exampleInt;
 }

 long GetExampleLong() const {
	 return exampleLong;
 }

 double GetExampleDouble() const {
	 return exampleDouble;
 }

 friend class dtl::DefaultBCA<ExampleCStr>;

 friend class dtl::DefaultInsValidate<ExampleCStr>;

 friend ostream& operator<<(ostream &o, const ExampleCStr &ex)
 {
    o << "ExampleCStr(" << ex.exampleInt << ", \"" << ex.exampleStr << "\", ";
	o << ex.exampleDouble << ", " << ex.exampleLong << ", ";
	o << ex.exampleDate << ")" ;

	return o;
 }

 friend bool operator<(const ExampleCStr &ex1, const ExampleCStr &ex2)
 {
	if (ex1.exampleInt < ex2.exampleInt)
		return true;
	if (ex1.exampleInt > ex2.exampleInt)
		return false;

	if (ex1.exampleStr < ex2.exampleStr)
		return true;
	if (ex1.exampleStr > ex2.exampleStr)
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


};

BEGIN_DTL_NAMESPACE

template<> class DefaultBCA<ExampleCStr>
{
public:
 void operator()(BoundIOs &cols, ExampleCStr &row)
 {
  int & ref = row.exampleInt;
  cols["INT_VALUE"]    == (int &)ref;
  cols["STRING_VALUE"] == row.exampleStr;
  cols["DOUBLE_VALUE"] == row.exampleDouble;
  cols["EXAMPLE_LONG"] == row.exampleLong;
  cols["EXAMPLE_DATE"] == row.exampleDate;
 }
};

END_DTL_NAMESPACE

class SelValNullData : public binary_function<BoundIOs &, ExampleCStr &, bool>
{
public:
	bool operator()(BoundIOs &boundIOs, ExampleCStr &rowbuf)
	{
		boundIOs.InitNullFields();
		return true;
	}
};

void CStringExample();

#endif
