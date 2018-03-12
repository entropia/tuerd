#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#include "door.h"
#include "util.h"


int send_lockd(char *data) {
	char* filename = getenv("LOCKD_SOCKET");

	struct sockaddr_un client_addr;
	client_addr.sun_family = AF_UNIX;
	strcpy(client_addr.sun_path, filename);

	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
	    perror("opening stream socket failed");
	    return EXIT_FAILURE;
	}

	if (connect(sock, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_un)) < 0) {
		perror("opening stream socket failed");
		close(sock);
		return EXIT_FAILURE;
	}

	if (write(sock, data, strlen(data)) < 0) {
	    perror("writing on stream socket failed");
		close(sock);
		return EXIT_FAILURE;
	}

	close(sock);
	return EXIT_SUCCESS;
}


void open_door(void) {
	if(send_lockd("O") > 0)
		perror("sending unlock signal failed");
		// SIGUSR2 on usblockd
}

void powercycle_reader(void) {
	if(send_lockd("P") > 0)
		perror("sending powercycle signal failed");
		// SIGHUP on usblockd
}

void close_door(void) {
	if(send_lockd("C") > 0)
		perror("sending lock signal failed");
		// SIGUSR1 on usblockd
}


void authentication_failed(void) {
	if(send_lockd("E") > 0)
		perror("sending powercycle signal failed");
		// Not supported on usblockd
}
