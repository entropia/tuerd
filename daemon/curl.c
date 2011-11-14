#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libmf.h>
#include <curl/curl.h>

#include "util.h"

#include "config.h"

static size_t append_string(const char *ptr, size_t size, size_t nmemb, void *userdata) {
	unsigned char **str = (unsigned char**)userdata;
	size_t in_len = size*nmemb;
	
	if(!in_len)
		return 0;

	//check for overflow
	if(in_len / size != nmemb)
		return 0;

	size_t old_len = strlen(*str);
	*str = realloc(*str, old_len + in_len + 1);
	if(!*str)
		return 0;

	strncat(*str, ptr, in_len);
}

int get_key_curl(uint8_t uid[7], mf_key_t key_out) {
	CURL *curl;

	curl = curl_easy_init();
	if(!curl) {
		log("curl_easy_init() failed");
		return -1;
	}

	unsigned char *result;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_string);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	curl_easy_setopt(curl, CURLOPT_URL, CURL_GETKEY_URL);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "foobar");

	curl_easy_perform(curl);
}

void open_door_curl() {

}
