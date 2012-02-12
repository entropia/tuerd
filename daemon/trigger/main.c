#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

#define PIDFILE "/var/run/tuerd.pid"

int main(int argc, char **argv) {
	FILE *f;

	f = fopen(PIDFILE, "r");
	if(!f) {
		perror("Could not open pidfile");
		return EXIT_FAILURE;
	}

	char buf[64];
	fgets(buf, 64, f);

	pid_t pid = atoi(buf);

	printf("\n\n\n\tBitte den Reader (USB) an fegefeuer (am Balken ueber der Tuer) powercyclen!\n\n\n");

	if(kill(pid, SIGUSR1) < 0) {
		perror("Signalling daemon failed");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
