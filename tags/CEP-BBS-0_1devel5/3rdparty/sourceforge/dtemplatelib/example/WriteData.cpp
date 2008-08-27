// example function for writing data to a view

#include "example_core.h"

// Write the contents of vector<Example> to a table in the database
void WriteData(const vector<Example> &examples)
{

 DBView<Example> view(DBView<Example>::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));


#if 0
 // Alternate insert form using sql_iterator
 //
 // constructor object version with convention of operator()()
 // automatically sets up the callbacks based on the SelVal and InsVal
 // function objects passed in!  Not as yuck as the others!
 DBView<Example> view("INSERT INTO DB_EXAMPLE(INT_VALUE, STRING_VALUE, DOUBLE_VALUE, EXAMPLE_LONG, EXAMPLE_DATE) "
	 "VALUES(?, ?, ?, ?, ?, ?)", ExampleBCA(), exampleOrderBy); 
#endif

 view.set_io_handler(LoggingHandler<Example>());

 // loop through vector and write Example objects to DB
 // write_it.GetCount() records written in loop

 DBView<Example>::insert_iterator write_it = view;

 // implicitly due to defaults, the following is performed:
 // write_it.set_io_handler(LoggingHandler<Example>());
 
 // the LoggingHandler creates an error log, which you can retrieve using
 // LoggingHandler<Example>::GetLog()

 for (vector<Example>::const_iterator ex_it =  examples.begin(); ex_it != examples.end(); ++ex_it, ++write_it)
 {
  *write_it = *ex_it;
  cout << "Writing element #" <<   write_it.GetCount() + 1<< endl;
 }

 // find out what errors occurred during the writing out of the vector to the DB
 
 typedef LoggingHandler<Example>::LoggedTriple LoggedTriple;
 
 // retrieve the LoggingHandler object from the iterator
 LoggingHandler<Example> log_handler = 
	 write_it.get_io_handler((LoggingHandler<Example> *) NULL);

 // the log is a vector of (error message, DataObj, ParamObj) triples,
 // (error message, Example object, DefaultParamObj<Example> object) in this case
 // the error itself along with the relevant DataObj and ParamObj that resulted with
 // the error
 vector<LoggedTriple> error_log = log_handler.GetLog();

 // nothing to do if no errors occurred
 if (error_log.empty())
	 return;

 cout << "*** Error Log in WriteData(): " << error_log.size() << " errors recorded! ***"
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

