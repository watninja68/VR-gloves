#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>  // Include for time functions

#pragma comment(lib, "ws2_32.lib")

#define PORT  6969
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET server_socket, new_socket, client_socket[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    int addrlen, activity, i, valread, sd;
    int max_sd;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    int data_count = 0; // Counter for received data
    time_t start_time = time(NULL);  // Record the start time

    // Initialize all client_socket[] to 0 so that no socket is active at the start
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind server socket to the specified address and port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_socket, 3) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    addrlen = sizeof(client_addr);

    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        // Add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            // If valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // Highest file descriptor number for select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // Wait for activity on any of the sockets, with no timeout
        activity = select(0, &readfds, NULL, NULL, NULL);

        if ((activity == SOCKET_ERROR) && (WSAGetLastError() != WSAEINTR)) {
            printf("Select error. Error Code: %d\n", WSAGetLastError());
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // If something happened on the master socket, it is an incoming connection
        if (FD_ISSET(server_socket, &readfds)) {
            if ((new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addrlen)) == INVALID_SOCKET) {
                printf("Accept failed. Error Code: %d\n", WSAGetLastError());
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is : %s, port : %d\n",
                   new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // Check if there is data to be read on any of the client sockets
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                // Check if it was for closing, and also read the incoming message
                valread = recv(sd, buffer, BUFFER_SIZE, 0);

                if (valread == SOCKET_ERROR) {
                    int error_code = WSAGetLastError();
                    if (error_code == WSAECONNRESET) {
                        // Client disconnected, get details and close the socket
                        getpeername(sd, (struct sockaddr*)&client_addr, &addrlen);
                        printf("Client disconnected, ip %s, port %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        closesocket(sd);
                        client_socket[i] = 0;
                    } else {
                        printf("Recv failed. Error Code: %d\n", error_code);
                    }
                } else if (valread == 0) {
                    // Client closed connection
                    getpeername(sd, (struct sockaddr*)&client_addr, &addrlen);
                    printf("Client disconnected, ip %s, port %d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    closesocket(sd);
                    client_socket[i] = 0;
                } else {
                    // Set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    printf("Received message: %s\n", buffer);

                    // Increment the data counter for each message received
                    data_count++;
                    printf("Data received in the last second: %d messages\n", data_count);

                    // Broadcast the message to all clients except the sender
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_socket[j] != 0 && client_socket[j] != sd) {
                            send(client_socket[j], buffer, strlen(buffer), 0);
                        }
                    }
                }
            }
        }

        // Check if one second has passed
        if (difftime(time(NULL), start_time) >= 1.0) {
            printf("Data received in the last second: %d messages\n", data_count);
            data_count = 0;  // Reset the counter
            start_time = time(NULL);  // Reset the timer
        }
    }

    // Clean up Winsock
    WSACleanup();

    return 0;
}