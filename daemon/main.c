#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "pcsc.h"
#include "desfire.h"
#include "curl.h"
#include "util.h"

int debug;
extern volatile sig_atomic_t open_requested;

void check_config() {
	if(!getenv("TUERD_GETKEY_URL"))
		die("Need TUERD_GETKEY_URL");

	if(!getenv("TUERD_UNLOCK_URL"))
		die("Need TUERD_UNLOCK_URL");

	if(!getenv("TUERD_POLICY_AUTH"))
		die("Need TUERD_POLICY_AUTH");
}

int main(int argc, char **argv) {
	check_config();

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

		intf = pcsc_wait(pcsc_ctx);
		if(!intf) {
			log("pcsc_wait() failed");
			continue;
		}

		if(open_requested) {
			uint8_t uid[7] = {0x23, 0x42, 0xD0, 0x05, 0xFA, 0x17, 0x00};
			open_requested = 0;

			log("Manual open requested, opening door now");
			open_door_curl(uid);
		}

		debug("Successfully got card");

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
