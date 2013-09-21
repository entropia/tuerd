#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#include "powercycle.h"
#include "util.h"

static pid_t powercycle_pid(void) {
	FILE *f;

	f = fopen(getenv("USBSCHALTER_PIDFILE"), "r");
	if(!f) {
		perror("opening usbschalter pidfile failed");
		return -1;
	}

	char buf[16];
	if(fgets(buf, 16, f) == NULL) {
		perror("reading usbschalter pidfile failed");
		return -1;
	}

	long long int pid = strtoll(buf, NULL, 10);
	return (pid_t)pid;
}

void powercycle_reader(void) {
	pid_t pid = powercycle_pid();

	log("Attempting to powercycle reader");
	if(kill(pid, SIGHUP) < 0)
		perror("Sending powercycle-signal failed");
}
