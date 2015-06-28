#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "rfid.h"
#include "door.h"
#include "git.h"
#include "util.h"

#define LOG_SECTION "main"

int debug;

void check_config() {
	const char *params[] = {
		"TUERD_REPO_PATH",
		"USBLOCKD_PIDFILE",
		NULL
	};

	for(int i = 0; params[i]; i++)
		if(!getenv(params[i]))
			die("Need %s", params[i]);
}

void sigalarm(int sig) {
	(void)sig;

	const char *msg = "watchdog signal caught, exiting\n";
	ssize_t remaining = strlen(msg);

	while(remaining) {
		ssize_t ret = write(STDERR_FILENO, msg, remaining);

		if(ret < 0)
			break;

		remaining -= ret;
		msg += ret;
	}

	_exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	check_config();

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	log("tuerd (rev " GIT_REV  ") starting up");

	if(getenv("TUERD_DEBUG"))
		debug = 1;

	debug("Debugging enabled. This allows people to be tracked!");

	// initialize git
	git_init();

	// initialize nfc
	nfc_device *nfc_dev = rfid_init();
	if(!nfc_dev)
		die("rfid_init failed");

	// initialize watchdog handler
	struct sigaction sa;
	sa.sa_handler = sigalarm;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGALRM, &sa, NULL);

	while(1) {
		debug("-----------------");
		debug("waiting for event");

		if(!rfid_poll(nfc_dev))
			die("rfid_poll failed");

		debug("successfully got target");

		alarm(10);

		if(rfid_authenticate_any(nfc_dev, get_key_git)) {
			debug("auth succeeded, opening door");
			open_door();
			alarm(0);

			sleep(10);
		} else {
			debug("auth failed");
			alarm(0);

			sleep(1);
		}
	}

	return EXIT_SUCCESS;
}
