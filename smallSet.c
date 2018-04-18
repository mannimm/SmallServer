#include "siserver.h"

int main(int argc, char **argv) {
	
	if (argc != 6) {
		fprintf(stderr, "usage: %s <MachineName> <TCPport> <SecretKey> <VariableName> <Value>\n", argv[0]);
		exit(1);
    	}

	setvbuf(stdout, NULL, _IONBF, 0);
	smallSet (argv[1], atoi(argv[2]), atoi(argv[3]), argv[4], argv[5], strlen(argv[5]));

	return 0;
}

