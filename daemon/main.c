#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "pcsc.h"
#include "desfire.h"
#include "curl.h"
#include "util.h"

int debug;

static volatile sig_atomic_t reader_crashed = 0, open_requested = 0;
static int failcnt = 0;

void check_config() {
	if(!getenv("TUERD_GETKEY_URL"))
		die("Need TUERD_GETKEY_URL");

	if(!getenv("TUERD_UNLOCK_URL"))
		die("Need TUERD_UNLOCK_URL");

	if(!getenv("TUERD_POLICY_AUTH"))
		die("Need TUERD_POLICY_AUTH");
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

	struct pcsc_context *pcsc_ctx = pcsc_init();
	if(!pcsc_ctx) {
		log("pcsc_init() failed");
		return EXIT_FAILURE;
	}

	while(1) {
		mf_interface *intf;

		debug("Waiting for card");

		if(open_requested) {
			uint8_t uid[7] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
			open_requested = 0;

			log("Manual open requested, opening door now");
			open_door_curl(uid);
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
		if(reader_crashed)
			log("Reader was broken recently. Disabling manual open now");

		failcnt = 0;
		reader_crashed = 0;

		// Authenticate card
		int auth_success;
		uint8_t uid[7];

		auth_success = desfire_authenticate(intf, get_key_curl, uid);
		pcsc_close(pcsc_ctx, intf);

		if(auth_success) {
			debug("Auth succeeded, opening door");
			open_door_curl(uid);

			sleep(10);
		} else {
			debug("Auth failed");

			sleep(1);
		}
	}

	return EXIT_SUCCESS;
}
