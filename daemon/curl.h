#include <stdint.h>
#include <libmf.h>

#include "keyset.h"

int get_key_curl(uint8_t uid[7], struct keyset *keyset);
void push_reader_state_curl(uint8_t bricked);
void open_door_curl();
