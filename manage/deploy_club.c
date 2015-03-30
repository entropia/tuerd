#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <gcrypt.h>

#include <pcsclite.h>
#include <winscard.h>

#include "libmf.h"

mf_interface *pcsc_init();

int get_safe_key(mf_key_t k) {
	gcry_cipher_hd_t des;

	int ret = gcry_cipher_open(&des, GCRY_CIPHER_DES, GCRY_CIPHER_MODE_ECB, 0);
	if(ret)
		return -1;

	do {
		gcry_randomize(k, 8, GCRY_STRONG_RANDOM);
		ret = gcry_cipher_setkey(des, k, 8);
	} while(ret && gcry_err_code(ret) == GPG_ERR_WEAK_KEY);

	if(ret && gcry_err_code(ret) != GPG_ERR_WEAK_KEY)
		return -1;

	do {
		gcry_randomize(k+8, 8, GCRY_STRONG_RANDOM);
		ret = gcry_cipher_setkey(des, k+8, 8);
	} while(ret && gcry_err_code(ret) == GPG_ERR_WEAK_KEY);

	if(ret && gcry_err_code(ret) != GPG_ERR_WEAK_KEY)
		return -1;

	mf_key_set_version(k, 0);

	return 0;
}

int print_buffer(FILE *f, uint8_t *b, size_t n) {
	for(int i=0; i < n; i++) {
		if(fprintf(f, "%02X", b[i]) != 2) {
			perror("Write failed");
			return -1;
		}
	}

	return 0;
}

void json_key(const char *name, mf_key_t k) {
	printf("   \"%s\" : \"", name);
	print_buffer(stdout, k, 16);
	printf("\"");
}

void json_uid(mf_version *v) {
	printf("   \"uid\" : \"");
	print_buffer(stdout, v->uid, 7);
	printf("\"");
}

#define LOG_FILE "./deploy_log"
int log_action(mf_version *v, mf_key_t master, mf_key_t amk, mf_key_t door) {
	FILE *f;

	f = fopen(LOG_FILE, "a+");
	if(!f) {
		perror("Opening logfile failed");
		return -1;
	}

	setbuf(f, NULL);

	if(print_buffer(f, v->uid, 7)) {
		fprintf(stderr, "Writing log failed");
		return -1;
	}

	fprintf(f, " ");
	if(print_buffer(f, master, 16)) {
		fprintf(stderr, "Writing log failed");
		return -1;
	}

	fprintf(f, " ");
	if(print_buffer(f, amk, 16)) {
		fprintf(stderr, "Writing log failed");
		return -1;
	}

	fprintf(f, " ");
	if(print_buffer(f, door, 16)) {
		fprintf(stderr, "Writing log failed");
		return -1;
	}
	fprintf(f, "\n");

	if(fflush(f)) {
		perror("Flushing logfile failed");
		return -1;
	}

	int fd = fileno(f);
	if(fd == -1) {
		perror("fileno() failed");
		return -1;
	}

	if(fsync(fd)) {
		perror("fsync() failed");
		return -1;
	}

	if(fclose(f)) {
		perror("fclose() failed");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "usage: %s name\n", argv[0]);
		return EXIT_FAILURE;
	}

	mf_interface *intf;
	intf = pcsc_init();

	mf_key_t mf_default_key;
	memset(mf_default_key, 0, 16);

	mf_err_t ret;

	mf_version v;
	ret = mf_get_version(intf, &v);
	if(ret != MF_OK) {
		fprintf(stderr, "Getting version failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	mf_session s;
	ret = mf_authenticate(intf, 0, mf_default_key, &s);
	if(ret != MF_OK) {
		fprintf(stderr, "Authentication failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

/*	ret = mf_format_picc(intf);
	if(ret != MF_OK) {
		fprintf(stderr, "Formatting PICC failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}
*/
	mf_key_t master, amk, door;
	get_safe_key(master);
	get_safe_key(amk);
	get_safe_key(door);

	if(log_action(&v, master, amk, door)) {
		fprintf(stderr, "WAL failed, aborting.\n");
		return EXIT_FAILURE;
	}

	printf("{\n");
	printf("   \"active\" : true,\n");

	json_key("ca0523_door_key", door); printf(",\n");
	json_key("ca0523_master_key", amk); printf(",\n");

	printf("   \"personal\" : {\n");
	printf("      \"name\" : \"%s\",\n", argv[1]);
	printf("      \"sponsors\" : []\n");
	printf("   },\n");

	json_key("picc_key", master); printf(",\n");
	json_uid(&v); printf("\n");

	printf("}\n");

	ret = mf_change_key_settings(intf, &s, MF_PERM_CHANGE_CFG | MF_PERM_CHANGE_KEY);
	if(ret != MF_OK) {
		fprintf(stderr, "Setting master key settings failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_change_current_key(intf, &s, master);
	if(ret != MF_OK) {
		fprintf(stderr, "Setting master key failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_authenticate(intf, 0, master, &s);
	if(ret != MF_OK) {
		fprintf(stderr, "Second authentication failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_create_application(intf, 0xCA0523,
			mf_build_key_settings(0x0, MF_PERM_CHANGE_CFG | MF_PERM_LIST | MF_PERM_CHANGE_KEY), 14);
	if(ret != MF_OK) {
		fprintf(stderr, "Creating application failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_select_application(intf, 0xCA0523);
	if(ret != MF_OK) {
		fprintf(stderr, "Selecting application failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_authenticate(intf, 0, mf_default_key, &s);
	if(ret != MF_OK) {
		fprintf(stderr, "Authentication with AMK failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_change_current_key(intf, &s, amk);
	if(ret != MF_OK) {
		fprintf(stderr, "Setting AMK failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_authenticate(intf, 0, amk, &s);
	if(ret != MF_OK) {
		fprintf(stderr, "Authentication with AMK failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	ret = mf_change_other_key(intf, &s, 0xD, mf_default_key, door);
	if(ret != MF_OK) {
		fprintf(stderr, "Setting door key failed: %s\n", mf_error_str(ret));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
