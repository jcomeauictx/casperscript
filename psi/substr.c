/* substring, for debugging with syslog */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

char *substr(char *dest, const char *src, int start);

char *substr(char *dest, const char *src, int start) {
	return strncpy(dest, src + start, strlen(dest));
}

int main(int argc, char **argv) {
	if (argc == 4) {
		char *buffer = argv[1];
		int start = atoi(argv[3]);
		printf("\"%s\"\n", substr(buffer, argv[2], start));
	}
}
