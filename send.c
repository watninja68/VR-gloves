#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <math.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 7001
#define BUFFER_SIZE 1024
#define DATA_RATE 90  // Hz - typical hand tracking data rate

typedef struct {
    float pitch;
    float roll;
    float yaw;
} PRYData;

typedef struct {
    PRYData thumb;
    PRYData index;
    PRYData middle;
    PRYData ring;
    PRYData pinky;
    PRYData palm;
} HandFrame;

// Function to format PRY data into string
void formatPRYString(char* buffer, const char* name, PRYData data) {
    sprintf(buffer + strlen(buffer), "%s:%.2f,%.2f,%.2f;", 
            name, data.pitch, data.roll, data.yaw);
}

// Function to generate simulated hand motion data
HandFrame generateHandFrame(int frameNum) {
    HandFrame frame;
    float t = (float)frameNum / DATA_RATE;  // Time in seconds
    
    // Generate smooth, periodic motions for each finger
    // Thumb
    frame.thumb.pitch = 45.0f * sin(0.5f * t);
    frame.thumb.roll = 30.0f * cos(0.7f * t);
    frame.thumb.yaw = 20.0f * sin(0.3f * t);
    
    // Index
    frame.index.pitch = 60.0f * sin(0.4f * t + 0.5f);
    frame.index.roll = 35.0f * cos(0.6f * t);
    frame.index.yaw = 25.0f * sin(0.5f * t);
    
    // Middle
    frame.middle.pitch = 55.0f * sin(0.3f * t + 1.0f);
    frame.middle.roll = 40.0f * cos(0.5f * t);
    frame.middle.yaw = 30.0f * sin(0.4f * t);
    
    // Ring
    frame.ring.pitch = 50.0f * sin(0.6f * t + 1.5f);
    frame.ring.roll = 35.0f * cos(0.4f * t);
    frame.ring.yaw = 25.0f * sin(0.6f * t);
    
    // Pinky
    frame.pinky.pitch = 45.0f * sin(0.7f * t + 2.0f);
    frame.pinky.roll = 30.0f * cos(0.3f * t);
    frame.pinky.yaw = 20.0f * sin(0.7f * t);
    
    // Palm
    frame.palm.pitch = 20.0f * sin(0.2f * t);
    frame.palm.roll = 15.0f * cos(0.3f * t);
    frame.palm.yaw = 30.0f * sin(0.1f * t);
    
    return frame;
}

// Function to write generated data to file
void generateDataFile(const char* filename, int numFrames) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error creating data file\n");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    for (int i = 0; i < numFrames; i++) {
        HandFrame frame = generateHandFrame(i);
        buffer[0] = '\0';  // Clear buffer
        
        // Format all finger and palm data
        formatPRYString(buffer, "Thumb", frame.thumb);
        formatPRYString(buffer, "Point", frame.index);
        formatPRYString(buffer, "Middle", frame.middle);
        formatPRYString(buffer, "Ring", frame.ring);
        formatPRYString(buffer, "Pinky", frame.pinky);
        formatPRYString(buffer, "Main", frame.palm);
        
        // Add timestamp (microseconds)
        sprintf(buffer + strlen(buffer), "%lu\n", (unsigned long)(i * (1000000.0f / DATA_RATE)));
        
        fputs(buffer, file);
    }
    
    fclose(file);
    printf("Generated %d frames of data\n", numFrames);
}

int main(int argc, char* argv[]) {
    const char* filename = "hand_tracking_data.txt";
    const int numFrames = 30000;  // Generate 30k lines of data
    
    // Generate data file first
    generateDataFile(filename, numFrames);
    
    // Open data file for streaming
    FILE* data_file = fopen(filename, "r");
    if (!data_file) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    WSADATA wsaData;
    SOCKET server_socket;
    struct sockaddr_in server_addr, client_addr;
    int addrlen;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Listen
    listen(server_socket, 1);
    printf("Waiting for Unity client connection on port %d...\n", PORT);

    // Accept connection
    addrlen = sizeof(struct sockaddr_in);
    SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addrlen);
    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Client connected!\n");

    // Main data sending loop
    char line[BUFFER_SIZE];
    unsigned long last_timestamp = 0;
    LARGE_INTEGER frequency, start_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);

    while (fgets(line, BUFFER_SIZE, data_file)) {
        // Get timestamp from the end of the line
        char* last_semicolon = strrchr(line, ';');
        if (last_semicolon) {
            unsigned long current_timestamp = strtoul(last_semicolon + 1, NULL, 10);
            
            // Calculate and apply delay
            if (last_timestamp > 0) {
                DWORD sleep_time = (DWORD)((current_timestamp - last_timestamp) / 1000); // Convert to milliseconds
                if (sleep_time > 0) {
                    Sleep(sleep_time);
                }
            }
            
            last_timestamp = current_timestamp;
        }

        // Send data
        if (send(client_socket, line, strlen(line), 0) == SOCKET_ERROR) {
            printf("Send failed\n");
            break;
        }

        printf("Sent frame\n");
    }

    // Cleanup
    fclose(data_file);
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    
    return 0;
}