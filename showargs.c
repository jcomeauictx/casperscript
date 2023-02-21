/* https://www.in-ulm.de/~mascheck/various/shebang/ */
#include <stdio.h>
int main(argc, argv)
	int argc; char** argv;
{
	int i;
	for (i=0; i<argc; i++)
    		fprintf(stdout, "argv[%d]: \"%s\"\n", i, argv[i]);
	return(0);
}
