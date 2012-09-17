#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "desfire.h"
#include "util.h"

int desfire_authenticate(mf_interface *intf, key_callback_t cb, uint8_t uid[static 7]) {
	mf_version v;
	mf_err_t ret;

	/*
	 * Retrieve the card's UID
	 */
	ret = mf_get_version(intf, &v);
	if(ret != MF_OK) {
		debug("mf_get_version: %s", mf_error_str(ret));
		return 0;
	}

	memcpy(uid, v.uid, 7);

	/*
	 * Check whether the UID is permitted and retrieve the keys.
	 */
	mf_key_t k;
	if(!cb(v.uid, k)) {
		fprintf(stderr, "Policy did not permit UID ");

		for(int i=0; i<7; i++) {
			fprintf(stderr, "%02X", v.uid[i]);
		}
		
		fprintf(stderr, "\n");

		return 0;
	}

	/*
	 * Do the authentication handshake
	 */
	ret = mf_select_application(intf, 0xCA0523);
	if(ret != MF_OK) {
		debug("mf_select_application: %s", mf_error_str(ret));
		return 0;
	}

	ret = mf_authenticate(intf, 0xD, k, NULL);
	if(ret == MF_OK) // Handshake completed successfully
		return 1;

	if(ret == MF_ERR_AUTHENTICATION_ERROR ||
	   ret == MF_ERR_CARD_AUTH_FAIL) {
		fprintf(stderr, "Authentication failed for UID ");

		for(int i=0; i<7; i++) {
			fprintf(stderr, "%02X", v.uid[i]);
		}

		fprintf(stderr, "\n");
	}
	else
		debug("mf_authenticate: %s", mf_error_str(ret));

	return 0;
}
