#include <libmf.h>

typedef int (*key_callback_t)(uint8_t uid[7], mf_key_t key);
int desfire_authenticate(mf_interface *intf, key_callback_t cb, mf_session *sess, uint8_t uid[static 7]);
