#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <zmq.h>

#define KEYSTORE_ENDPOINT "tcp://127.0.0.1:4242"

int main(int argc, char **argv) {	
	void *zmq_ctx = zmq_init(1);
	if(!zmq_ctx) {
		fprintf(stderr, "zmq_init: %s\n", zmq_strerror(errno));
		return EXIT_FAILURE;
	}
	
	void *zmq_keystore = zmq_socket(zmq_ctx, ZMQ_REP);
        if(!zmq_keystore) {
	        fprintf(stderr, "zmq_socket(ZMQ_REP): %s\n", zmq_strerror(errno));
	        return EXIT_FAILURE;
        }

        if(zmq_bind(zmq_keystore, KEYSTORE_ENDPOINT) < 0) {
	        fprintf(stderr, "zmq_bind(KEYSTORE): %s\n", zmq_strerror(errno));
	        return EXIT_FAILURE;
        }

        while(1) {
		zmq_msg_t m;
		zmq_msg_init(&m);

		if(zmq_recv(zmq_keystore, &m, 0) < 0) {
		        fprintf(stderr, "zmq_bind(KEYSTORE): %s\n", zmq_strerror(errno));
		        return EXIT_FAILURE;
	        }
		
		if(zmq_msg_size(&m) != 7)
			goto out;

		unsigned char *c = "\x04\x3E\x20\xB1\xBB\x25\x80";
		if(!memcmp(zmq_msg_data(&m), c, 7)) {
			printf("florolf!\n");
			zmq_close(&m);
			zmq_msg_init_size(&m, 17);
			memcpy(zmq_msg_data(&m), "t\x90\xA7\x58\x3E\xB1\xF6\xE1\x0B\xE0\x30\xC6\x9F\x15\x28\x92\xD3", 17);
		}
		else {
			printf("unbekannt\n");
			zmq_close(&m);
			zmq_msg_init_size(&m, 1);
			memcpy(zmq_msg_data(&m), "f", 1);
		}

		zmq_send(zmq_keystore, &m, 0);

out:
		zmq_close(&m);
	}
}
