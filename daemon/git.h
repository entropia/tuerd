#pragma once

#include <stdint.h>
#include <libmf.h>

void git_init(void);
int get_key_git(uint8_t uid[7], mf_key_t key_out);
