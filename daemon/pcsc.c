#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <winscard.h>
#include <libmf.h>

#include "util.h"

struct pcsc_context {
	SCARDCONTEXT pcsc_ctx;
	char *reader;

	SCARDHANDLE crd;
	SCARD_IO_REQUEST proto;
};

static ssize_t pcsc_send(void *tr_data, uint8_t *data, size_t dlen, uint8_t **out) {
	struct pcsc_context *ctx = (struct pcsc_context*)tr_data;
	static uint8_t buf[256];
	DWORD len = 256;

	LONG rv = SCardTransmit(ctx->crd, &ctx->proto, data, dlen, NULL, buf, &len);
	if(rv != SCARD_S_SUCCESS) {
		log("SCardTransmit: %s", pcsc_stringify_error(rv));
		return -1;
	}

	*out = buf;
	return len;
}

struct pcsc_context *pcsc_init() {
	struct pcsc_context *ctx;

	ctx = malloc(sizeof(struct pcsc_context));
	if(!ctx) {
		log("allocating context failed");
		return NULL;
	}

	LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &ctx->pcsc_ctx);
	if(rv != SCARD_S_SUCCESS) {
		log("SCardEstablishContext: %s", pcsc_stringify_error(rv));
		return NULL;
	}
	
	char *supplied_reader = getenv("PCSC_READER");
	if(supplied_reader) {
		ctx->reader = strdup(supplied_reader);
		return ctx;
	}
	else {
		DWORD dwReaders;
		LONG rv = SCardListReaders(ctx->pcsc_ctx, NULL, NULL, &dwReaders);
		if(rv != SCARD_S_SUCCESS) {
			log("SCardListReaders: %s", pcsc_stringify_error(rv));
			return NULL;
		}
	
		ctx->reader = malloc(dwReaders);
		rv = SCardListReaders(ctx->pcsc_ctx, NULL, ctx->reader, &dwReaders);
		if(rv != SCARD_S_SUCCESS) {
			log("SCardListReaders: %s", pcsc_stringify_error(rv));
			return NULL;
		}
	}

	return ctx;
}

mf_interface *pcsc_wait(struct pcsc_context *ctx) {
	SCARD_READERSTATE rs;
	memset(&rs, 0, sizeof(SCARD_READERSTATE));

	rs.szReader = ctx->reader;
	rs.dwCurrentState = SCARD_STATE_UNAWARE;

	do {
		LONG rv = SCardGetStatusChange(ctx->pcsc_ctx, INFINITE, &rs, 1);
		if(rv != SCARD_S_SUCCESS) {
			log("SCardGetStatusChange: %s", pcsc_stringify_error(rv));
		}

		rs.dwCurrentState = rs.dwEventState;
	} while(!(rs.dwEventState & SCARD_STATE_PRESENT));

	DWORD proto;
	LONG rv = SCardConnect(ctx->pcsc_ctx, ctx->reader, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &ctx->crd, &proto);
	if(rv != SCARD_S_SUCCESS) {
		log("SCardConnect: %s", pcsc_stringify_error(rv));
		return NULL;
	}

	switch(proto) {
	case SCARD_PROTOCOL_T0:
		ctx->proto = *SCARD_PCI_T0;
		break;

	case SCARD_PROTOCOL_T1:
		ctx->proto = *SCARD_PCI_T1;
		break;
	}

	return mf_interface_new(pcsc_send, ctx);
}

void pcsc_close(struct pcsc_context *ctx, mf_interface *intf) {
	mf_interface_free(intf);

	SCardDisconnect(ctx->crd, SCARD_UNPOWER_CARD);
}
