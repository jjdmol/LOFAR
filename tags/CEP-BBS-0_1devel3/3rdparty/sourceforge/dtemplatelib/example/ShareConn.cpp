#include "ShareConn.h"

// this function tests reading from a DBConnection that's created and shared
void SharedConnectionRead(const string &DSN)
{
	// create new connection
	cout << "Creating new connection and setting it to be the default " 
	     << " to reference it!" << endl;
	DBConnection conn(DSN);
	conn.Connect();

	// make the default connection reference this new connection
	DBConnection::GetDefaultConnection().Share(conn.GetHENV(), conn.GetHDBC());

	// read and print out examples as output
	DBView<Example> view(DBView<Example>::Args().tables("DB_EXAMPLE").postfix(exampleOrderBy));

	cout << "Printing out examples from this new default connection!" << endl;

	copy(view.begin(), view.end(), ostream_iterator<Example>(cout, "\n"));

	// as conn destroyed in this example at closing brace,
	// default connection must be released
	// else we'll have a dangling DBConnection object for the default
	DBConnection::GetDefaultConnection().Release();

	// reconnect the default connection under its own power
	// so we won't get surprises in later examples
	DBConnection::GetDefaultConnection().Connect(DSN);
}
