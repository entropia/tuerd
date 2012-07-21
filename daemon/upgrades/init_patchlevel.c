#include <stdio.h>
#include <libmf.h>

#include "keyset.h"
//#include "util.h"

int level = 1;

int apply(mf_interface *intf, struct keyset *keyset) {
        mf_err_t ret;

	ret = mf_select_application(intf, 0xCA0523);
	if(ret != MF_OK) {
		printf("mf_select_application: %s", mf_error_str(ret));
		return -1;
	}

	ret = mf_authenticate(intf, 0x0, keyset->amk, NULL);
	if(ret != MF_OK) {
		printf("mf_authenticate: %s", mf_error_str(ret));
		return -1;
	}

	mf_access_t access = 0;

	MF_ACCESS_SET_RW(access, 0xD);
	MF_ACCESS_SET_R(access, 0xF);
	MF_ACCESS_SET_W(access, 0xF);

	printf("%X\n", access);
	ret = mf_create_value_file(intf, 0x0, MF_COMM_PLAIN, access, 0, 0xFFFF, 0, 0);
	if(ret != MF_OK) {
		printf("mf_create_value_file: %s", mf_error_str(ret));
		return -1;
	}
	
	return 0;
}
