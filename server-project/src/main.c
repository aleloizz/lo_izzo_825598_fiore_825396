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
#include <time.h>
#include <ctype.h>


void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
	printf ("%s", errorMessage);
}

float get_temperature(void) {
	return ((float)(rand() % 501) / 10.0f) - 10.0f; // -10.0 to 40.0 °C
}

float get_humidity(void) {
	return ((float)(rand() % 801) / 10.0f) + 20.0f; // 20.0 to 100.0 %
}

float get_wind(void) {
	return ((float)(rand() % 1001) / 10.0f); // 0.0 to 100.0 km/h
}

float get_pressure(void) {
	return ((float)(rand() % 1011) / 10.0f) + 950.0f; // 950.0 to 1050.0 hPa
}


int main(int argc, char *argv[]) {

	srand(time(NULL));
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
	server_addr.sin_addr.s_addr = inet_addr("192.168.1.105");

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
		// gestione della connessione con il client
		char *client_ip = inet_ntoa(cad.sin_addr);
		printf( "Gestione del client %s\n", client_ip );
		handleclientconnection(client_socket, client_ip);
	}// fine while loop

	printf("Server terminato.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end


int handleclientconnection(int client_socket, const char *client_ip) {
	char buffer[BUFFER_SIZE];

	// loop mer messaggi multipli in singola connesione (simple echo protocol)
	while(1) {
		int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
		if (bytes_received < 0) {
			errorhandler("Errore nella ricezione dei dati.\n");
			closesocket(client_socket);
			return -1;
		}
		if (bytes_received == 0) { // gracful close by client
			break;
		}
		// Null-terminate for safe printing
		buffer[bytes_received] = '\0';
		printf("Ricevuto dal client: %s\n", buffer);

		// Parsing prima e seconda parola come type e city
		char type[64] = "(non_definito)";
		char city[64] = "(non_definita)";
		// Salta spazi iniziali
		char *p = buffer;
		while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
		// Estrae token
		int matched = sscanf(p, "%63s %63s", type, city);
		if (matched == 1) { // solo type presente
			// city già impostata a default
		} else if (matched == 0) {
			// lascia i default
		}
		printf("Richiesta '%s %s' dal client ip %s\n", type, city, client_ip);


		// Costruzione e invio della risposta meteo secondo specifica
		char req_type = (type[0] != '\0') ? tolower((unsigned char)type[0]) : '\0';
		risposta_meteo_t response = build_weather_response(req_type, city);

		// Invio della struttura in un'unica write (gestione invio parziale se necessario)
		const char *send_ptr = (const char *)&response;
		int to_send = (int)sizeof(response);
		int sent = 0;
		while (sent < to_send) {
			int s = send(client_socket, send_ptr + sent, to_send - sent, 0);
			if (s <= 0) {
				errorhandler("Errore nell'invio della risposta.\n");
				closesocket(client_socket);
				return -1;
			}
			sent += s;
		}
	}

	closesocket(client_socket);
	return 0;
}

float typecheck(char type){
	switch (type){
		case 't':
			get_temperature();
			break;
		case 'h':
			get_humidity();
			break;
		case 'w':
			get_wind();
			break;
		case 'p':
			get_pressure();
			break;
		default:
			printf("Tipo non valido");
			return 2;
	}
	return 0;
}

char citycheck(const char *city) {
	static const char *valid_cities[] = {
		"Bari","Roma","Milano","Napoli","Torino",
		"Palermo","Genova","Bologna","Firenze","Venezia"
	};
	for (size_t i = 0; i < sizeof(valid_cities)/sizeof(valid_cities[0]); ++i) {
		const char *a = city;
		const char *b = valid_cities[i];
		while (*a && *b) {
			if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
				break;
			}
			a++; b++;
		}
		if (*a == '\0' && *b == '\0') {
			return 0; // valida (case insensitive)
		}
	}
	return 2; // non valida
}

// Funzione che combina validazione e generazione valore secondo specifica.
risposta_meteo_t build_weather_response(char type, const char *city) {
	risposta_meteo_t r;
	r.stato = 0;
	r.tipo = '\0';
	r.valore = 0.0f;

	// Validazione type
	if (type == '\0' || typecheck(type) != 0) {
		// Richiesta non valida (tipo errato)
		r.stato = 2;
		return r;
	}

	// Validazione city
	if (city == NULL || *city == '\0' || citycheck(city) != 0) {
		// Città non disponibile
		r.stato = 1;
		return r;
	}

	// Generazione valore meteo
	float value = 0.0f;
	switch (type) {
		case 't': value = get_temperature(); break;
		case 'h': value = get_humidity(); break;
		case 'w': value = get_wind(); break;
		case 'p': value = get_pressure(); break;
		default:
			// fallback (non dovrebbe accadere se typecheck ha passato)
			r.stato = 2;
			return r;
	}

	// Popolamento struttura in caso di successo
	r.stato = 0;
	r.tipo = type;
	r.valore = value;
	return r;
}