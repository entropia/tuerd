#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "desfire.h"
#include "keyset.h"
#include "util.h"

int desfire_get_uid(mf_interface *intf, uint8_t uid[static 7]) {
	mf_version v;
	mf_err_t ret;

	ret = mf_get_version(intf, &v);
	if(ret != MF_OK) {
		debug("mf_get_version: %s", mf_error_str(ret));
		return -1;
	}

	memcpy(uid, v.uid, 7);

	return 0;
}

int desfire_authenticate(mf_interface *intf, struct keyset *keyset, uint8_t uid[static 7], mf_session *sess) {
	mf_err_t ret = mf_select_application(intf, 0xCA0523);
	if(ret != MF_OK) {
		debug("mf_select_application: %s", mf_error_str(ret));
		return 0;
	}

	ret = mf_authenticate(intf, 0xD, keyset->door_key, sess);
	if(ret == MF_OK)
		return 1;

	if(ret == MF_ERR_AUTHENTICATION_ERROR ||
	   ret == MF_ERR_CARD_AUTH_FAIL)
		fprintf(stderr, "Authentication failed for UID %s\n", format_uid(uid));
	else
		debug("mf_authenticate: %s", mf_error_str(ret));

	return 0;
}
