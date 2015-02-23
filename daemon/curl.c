#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <libmf.h>
#include <curl/curl.h>

#include "util.h"

static size_t discard_write(const char *ptr, size_t size, size_t nmemb, void *userdata) {
	return size * nmemb;
}

/*
 * Request to open the door.
 *
 * The UID is sent again for accounting purposes.
 */
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
