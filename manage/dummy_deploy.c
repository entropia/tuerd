#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <gcrypt.h>

typedef uint8_t mf_key_t[16];

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

	return 0;
}

void json_key(const char *name, mf_key_t k) {
	printf("\"%s\": \"", name);
	for(int i = 0; i < 16; i++)
		printf("%02X", k[i]);
	printf("\"");
}

void json_uid() {
	printf("\"uid\": \"");
	for(int i = 0; i < 7; i++)
		printf("%02X", i);
	printf("\"");
}

int main(int argc, char **argv) {
	mf_key_t master, amk, door;
	get_safe_key(master);
	get_safe_key(amk);
	get_safe_key(door);

	printf("{\n");
	json_uid(); printf(",\n");
	json_key("picc_key", master); printf(",\n");
	json_key("ca0523_master_key", amk); printf(",\n");
	json_key("ca0523_door_key", door); printf("\n");
	printf("}\n");
	
	return EXIT_SUCCESS;
}
