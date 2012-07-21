#ifndef _DESFIRE_H_
#define _DESFIRE_H_

#include <libmf.h>
#include "keyset.h"

int desfire_get_uid(mf_interface *intf, uint8_t uid[static 7]);
int desfire_authenticate(mf_interface *intf, struct keyset *keyset, uint8_t uid[static 7], mf_session *sess);

#endif /* _DESFIRE_H_ */
