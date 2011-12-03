#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <winscard.h>
#include <libmf.h>

#include "util.h"

struct pcsc_context {
	SCARDCONTEXT pcsc_ctx;

	SCARDHANDLE crd;
	SCARD_IO_REQUEST proto;
};

static ssize_t pcsc_send(void *tr_data, uint8_t *data, size_t dlen, uint8_t **out) {
	struct pcsc_context *ctx = (struct pcsc_context*)tr_data;
	static uint8_t buf[256];
	DWORD len = 256;

	LONG rv = SCardTransmit(ctx->crd, &ctx->proto, data, dlen, NULL, buf, &len);
	if(rv != SCARD_S_SUCCESS) {
		debug("SCardTransmit: %s", pcsc_stringify_error(rv));
		return -1;
	}

	*out = buf;
	return len;
}

static void await_reader_change(struct pcsc_context *ctx) {
	SCARD_READERSTATE rs = {
		.szReader = "\\\\?PnP?\\Notification",
		.dwCurrentState = SCARD_STATE_UNAWARE
	};

	LONG rv = SCardGetStatusChange(ctx->pcsc_ctx, INFINITE, &rs, 1);
	if(rv != SCARD_S_SUCCESS) {
		die("await_reader_change() SCardGetStatusChange: %s", pcsc_stringify_error(rv));
	}
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

		free(ctx);
		return NULL;
	}

	return ctx;
}

static char *get_reader(struct pcsc_context *ctx) {
	while(1) {
		DWORD dwReaders;
		LONG rv = SCardListReaders(ctx->pcsc_ctx, NULL, NULL, &dwReaders);
		if(rv == SCARD_E_NO_READERS_AVAILABLE)
			goto no_reader;

		if(rv != SCARD_S_SUCCESS) {
			die("SCardListReaders: %s", pcsc_stringify_error(rv));
		}

		char *readers = malloc(dwReaders);
		rv = SCardListReaders(ctx->pcsc_ctx, NULL, readers, &dwReaders);
		if(rv == SCARD_E_NO_READERS_AVAILABLE) {
			free(readers);
			goto no_reader;
		}

		if(rv != SCARD_S_SUCCESS) {
			die("SCardListReaders: %s", pcsc_stringify_error(rv));
		}

		char *pattern = getenv("PCSC_READER_PATTERN");
		if(!pattern) {
			return readers;
		}

		for(char *p = readers; p - readers < dwReaders; ) {
			if(strstr(p, pattern)) {
				char *reader = strdup(p);
				free(readers);
				return reader;
			}

			p += strlen(p) + 1;
		}

		free(readers);

no_reader:
		log("Specified reader not found, waiting for change");
		await_reader_change(ctx);
		log("Got a reader-event");
	}
}

mf_interface *pcsc_wait(struct pcsc_context *ctx) {
	SCARD_READERSTATE rs;
	memset(&rs, 0, sizeof(SCARD_READERSTATE));

	char *reader = get_reader(ctx);

	rs.szReader = reader;
	rs.dwCurrentState = SCARD_STATE_UNAWARE;

	do {
		LONG rv = SCardGetStatusChange(ctx->pcsc_ctx, INFINITE, &rs, 1);
		if(rv != SCARD_S_SUCCESS) {
			debug("SCardGetStatusChange: %s", pcsc_stringify_error(rv));

			goto error;
		}

		if(rs.dwEventState & (SCARD_STATE_UNAVAILABLE | SCARD_STATE_UNKNOWN)) {
			log("Reader is offline");

			// TODO: Do fancy powercycling here?
			log("Sleeping for 60 seconds in hope of self-healing");
			sleep(60);

			goto error;
		}

		rs.dwCurrentState = rs.dwEventState;
	} while(!(rs.dwEventState & SCARD_STATE_PRESENT));

	DWORD proto;
	LONG rv = SCardConnect(ctx->pcsc_ctx, reader, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &ctx->crd, &proto);
	if(rv != SCARD_S_SUCCESS) {
		debug("SCardConnect: %s", pcsc_stringify_error(rv));
		goto error;
	}

	free(reader);

	switch(proto) {
	case SCARD_PROTOCOL_T0:
		ctx->proto = *SCARD_PCI_T0;
		break;

	case SCARD_PROTOCOL_T1:
		ctx->proto = *SCARD_PCI_T1;
		break;
	}

	return mf_interface_new(pcsc_send, ctx);

error:
	free(reader);
	return NULL;
}

void pcsc_close(struct pcsc_context *ctx, mf_interface *intf) {
	mf_interface_free(intf);

	SCardDisconnect(ctx->crd, SCARD_UNPOWER_CARD);
}
