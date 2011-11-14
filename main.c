#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libmf.h>
#include <winscard.h>
#include <zmq.h>

#define KEYSTORE_ENDPOINT "tcp://127.0.0.1:4242"
#define DOOR_ENDPOINT "tcp://127.0.0.1:4242"

struct pcsc_state {
	SCARDHANDLE crd;
	SCARD_IO_REQUEST proto;
};

ssize_t pcsc_send(void *tr_data, uint8_t *data, size_t dlen, uint8_t **out) {
	struct pcsc_state *state = (struct pcsc_state*)tr_data;
	static uint8_t buf[256];
	DWORD len = 256;

	LONG rv = SCardTransmit(state->crd, &state->proto, data, dlen, NULL, buf, &len);
	if(rv != SCARD_S_SUCCESS) {
		fprintf(stderr, "SCardTransmit: %s\n", pcsc_stringify_error(rv));
		return -1;
	}

	*out = buf;
	return len;
}

SCARDCONTEXT pcsc_init(void) {
	SCARDCONTEXT ctx;

	LONG rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &ctx);
	if(rv != SCARD_S_SUCCESS) {
		fprintf(stderr, "SCardEstablishContext: %s\n", pcsc_stringify_error(rv));
		exit(EXIT_FAILURE);
	}

	return ctx;
}

unsigned char *pcsc_get_reader(SCARDCONTEXT ctx) {
	DWORD dwReaders;

	LONG rv = SCardListReaders(ctx, NULL, NULL, &dwReaders);
	if(rv != SCARD_S_SUCCESS) {
		fprintf(stderr, "SCardListReaders: %s\n", pcsc_stringify_error(rv));
		exit(EXIT_FAILURE);
	}

	unsigned char *reader = malloc(dwReaders);
	rv = SCardListReaders(ctx, NULL, reader, &dwReaders);
	if(rv != SCARD_S_SUCCESS) {
		fprintf(stderr, "SCardListReaders: %s\n", pcsc_stringify_error(rv));
		exit(EXIT_FAILURE);
	}

	return reader;
}

int pcsc_wait(SCARDCONTEXT ctx, const unsigned char *reader, struct pcsc_state *state) {
	SCARD_READERSTATE rs;
	memset(&rs, 0, sizeof(SCARD_READERSTATE));

	rs.szReader = reader;
	rs.dwCurrentState = SCARD_STATE_UNAWARE;

	do {
		LONG rv = SCardGetStatusChange(ctx, INFINITE, &rs, 1);
		if(rv != SCARD_S_SUCCESS) {
			fprintf(stderr, "SCardGetStatusChange: %s\n", pcsc_stringify_error(rv));
		}

		rs.dwCurrentState = rs.dwEventState;
	} while(!(rs.dwEventState & SCARD_STATE_PRESENT));

	DWORD proto;
        LONG rv = SCardConnect(ctx, reader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &state->crd, &proto);
        if(rv != SCARD_S_SUCCESS) {
                fprintf(stderr, "SCardConnect: %s\n", pcsc_stringify_error(rv));
		return -1;
        }

        switch(proto) {
        case SCARD_PROTOCOL_T0:
                state->proto = *SCARD_PCI_T0;
                break;

        case SCARD_PROTOCOL_T1:
                state->proto = *SCARD_PCI_T1;
                break;
        }

	return 0;
}

void print_uid(mf_version *v) {
	for(int i=0; i < 7; i++)
		fprintf(stderr, "%X", v->uid[i]);
}

