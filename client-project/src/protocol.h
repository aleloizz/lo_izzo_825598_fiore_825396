/*
 * protocol.h
 *
 * Client header file
 * Definitions, constants and function prototypes for the client
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Parametri condivisi dell'applicazione (valori di default)
// Queste definizioni vengono usate sia dal client che dal server
#define DEFAULT_SERVER_PORT 57015
#define DEFAULT_SERVER_IP "192.168.1.105"
#define BUFFERSIZE 512    // Buffer size for messages

typedef struct {
    char type;        // 't', 'h', 'w', 'p'
    char city[64];    // null-terminated city name
} weather_request_t;

typedef struct {
    unsigned int status; // 0 success, 1 city not available, 2 invalid request
    char type;           // echo of request type
    float value;         // weather value
} weather_response_t;

// Status codes
#define STATUS_SUCCESS 0
#define STATUS_CITY_NOT_AVAILABLE 1
#define STATUS_INVALID_REQUEST 2

// Prototipi delle funzioni di generazione dati usate dal server

float get_temperature(void); // -10.0 .. 40.0
float get_humidity(void);    // 20.0 .. 100.0
float get_wind(void);        // 0.0 .. 100.0
float get_pressure(void);    // 950.0 .. 1050.0

#endif /* PROTOCOL_H_ */
