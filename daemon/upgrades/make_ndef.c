#include <stdio.h>
#include <libmf.h>

#include "keyset.h"
//#include "util.h"

int level = 2;

int apply(mf_interface *intf, struct keyset *keyset) {
        mf_err_t ret;

	ret = mf_select_application(intf, 0x000000);
	if(ret != MF_OK) {
		printf("mf_select_application: %s", mf_error_str(ret));
		return -1;
	}

	mf_session sess;
	ret = mf_authenticate(intf, 0x0, keyset->picc_key, &sess);
	if(ret != MF_OK) {
		printf("mf_authenticate: %s", mf_error_str(ret));
		return -1;
	}

	ret = mf_change_key_settings(intf, &sess, 0x0F);
	if(ret != MF_OK) {
		fprintf(stderr, "ChangeKeySettings failed: %s\n", mf_error_str(ret));
		return -1;
	}

	/*
	 * custom create application for hidden ISO tag:
	 * ca  01 00 00 0F 21 10 E1 D2 76 00 00 85 01 01
	 */

	uint8_t create_application[] = {
		0x01 , 0x00 , 0x00 , 0x0F , 0x21 , 0x10 , 0xE1 , 0xD2 , 0x76 , 0x00 , 0x00 , 0x85 , 0x01 , 0x01
	};

	ret = mf_call(intf, 0xCA, create_application, sizeof(create_application), NULL, NULL);
	if(ret != MF_OK) {
		printf("mf_create_application: %s", mf_error_str(ret));
		return -1;
	}

	ret = mf_select_application(intf, 0x010000);
	if(ret != MF_OK) {
		printf("mf_select_application: %s", mf_error_str(ret));
		return -1;
	}

	/*
	 * custom create standard data file for hidden ISO tag:
	 * cd  01 03 e1 00 ee ee 0f 00 00
	 */

	uint8_t create_cc[] = {
		0x01, 0x03, 0xe1 , 0x00 , 0xee , 0xee , 0x0f , 0x00 , 0x00
	};

	ret = mf_call(intf, 0xCD, create_cc, sizeof(create_cc), NULL, NULL);
	if(ret != MF_OK) {
		printf("mf_create_std_data_file: %s", mf_error_str(ret));
		return -1;
	}
	
	uint8_t cc_data[] = {
	 0x00, 0x0F, 0x20, 0x00, 0x3B, 0x00, 0x34, 0x04, 0x06, 0xE1, 0x04, 0x10, 0x00, 0x00, 0x00	
	};
	ret = mf_write_file(intf, &sess, 0x01, 0, 15, cc_data);
	if(ret != MF_OK) {
		printf("mf_write_file: %s", mf_error_str(ret));
		return -1;
	}
	
	/*
	 * custom create standard data file for hidden ISO tag:
	 * cd  02 04 e1 00 ee ee 80 00 00
	 */

	uint8_t create_ndef[] = {
		0x02, 0x04, 0xe1, 0x00, 0xee, 0xee, 0x80, 0x00, 0x00
	};

	ret = mf_call(intf, 0xCD, create_ndef, sizeof(create_ndef), NULL, NULL);
	if(ret != MF_OK) {
		printf("mf_create_std_data_file: %s", mf_error_str(ret));
		return -1;
	}
	
	//create data file
	uint8_t ndef_data[] = {
	 0, 19, 0xd1, 0x01, 0x0f, 0x55, 0x01, 0x64, 0x61, 0x66, 0x6b, 0x2e, 0x6e, 0x65, 0x74, 0x2f, 0x77, 0x68, 0x61, 0x74, 0x2f
	};
	ret = mf_write_file(intf, &sess, 0x02, 0, sizeof(ndef_data), ndef_data);
	if(ret != MF_OK) {
		printf("mf_write_file: %s", mf_error_str(ret));
		return -1;
	}

	return 0;
}
