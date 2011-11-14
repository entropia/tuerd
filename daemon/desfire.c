#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "desfire.h"
#include "util.h"

int desfire_authenticate(mf_interface *intf, key_callback_t cb) {
	mf_version v;
	mf_err_t ret;

	ret = mf_get_version(intf, &v);
	if(ret != MF_OK) {
		log("mf_get_version: %s", mf_error_str(ret));
		return 0;
	}

	mf_key_t k;
	if(cb(v.uid, k)) {
		fprintf(stderr, "Policy did not permit UID ");

		for(int i=0; i<7; i++) {
			fprintf(stderr, "%02X", v.uid[i]);
		}
		
		fprintf(stderr, "\n");

		return 0;
	}

	ret = mf_select_application(intf, 0xCA0523);
	if(ret != MF_OK) {
		log("mf_select_application: %s", mf_error_str(ret));
		return 0;
	}

	ret = mf_authenticate(intf, 0xD, k, NULL);
	if(ret != MF_OK) {
		if(ret == MF_ERR_CARD_AUTH_FAIL) {
			fprintf(stderr, "Authentication failed for UID ");

			for(int i=0; i<7; i++) {
				log("%02X", v.uid[i]);
			}
			
			fprintf(stderr, "\n");
		}
		else
			fprintf(stderr, "mf_authenticate: %s", mf_error_str(ret));

		return 0;
	}

	return 1;
}
