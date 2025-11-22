/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#define PROTOPORT 57015 // default protocol port number
#define QLEN 6 // size of request queue

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
	printf ("%s", errorMessage);
}


int main(int argc, char *argv[]) {

	int port;
	if (argc > 1) {
	port = atoi(argv[1]); // if argument specified convert
	// argument to binary
	}
	else
	port = PROTOPORT; // use default port number
	if (port < 0) {
	printf("port number errata %s \n", argv[1]);
	return 0;
	}

	// TODO: Implement server logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	//creazione della socket
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (my_socket < 0) {
		errorhandler("errore nella creazione del socket.\n");
		return -1;
	}

	// assegnazione di una indirizzo alla socket
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// socket binding
	if (bind(my_socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) <0) {
		errorhandler("errore nella bind.\n");
		closesocket(my_socket);
		return -1;
	}

	// settaggio della socket in listening
	
	if (listen (my_socket, QLEN) < 0) {
		errorhandler("errore nella listen.\n");
		closesocket(my_socket);
		return -1;
	}

	// accettazione connessioni dai client
	struct sockaddr_in cad; //structure for the client address
	int client_socket;      //socket descriptor for the client
	int client_len;         //the size of the client address
	printf( "In attesa di connessioni sulla porta %d...\n", SERVER_PORT );

	while (1) {
		client_len = sizeof(cad); //set the size of the client address
		if ( (client_socket=accept(my_socket, (struct sockaddr *)&cad,
		&client_len)) < 0 ) {
			errorhandler("errore nella accept.\n");
			closesocket(my_socket);	//chiusura della connesione
			clearwinsock();
			return -1;
		}
		// clientSocket is connected to a client
		printf( "Gestione del client %s\n", inet_ntoa(cad.sin_addr) );
		handleclientconnection(client_socket);
	}// end of the while loop

	printf("Server terminato.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
