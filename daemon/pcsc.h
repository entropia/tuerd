#include <libmf.h>

struct pcsc_context;

struct pcsc_context *pcsc_init();
mf_interface *pcsc_wait();
void pcsc_close(struct pcsc_context *ctx, mf_interface *intf);
