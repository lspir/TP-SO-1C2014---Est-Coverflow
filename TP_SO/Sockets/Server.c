
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "bibSockets.h"

#define BACKLOG 5
#define PACKAGESIZE 1024

int main(int argc, char **argv){
	int listenningSocket;
	char *PUERTO=argv[1];

	crearSocketL(&listenningSocket, PUERTO);
	listen(listenningSocket, BACKLOG);

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);

	char package[PACKAGESIZE];
	int status = 1;

	printf("Cliente conectado. Esperando mensajes:\n");

	while (strncmp(package,"Ya, estamos",11)){
		status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
		if ((status != 0)) printf("%s", package);
	}
	close(socketCliente);
	close(listenningSocket);
	return 0;
}