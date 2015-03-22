#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

#include "door.h"
#include "util.h"

static pid_t door_pid(void) {
	FILE *f;

	f = fopen(getenv("USBLOCKD_PIDFILE"), "r");
	if(!f) {
		perror("opening usblockd pidfile failed");
		return -1;
	}

	char buf[16];
	if(fgets(buf, 16, f) == NULL) {
		perror("reading usblockd pidfile failed");
		return -1;
	}

	long long int pid = strtoll(buf, NULL, 10);
	return (pid_t)pid;
}

void open_door(void) {
	pid_t pid = door_pid();

	if(kill(pid, SIGUSR2) < 0)
		perror("sending unlock signal failed");
}

void powercycle_reader(void) {
	pid_t pid = door_pid();

	if(kill(pid, SIGHUP) < 0)
		perror("sending powercycle signal failed");
}
