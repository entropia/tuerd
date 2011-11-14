#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pcsc.h"
#include "desfire.h"
#include "curl.h"
#include "util.h"

int main(int argc, char **argv) {
	log("tuerd starting up");

	struct pcsc_context *pcsc_ctx = pcsc_init();
	if(!pcsc_ctx) {
		log("pcsc_init() failed");
		return EXIT_FAILURE;
	}

	while(1) {
		mf_interface *intf;

		sleep(1);

		intf = pcsc_wait(pcsc_ctx);
		if(!intf) {
			log("pcsc_wait() failed");
			continue;
		}

		int auth_success;
		auth_success = desfire_authenticate(intf, get_key_curl);
		if(auth_success)
			open_door_curl();
	}

	return EXIT_SUCCESS;
}
