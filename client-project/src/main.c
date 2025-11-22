/* main.c
 * TCP Weather Client
 * Sends a fixed-size request (1 + 64 bytes) and receives a fixed-size response (4 + 1 + 4 bytes)
 * Compatible with Windows (MinGW), Linux and macOS
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#if defined _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include "protocol.h"

// On Windows console the degree symbol may appear corrupted; use ASCII fallback
#if defined _WIN32
#define DEG_C_SUFFIX "C"
#else
#define DEG_C_SUFFIX "°C"
#endif

static void print_usage(const char *progname)
{
    printf("Usage: %s [-s server] [-p port] -r \"type city\"\n", progname);
}

static int send_all(int sock, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    size_t remaining = len;
    while (remaining > 0) {
        int sent = send(sock, p, (int)remaining, 0);
        if (sent <= 0) return -1;
        p += sent;
        remaining -= sent;
    }
    return 0;
}

static int recv_all(int sock, void *buf, size_t len)
{
    char *p = (char *)buf;
    size_t remaining = len;
    while (remaining > 0) {
        int r = recv(sock, p, (int)remaining, 0);
        if (r <= 0) return -1;
        p += r;
        remaining -= r;
    }
    return 0;
}

static float ntohf(uint32_t i)
{
    i = ntohl(i);
    float f;
    memcpy(&f, &i, sizeof(f));
    return f;
}

int main(int argc, char *argv[])
{
    const char *server = DEFAULT_SERVER_IP;
    int port = DEFAULT_SERVER_PORT;
    const char *request = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            server = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            request = argv[++i];
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!request) {
        print_usage(argv[0]);
        return 1;
    }

#if defined _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
#endif

    // Parse request: first non-space char is type, rest is city
    const char *p = request;
    while (*p && isspace((unsigned char)*p)) p++;
    char type = *p;
    if (type == '\0') {
        print_usage(argv[0]);
        return 1;
    }
    p++;
    while (*p && isspace((unsigned char)*p)) p++;
    char city[64];
    memset(city, 0, sizeof(city));
    strncpy(city, p, sizeof(city) - 1);

    // Resolve server address (IPv4)
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, server, &server_addr.sin_addr) != 1) {
        struct hostent *he = gethostbyname(server);
        if (!he) {
            fprintf(stderr, "Failed to resolve server address\n");
#if defined _WIN32
            WSACleanup();
#endif
            return 1;
        }
        server_addr.sin_addr = *(struct in_addr *)he->h_addr_list[0];
    }

    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket");
#if defined _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        closesocket(sock);
#if defined _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Prepare and send request: fixed 65 bytes (1 type + 64 city)
    unsigned char reqbuf[65];
    memset(reqbuf, 0, sizeof(reqbuf));
    reqbuf[0] = (unsigned char)type;
    strncpy((char *)&reqbuf[1], city, 63);
    if (send_all(sock, reqbuf, sizeof(reqbuf)) != 0) {
        fprintf(stderr, "Failed to send request\n");
        closesocket(sock);
#if defined _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Receive response: 4 bytes status (network), 1 byte type, 4 bytes float
    unsigned char respbuf[9];
    if (recv_all(sock, respbuf, sizeof(respbuf)) != 0) {
        fprintf(stderr, "Failed to receive response\n");
        closesocket(sock);
#if defined _WIN32
        WSACleanup();
#endif
        return 1;
    }

    uint32_t net_status;
    memcpy(&net_status, respbuf, 4);
    uint32_t status = ntohl(net_status);
    char rtype = (char)respbuf[4];
    uint32_t net_f;
    memcpy(&net_f, &respbuf[5], 4);
    float value = ntohf(net_f);

    // Get peer IP for printing
    struct sockaddr_in peer_addr;
#if defined _WIN32
    int peer_len = sizeof(peer_addr);
#else
    socklen_t peer_len = sizeof(peer_addr);
#endif
    char peer_ip[INET_ADDRSTRLEN] = "";
    if (getpeername(sock, (struct sockaddr *)&peer_addr, &peer_len) == 0) {
        inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, sizeof(peer_ip));
    } else {
        strncpy(peer_ip, server, sizeof(peer_ip) - 1);
    }

    // Build message per spec
    char message[256];
    if (status == STATUS_SUCCESS) {
        switch (rtype) {
        case 't':
            snprintf(message, sizeof(message), "%s: Temperatura = %.1f%s", city, value, DEG_C_SUFFIX);
            break;
        case 'h':
            snprintf(message, sizeof(message), "%s: Umidità = %.1f%%", city, value);
            break;
        case 'w':
            snprintf(message, sizeof(message), "%s: Vento = %.1f km/h", city, value);
            break;
        case 'p':
            snprintf(message, sizeof(message), "%s: Pressione = %.1f hPa", city, value);
            break;
        default:
            snprintf(message, sizeof(message), "Richiesta non valida");
            break;
        }
    } else if (status == STATUS_CITY_NOT_AVAILABLE) {
        snprintf(message, sizeof(message), "Città non disponibile");
    } else if (status == STATUS_INVALID_REQUEST) {
        snprintf(message, sizeof(message), "Richiesta non valida");
    } else {
        snprintf(message, sizeof(message), "Richiesta non valida");
    }

    printf("Ricevuto risultato dal server ip %s. %s\n", peer_ip, message);

    closesocket(sock);
#if defined _WIN32
    WSACleanup();
#endif
    return 0;
}
