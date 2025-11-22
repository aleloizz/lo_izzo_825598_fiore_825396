/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
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

#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

#define NO_ERROR 0

void errorhandler(char *error_message)
{
	printf("%s", error_message);
}

void clearwinsock()
{
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[])
{
	// TODO: Implement client logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	// Socket creation
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	// Server address configuration
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	// Connect to server
	if (connect(my_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		errorhandler("Failed to connect.\n");
		closesocket(my_socket);
		clearwinsock();
		return 0;
	}

	char *input_string = "I'm in your system"; // Stringa da inviare
	int string_len = strlen(input_string); // Determina la lunghezza

	// TODO: Implement communication logic
	// INVIARE DATI AL SERVER
	if (send(my_socket, input_string, string_len, 0) != string_len) {
		errorhandler("send() sent a different number of bytes than expected");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	// RICEVERE DATI DAL SERVER
	int bytes_rcvd;
	int total_bytes_rcvd = 0;
	char buf[BUFFERSIZE]; // buffer for data from the server
	memset(buf, 0, BUFFERSIZE); // ensures extra bytes contain 0
	printf("Received: "); // Setup to print the echoed string

	while (total_bytes_rcvd < string_len) {
		if ((bytes_rcvd = recv(my_socket, buf, BUFFERSIZE - 1, 0)) <= 0) {
			errorhandler("recv() failed or connection closed prematurely");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}
		total_bytes_rcvd += bytes_rcvd; // Keep tally of total bytes
	}
	buf[total_bytes_rcvd] = '\0'; // terminate string
	printf("%s", buf); // Print the echo buffer

	// CHIUSURA DELLA CONNESSIONE
	closesocket(my_socket);
	clearwinsock();
	printf("Client terminated.\n");
#if defined WIN32
	system("pause");
#endif
	return 0;
} // main end
