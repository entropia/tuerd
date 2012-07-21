#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "pcsc.h"
#include "desfire.h"
#include "upgrade.h"
#include "curl.h"
#include "keyset.h"
#include "util.h"

int debug;

static volatile sig_atomic_t reader_crashed = 0, open_requested = 0;
static int failcnt = 0;

void check_config() {
	const char *params[] = {
		"TUERD_READER_BRICKED_URL",
		"TUERD_READER_UNBRICKED_URL",
		"TUERD_GETKEY_URL",
		"TUERD_UNLOCK_URL",
		"TUERD_POLICY_AUTH",
		"TUERD_UPGRADE_DIR",
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

	struct pcsc_context *pcsc_ctx = pcsc_init();
	if(!pcsc_ctx)
		die("pcsc_init() failed");

	debug("Loading upgrades");
	if(load_upgrades())
		die("Loading upgrades failed");

	// Initially, the reader is fine
	push_reader_state_curl(0);

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
				push_reader_state_curl(1);
			}

			continue;
		}

		debug("Successfully got card");

		// Getting card succeeded, reset crash state
		if(reader_crashed) {
			log("Reader was broken recently. Disabling manual open now");
			push_reader_state_curl(0);
		}

		failcnt = 0;
		reader_crashed = 0;

		// Authenticate card
		uint8_t uid[7];
		if(desfire_get_uid(intf, uid) < 0) {
			debug("Retrieving UID failed");
			continue;
		}

		struct keyset keyset;
		if(!get_key_curl(uid, &keyset)) {
			log("Policy did not permit UID %s\n", format_uid(uid));
			continue;
		}

		mf_session sess;
		int auth_success = desfire_authenticate(intf, &keyset, uid, &sess);

		if(auth_success) {
			debug("Auth succeeded");

			debug("Checking for upgrades");
			uint32_t level = do_upgrades(intf, &keyset, &sess, uid);
			if(level < 0)
				log("Upgrading card failed");
			if(level)
				debug("Successfully upgraded card to level %u", level);

			debug("Opening door");
			open_door_curl(uid);

			pcsc_close(pcsc_ctx, intf);
			sleep(10);
		} else {
			debug("Auth failed");

			pcsc_close(pcsc_ctx, intf);
			sleep(1);
		}
	}

	return EXIT_SUCCESS;
}
