#include "example.h"


int main(int argc, char **argv)
{
	if (argc != 2)
	{
		cout << "Usage: example.exe <DSN connect string>" << "\n";
		cout << "As an example, to use the ''example'' data source name with a username and password of ''example''" << "\n";
		cout << "One would write: example.exe UID=example;PWD=example;DSN=example;" << endl;
		return -1;
	}

	const string DSN_argv(argv[1]);
    
	CallAllExamples(DSN_argv);

	return 0;
}
