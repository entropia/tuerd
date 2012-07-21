#include <libmf.h>
#include "desfire.h"
#include "keyset.h"

int load_upgrades(void);
int64_t do_upgrades(mf_interface *intf, struct keyset *keyset, mf_session *sess, uint8_t uid[static 7]);
