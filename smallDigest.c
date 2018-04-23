#include "siserver.h"

int main(int argc, char **argv) {
	if (argc != 5) {
		fprintf(stderr,
				"usage: %s <MachineName> <TCPport> <SecretKey> <Data>\n",
				argv[0]);
		exit(1);
	}
	setvbuf(stdout, NULL, _IONBF, 0);
	smallDigest(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4]);
	return 0;
}
