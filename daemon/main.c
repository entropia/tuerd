#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pcsc.h"
#include "desfire.h"
#include "curl.h"
#include "util.h"

int debug;

int main(int argc, char **argv) {
	log("tuerd starting up");

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

		sleep(1);

		debug("Waiting for card");

		intf = pcsc_wait(pcsc_ctx);
		if(!intf) {
			log("pcsc_wait() failed");
			continue;
		}

		debug("Successfully got card");

		int auth_success;
		uint8_t uid[7];

		auth_success = desfire_authenticate(intf, get_key_curl, uid);
		if(auth_success) {
			debug("Auth succeeded, opening door");
			open_door_curl(uid);
		} else {
			debug("Auth failed");
		}

		pcsc_close(pcsc_ctx, intf);
	}

	return EXIT_SUCCESS;
}
