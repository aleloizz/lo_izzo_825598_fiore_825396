/*
 * protocol.h
 *
 * Server header file
 * Definitions, constants and function prototypes for the server
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define SERVER_PORT 57015  // Server port (change if needed)
#define BUFFER_SIZE 512    // Buffer size for messages
#define QUEUE_SIZE 5       // Size of pending connections queue

// Function prototypes
// Add here the signatures of the functions implemented by students

typedef struct {
    unsigned int stato;  // Response status code
    char tipo;            // Echo of request type
    float valore;          // Weather data value
} risposta_meteo_t;

int handleclientconnection(int client_socket, const char *client_ip);
float typecheck(char type);
char citycheck(const char *city);

float get_temperature(void);    // Range: -10.0 to 40.0 °C
float get_humidity(void);       // Range: 20.0 to 100.0 %
float get_wind(void);           // Range: 0.0 to 100.0 km/h
float get_pressure(void);       // Range: 950.0 to 1050.0 hPa

risposta_meteo_t build_weather_response(char type, const char *city);


#endif /* PROTOCOL_H_ */
