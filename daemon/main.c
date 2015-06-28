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

	git_init();

	nfc_device *nfc_dev = rfid_init();
	if(!nfc_dev)
		die("rfid_init failed");

	while(1) {
		debug("-----------------");
		debug("waiting for event");

		rfid_poll(nfc_dev);

		debug("successfully got target");

		if(rfid_authenticate_any(nfc_dev, get_key_git)) {
			debug("auth succeeded, opening door");
			open_door();

			sleep(10);
		} else {
			debug("auth failed");

			sleep(1);
		}
	}

	return EXIT_SUCCESS;
}
