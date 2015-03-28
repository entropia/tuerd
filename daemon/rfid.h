#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <nfc/nfc.h>

struct rfid_key {
	uint8_t key[16];
};

enum rfid_key_cb_result {
	KEY_CB_ERROR, TAG_UNKNOWN, TAG_ALLOWED, TAG_FORBIDDEN
};

typedef enum rfid_key_cb_result (*key_callback_t)(const char *uid, struct rfid_key *key);

nfc_device *rfid_init(void);
nfc_target *rfid_poll(nfc_device *dev);
bool rfid_authenticate_any(nfc_device *dev, key_callback_t cb);
