#ifndef _KEYSET_H_
#define _KEYSET_H_

#include <libmf.h>

struct keyset {
	mf_key_t picc_key;
	mf_key_t amk;
	mf_key_t door_key;
};

#endif /* _KEYSET_H_ */
