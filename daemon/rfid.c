#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <nfc/nfc.h>
#include <freefare.h>

#include "util.h"

#include "rfid.h"

#define LOG_SECTION "rfid"
#define NFC_MAX_DEVICES 8

nfc_context *nfc_ctx;
MifareDESFireAID door_aid;

nfc_device *rfid_init(void) {
	nfc_init(&nfc_ctx);
	if(!nfc_ctx) {
		log("initializing libnfc failed");
		return NULL;
	}

	door_aid = mifare_desfire_aid_new(0x2305CA);

	nfc_connstring devices[NFC_MAX_DEVICES];
	int device_count = nfc_list_devices(nfc_ctx, devices, NFC_MAX_DEVICES);
	if(device_count <= 0) {
		log("no NFC devices found");
		return NULL;
	}

	log("found %u NFC devices:", device_count);
	for(int i = 0; i < device_count; i++)
		log(" - %s", devices[i]);

	int selected_device = -1;
	const char *conf_connstring = getenv("RFID_CONNSTRING");
	if(conf_connstring) {
		for(int i = 0; i < device_count; i++)
			if(strstr(devices[i], conf_connstring)) {
				selected_device = i;
				break;
			}
	} else {
		log("no connection string supplied, using first device");
		selected_device = 0;
	}

	if(selected_device == -1) {
		log("could not find requested device");
		return NULL;
	}

	log("using device %s", devices[selected_device]);

	nfc_device *dev = nfc_open(nfc_ctx, devices[selected_device]);
	if(!dev) {
		log("opening NFC device failed");
		return NULL;
	}

	if(nfc_initiator_init(dev) < 0) {
		nfc_perror(dev, "configuring NFC device as initiator failed");
		return NULL;
	}

	return dev;
}

nfc_target *rfid_poll(nfc_device *dev) {
	static nfc_target target;
	int failcnt = 0;

	while(1) {
		nfc_modulation modulation = {
			.nmt = NMT_ISO14443A,
			.nbr = NBR_106
		};

		int ret = nfc_initiator_poll_target(dev, &modulation, 1, 2, 2, &target);

		// NFC_ECHIP means timeout
		if(ret > 0)
			return &target;

		if(ret != NFC_ECHIP) {
			log("nfc_initiator_poll_target() failed");

			failcnt++;
			if(failcnt >= 6) {
				log("nfc_initiator_poll_target() failed too often, aborting");
				return NULL;
			}

			sleep(1 << (((failcnt > 4) ? 4 : failcnt) - 1));
		}
	}
}

static bool rfid_authenticate(MifareTag tag, struct rfid_key *key) {
	int ret;
	bool result = false;

	ret = mifare_desfire_connect(tag);
	if(ret < 0) {
		log("failed to connect to tag");
		return false;
	}

	ret = mifare_desfire_select_application(tag, door_aid);
	if(ret < 0) {
		log("failed to select application");
		goto out_tag;
	}

	MifareDESFireKey dfkey = mifare_desfire_3des_key_new(key->key);

	ret = mifare_desfire_authenticate(tag, 0xD, dfkey);
	if(ret < 0) {
		log("authentication failed");
		goto out_key;
	}

	if(ret == 0)
		result = true;

out_key:
	mifare_desfire_key_free(dfkey);
out_tag:
	mifare_desfire_disconnect(tag);

	return result;
}

bool rfid_authenticate_any(nfc_device *dev, key_callback_t cb) {
	MifareTag *tags = freefare_get_tags(dev);
	if(!tags) {
		// FIXME: reset reader?
		log("error listing tags");
		return false;
	}

	for(int i = 0; tags[i]; i++) {
		MifareTag tag = tags[i];

		if(freefare_get_tag_type(tag) != DESFIRE) {
			log("tag is not a desfire tag");
			continue;
		}

		char *uid = freefare_get_tag_uid(tag);
		debug("got uid %s", uid);

		struct rfid_key key;
		enum rfid_key_cb_result result = cb(uid, &key);
		switch(result) {
			case TAG_UNKNOWN:
				log("no key found in git");
				break;
			case TAG_FORBIDDEN:
				log("tag forbidden by policy");
				break;
			case TAG_ALLOWED:
				debug("tag allowed by policy");
				return rfid_authenticate(tag, &key);
			case KEY_CB_ERROR:
				log("key callback error");
				break;
			default:
				die("unknown key callback result received: %d", result);
		}

		free(uid);
	}

	debug("exhausted all available tags");

	freefare_free_tags(tags);
	return false;
}
