#include "GlobalHandler.h"

dtl_ios_base::MeansOfRecovery 
		GlobalHandlerExample::operator()(const RootException *pEx,
										 ErrorSeverity::SeverityCode severity)
{
	switch (severity)
	{
	    case ErrorSeverity::ERR_WARNING : 
			cout << "DTL_WARN encountered!  Handle your warnings here!" << endl;
			cout << "Suppressing error!" << endl;
			return dtl_ios_base::SUPPRESS_ERROR;
		
		case ErrorSeverity::ERR_FAILURE :
			cout << "DTL_THROW encountered!  Handle your exceptions here!" << endl;
			cout << "Rethrowing exception!" << endl;
			return dtl_ios_base::THROW_EXCEPTION;
	}

	cout << "Shouldn't get here!" << endl;
	return dtl_ios_base::THROW_EXCEPTION; // suppress return value warning for MSVC
}

void UseGlobalHandlerExample()
{
    PrintHeader(cout, "UseGlobalHandlerExample()");

	// install our example handler
	GetErrorHandler().SetHandler(GlobalHandlerExample());

	try
	{
		RootException ex("UseGlobalHandlerExample()", "test exception!");

		// this should warn
		DTL_WARN RootException("UseGlobalHandlerExample()", 
			"You should see a statement from handler about DTL_WARN.");
		
		// this will throw ... should trigger our handler
		cout << "Attempting bogus select to trigger DTL_THROW ..." << endl;
		cout << "Next statement you should see should see from handler should be "
			 << "about DTL_THROW." << endl;

		DBStmt("select garbage from trash").Execute();
	}
	catch (RootException &ex)
	{
		cout << "Exception caught as expected!  Caught exception follows ..." << endl;
		cout << ex.what() << endl;
	}

	// clear out the error handler to restore default behavior
	GetErrorHandler().ClearHandler();

	PrintSeparator(cout);
}