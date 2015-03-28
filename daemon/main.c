#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <git2.h>

#include "desfire.h"
#include "door.h"
#include "git.h"
#include "util.h"

int debug;

static volatile sig_atomic_t reader_crashed = 0, open_requested = 0;
static int failcnt = 0;

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

void open_handler(int sig) {
	if(reader_crashed)
		open_requested = 1;
}

void register_handler() {
	struct sigaction sa;
	sa.sa_handler = open_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(sigaction(SIGUSR1, &sa, NULL) < 0)
		die("Registering signal handler failed");
}

int main(int argc, char **argv) {
	check_config();
	register_handler();

	log("tuerd (rev " GIT_REV  ") starting up");

	if(getenv("TUERD_DEBUG"))
		debug = 1;

	debug("Debugging enabled. This allows people to be tracked!");

	nfc_context *nfc_ctx;
	nfc_init(&nfc_ctx);
	if(!ctx)
		die("initializing libnfc failed");

	git_threads_init();
	atexit(git_threads_shutdown);

	while(1) {
		mf_interface *intf;

		debug("Waiting for card");

		// If a manual open was requested, do it now.
		if(open_requested) {
			open_requested = 0;

			log("Manual open requested, opening door now");
			open_door();
		}

		intf = pcsc_wait(pcsc_ctx);
		if(!intf) {
			log("pcsc_wait() failed");

			if(++failcnt >= 3 && !reader_crashed) {
				log("Failing too often, allowing for manual open");
				reader_crashed = 1;
			}

			continue;
		}

		debug("Successfully got card");

		// Getting card succeeded, reset crash state
		if(reader_crashed) {
			log("Reader was broken recently. Disabling manual open now");
		}

		failcnt = 0;
		reader_crashed = 0;

		// Authenticate card
		int auth_success;
		uint8_t uid[7];

		auth_success = desfire_authenticate(intf, get_key_git, uid);
		pcsc_close(pcsc_ctx, intf);

		if(auth_success) {
			debug("Auth succeeded, opening door");
			open_door();

			sleep(10);
		} else {
			debug("Auth failed");

			sleep(1);
		}
	}

	return EXIT_SUCCESS;
}
