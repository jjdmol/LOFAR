#include <lofar_config.h>
#include <stdio.h>

int main (int argc,	char* argv[]) 
{
	printf ("Program invocation: ");

	for (int i = 0; i < argc; i++) {
		printf ("%s ", argv[i]);
	}

	printf ("\n");

	return (1);
}
