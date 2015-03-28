#pragma once

#include <stdint.h>

#include "rfid.h"

void git_init(void);
enum rfid_key_cb_result get_key_git(const char *uid, struct rfid_key *key_out);
