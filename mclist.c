#include "cloud.h"

int main(int argc, char **argv) {
	if (argc != 4) {
		fprintf(stderr, "usage: %s <MachineName> <TCPport> <SecretKey>\n",
				argv[0]);
		exit(1);
	}
	setvbuf(stdout, NULL, _IONBF, 0);
	mycloud_listfile(argv[1], atoi(argv[2]), atoi(argv[3]));
	return 0;
}

