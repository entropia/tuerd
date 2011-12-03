#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libmf.h>
#include <curl/curl.h>

#include "util.h"

static size_t append_string(const char *ptr, size_t size, size_t nmemb, void *userdata) {
	char **str = (char**)userdata;
	size_t in_len = size*nmemb;
	
	if(!in_len)
		return 0;

	//check for overflow
	if(in_len / size != nmemb)
		return 0;

	size_t old_len;
	if(!*str) {
		*str = strndup(ptr, in_len);
		return in_len;
	}

	old_len = strlen(*str);

	*str = realloc(*str, old_len + in_len + 1);
	if(!*str)
		return 0;

	strncat(*str, ptr, in_len);
	return in_len;
}

int get_key_curl(uint8_t uid[static 7], mf_key_t key_out) {
	int ret = 0;
	CURL *curl;

	curl = curl_easy_init();
	if(!curl) {
		debug("curl_easy_init() failed");
		return 0;
	}

	char *result = NULL;
	char errbuf[CURL_ERROR_SIZE];
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, append_string);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	curl_easy_setopt(curl, CURLOPT_URL, getenv("TUERD_GETKEY_URL"));
	curl_easy_setopt(curl, CURLOPT_USERPWD, getenv("TUERD_POLICY_AUTH"));
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

	char argbuf[19];
	strcpy(argbuf, "UID=");
	for(int i=0; i < 7; i++) {
		snprintf(argbuf + 4 + 2*i, 3, "%02X", uid[i]);
	}
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, argbuf);

	if(curl_easy_perform(curl)) {
		log("Request to policy server failed: %s", errbuf);

		ret = 0;
		goto out;
	}

	long http_response = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_response);
	if(http_response != 200) {
		debug("Got code %ld in response to get_key.\n", http_response);

		ret = 0;
		goto out;
	}

	if(!result || *result != 't') {
		ret = 0;
		goto out;
	}

	char *keystr = strchr(result, ' ');
	if(!keystr || strlen(keystr+1) != 32) {
		ret = 0;
		goto out;
	}

	mf_key_parse(key_out, keystr+1);

	ret = 1;
out:
	free(result);
	curl_easy_cleanup(curl);

	return ret;
}

static size_t discard_write(const char *ptr, size_t size, size_t nmemb, void *userdata) {
	return size * nmemb;
}

void open_door_curl(uint8_t uid[static 7]) {
	CURL *curl;

	curl = curl_easy_init();
	if(!curl) {
		debug("curl_easy_init() failed");
		return;
	}

	char errbuf[CURL_ERROR_SIZE];
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discard_write);
	curl_easy_setopt(curl, CURLOPT_URL, getenv("TUERD_UNLOCK_URL"));
	curl_easy_setopt(curl, CURLOPT_USERPWD, getenv("TUERD_POLICY_AUTH"));
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

	char argbuf[19];
	strcpy(argbuf, "UID=");
	for(int i=0; i < 7; i++) {
		snprintf(argbuf + 4 + 2*i, 3, "%02X", uid[i]);
	}
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, argbuf);

	if(curl_easy_perform(curl))
		debug("Request to open door failed: %s", errbuf);

	curl_easy_cleanup(curl);
}
