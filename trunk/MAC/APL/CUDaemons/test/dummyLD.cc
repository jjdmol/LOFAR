#include <lofar_config.h>
#include <stdio.h>
#include <unistd.h>

int main (int argc,	char* argv[]) 
{
	printf ("Program invocation: ");

	for (int i = 0; i < argc; i++) {
		printf ("%s ", argv[i]);
	}

	printf ("\n");

	sleep (20);

	return (0);
}
