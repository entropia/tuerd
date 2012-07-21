#include <libmf.h>

int load_upgrades(void);
uint32_t do_upgrades(mf_interface *intf, mf_session *sess, uint8_t uid[static 7]);
