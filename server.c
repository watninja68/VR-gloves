#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 7001
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Function to extract timestamp from data line
unsigned long get_timestamp(const char* line) {
    char* last_semicolon = strrchr(line, ';');
    if (last_semicolon) {
        return strtoul(last_semicolon + 1, NULL, 10);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <data_file>\n", argv[0]);
        return 1;
    }

    // Open data file
    FILE* data_file = fopen(argv[1], "r");
    if (!data_file) {
        printf("Error opening file: %s\n", argv[1]);
        return 1;
    }

    WSADATA wsaData;
    SOCKET server_socket, client_socket[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    int addrlen, i;
    char buffer[BUFFER_SIZE];

    // Initialize all client_socket[] to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        fclose(data_file);
        exit(EXIT_FAILURE);
    }

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        fclose(data_file);
        exit(EXIT_FAILURE);
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind server socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        fclose(data_file);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        fclose(data_file);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    printf("Waiting for Unity client to connect...\n");

    addrlen = sizeof(client_addr);

    // Accept connection
    SOCKET new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addrlen);
    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        fclose(data_file);
        exit(EXIT_FAILURE);
    }

    printf("Client connected from IP: %s, Port: %d\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));

    // Main data sending loop
    unsigned long last_timestamp = 0;
    char line[BUFFER_SIZE];
    int running = 1;

    while (running) {
        if (fgets(line, BUFFER_SIZE, data_file)) {
            // Get current timestamp
            unsigned long current_timestamp = get_timestamp(line);
            
            // Calculate and apply delay
            if (last_timestamp != 0) {
                unsigned long delay = current_timestamp - last_timestamp;
                Sleep(delay); // Windows Sleep function (takes milliseconds)
            }

            // Send data
            if (send(new_socket, line, strlen(line), 0) == SOCKET_ERROR) {
                printf("Client disconnected. Error Code: %d\n", WSAGetLastError());
                break;
            }
            
            last_timestamp = current_timestamp;
        } else {
            // Reset file position when reaching EOF
            rewind(data_file);
            last_timestamp = 0;
            printf("Reached end of file, starting over...\n");
        }

        // Quick check for socket status
        char test;
        if (recv(new_socket, &test, 1, MSG_PEEK) == 0) {
            printf("Client disconnected.\n");
            break;
        }
    }

    // Cleanup
    closesocket(new_socket);
    closesocket(server_socket);
    WSACleanup();
    fclose(data_file);
    
    return 0;
}