int get_door_key(void *s, mf_version *v, mf_key_t *key) {
	zmq_msg_t m;
	if(zmq_msg_init_size(&m, 7) < 0) {
		fprintf(stderr, "zmq_msg_init_size: %s\n", zmq_strerror(errno));
	        return -1;
        }

	memcpy(zmq_msg_data(&m), v->uid, 7);

	if(zmq_send(s, &m, 0) < 0) {
		fprintf(stderr, "zmq_send: %s\n", zmq_strerror(errno));
	        return -1;
        }

	if(zmq_msg_close(&m) < 0) {
		fprintf(stderr, "zmq_msg_close: %s\n", zmq_strerror(errno));
	        return -1;
        }

	if(zmq_msg_init(&m) < 0) {
		fprintf(stderr, "zmq_msg_init: %s\n", zmq_strerror(errno));
	        return -1;
        }

	if(zmq_recv(s, &m, 0) < 0) {
		fprintf(stderr, "zmq_recv: %s\n", zmq_strerror(errno));
	        return -1;
        }

	unsigned char *resp = (unsigned char*)zmq_msg_data(&m);
	if(resp[0] == 't' && zmq_msg_size(&m) == 17) {
		memcpy(*key, resp+1, 16);

		zmq_msg_close(&m);
		return 0;
	}

	fprintf(stderr, "Disallowed UID: ");
	print_uid(v);
	fprintf(stderr, "\n");

	zmq_msg_close(&m);
	return -2; //authentication failed
}

int main(int argc, char **argv) {
	struct pcsc_state state;

	void *zmq_ctx = zmq_init(1);
	if(!zmq_ctx) {
		fprintf(stderr, "zmq_init: %s\n", zmq_strerror(errno));
		return EXIT_FAILURE;
	}

	SCARDCONTEXT pcsc_ctx = pcsc_init();
	unsigned char *reader = pcsc_get_reader(pcsc_ctx);

	mf_interface *intf = mf_interface_new(pcsc_send,(void*)&state);

        void *zmq_keystore = zmq_socket(zmq_ctx, ZMQ_REQ);
        if(!zmq_keystore) {
	        fprintf(stderr, "zmq_socket(ZMQ_REQ): %s\n", zmq_strerror(errno));
	        return EXIT_FAILURE;
        }

        if(zmq_connect(zmq_keystore, KEYSTORE_ENDPOINT) < 0) {
	        fprintf(stderr, "zmq_connect(KEYSTORE): %s\n", zmq_strerror(errno));
	        return EXIT_FAILURE;
        }

        void *zmq_door = zmq_socket(zmq_ctx, ZMQ_REQ);
        if(!zmq_door) {
	        fprintf(stderr, "zmq_socket(ZMQ_REQ): %s\n", zmq_strerror(errno));
	        return EXIT_FAILURE;
        }

        if(zmq_connect(zmq_door, DOOR_ENDPOINT) < 0) {
	        fprintf(stderr, "zmq_connect(KEYSTORE): %s\n", zmq_strerror(errno));
	        return EXIT_FAILURE;
        }

        while(1) {
		fprintf(stdout, "Waiting for card\n");
	        if(pcsc_wait(pcsc_ctx, reader, &state) < 0)
			continue;

	        mf_version v;
	        mf_err_t ret;
	        ret = mf_get_version(intf, &v);
		if(ret != MF_OK) {
			fprintf(stderr, "mf_get_version: %s\n", mf_error_str(ret));
			goto out;
		}

	        int i;
	        printf("Got card: ");
	        for(i=0; i<7; i++) {
		        printf("%02X", v.uid[i]);
	        }
	        printf("\n");

	        mf_key_t door_key;
	        if(get_door_key(zmq_keystore, &v, &door_key) < 0)
		        goto out;

	        mf_err_t ret;
	        ret = mf_select_application(intf, 0xCA0523);
	        if(ret != MF_OK) {
		        fprintf(stderr, "mf_select_application: %s\n", mf_error_str(ret));
		        goto out;
	        }

	        ret = mf_authenticate(intf, 0xD, door_key, NULL);
	        if(ret != MF_OK) {
		        if(ret == MF_ERR_AUTHENTICATION_ERROR) {
			        fprintf(stderr, "Authentication failed for UID ");
			        print_uid(&v);
			        fprintf(stderr, "\n");
		        }
		        else
			        fprintf(stderr, "mf_select_application: %s\n", mf_error_str(ret));
	        }
		else {
		        printf("alles gut <3\n");
		}

        out:
	        SCardDisconnect(state.crd, SCARD_EJECT_CARD);

	        sleep(2);
	}
}
