# Gesture-Based Control and Communication System

## Overview

This project is a collection of C and Python modules designed for capturing, processing, simulating, transmitting, and interpreting gesture data. The primary goal is to enable interaction or communication (e.g., text-to-speech output) based on physical gestures. The system components cover sensor data acquisition (specifically from MPU6050 IMUs), data mapping and scaling, network communication for data streaming (with a focus on Windows Sockets for TCP/IP, potentially for Unity integration), data simulation for development and testing, and Python-based gesture interpretation and voice output.

## Components

The project consists of the following key files:

### 1. `maping.c`

This file contains two distinct pieces of code:

* **Part 1: C Utility Functions**
    * `mapps(float val, float *mins, float *maxx)`: A C function that dynamically updates the minimum and maximum values encountered so far from a stream of `val` inputs.
    * `map(float val, float in_min, float in_max, float out_min, float out_max)`: A C function that performs linear scaling, remapping `val` from an input range (`in_min` to `in_max`) to an output range (`out_min` to `out_max`).
    * A `main()` function demonstrates the usage of `mapps` and `map`.

* **Part 2: Arduino MPU6050 Sketch**
    * This is an Arduino (C++) sketch designed to interface with three Adafruit MPU6050 gyroscope/accelerometer sensors.
    * **Sensor Initialization & Selection**: Initializes I2C communication and the MPU6050 sensors. It uses GPIO pins (15, 13, 12) to presumably select one active sensor at a time by toggling their power or an enable pin (though the exact mechanism for selection via these pins beyond `digitalWrite` needs MPU6050 specific wiring).
    * **Data Acquisition**: Reads accelerometer and gyroscope data from the selected MPU6050 sensor.
    * **Orientation Calculation**:
        * Calculates `accAngleX` and `accAngleY` from accelerometer data.
        * Calculates `gyroAngleX`, `gyroAngleY`, and `yaw` from gyroscope data, taking into account elapsed time.
        * Implements a complementary filter to combine accelerometer and gyroscope data for more stable pitch and roll estimations.
    * **Dynamic Scaling**:
        * Utilizes local versions of `mapps` and `map` functions.
        * The `accToDisp` function attempts to update global `mins` and `maxx` variables (though these seem to have scoping issues in the provided snippet, as `mins`/`maxx` are not explicitly declared globally or passed effectively for the entire system's dynamic range in `accToDisp`).
        * The `mapValues` function then scales the pitch, roll, and yaw values from all three sensors to a 0-360 degree range using these `mins` and `maxx` values.
    * **Output**: Prints the mapped pitch, roll, and yaw values for each sensor to the Serial monitor.

### 2. `send.c`

* **Purpose**: A Windows C application that simulates and transmits hand tracking data over TCP/IP.
* **Functionality**:
    * Defines data structures `PRYData` (Pitch, Roll, Yaw) and `HandFrame` (PRY data for thumb, index, middle, ring, pinky fingers, and palm).
    * `generateHandFrame()`: Generates simulated, smoothly varying PRY data for each part of the hand based on a frame number, mimicking motion over time.
    * `generateDataFile()`: Creates a text file (`hand_tracking_data.txt`) containing a specified number of frames (default 30,000) of the simulated hand data. Each line includes PRY data for all parts and a timestamp.
    * **TCP Server**: Initializes Winsock and sets up a TCP server on port 7001.
    * **Data Streaming**: Waits for a single client (e.g., a Unity application) to connect. Once connected, it reads the `hand_tracking_data.txt` file line by line and sends each line to the client.
    * **Rate Control**: Attempts to simulate real-time data streaming by extracting timestamps from the data file and using `Sleep()` to introduce delays between sending frames, matching the `DATA_RATE` (90 Hz).
* **Dependencies**: Windows Sockets (`ws2_32.lib`).

### 3. `server.c`

* **Purpose**: A Windows C TCP server that streams data from a specified file to a connected client.
* **Functionality**:
    * Takes a data file path as a command-line argument.
    * Initializes Winsock and sets up a TCP server on port 7001.
    * Waits for a single client to connect.
    * **Data Streaming**: Reads the specified data file line by line.
    * `get_timestamp()`: Extracts a timestamp assumed to be at the end of each line after the last semicolon.
    * **Rate Control**: Calculates the delay required between sending lines based on the difference between current and last timestamps and uses `Sleep()`.
    * **Looping**: If it reaches the end of the data file, it rewinds the file pointer and continues streaming from the beginning.
    * Monitors client connection and stops if the client disconnects or a send error occurs.
* **Dependencies**: Windows Sockets (`ws2_32.lib`).

### 4. `temp_server.c`

* **Purpose**: A more general-purpose Windows C TCP server capable of handling multiple clients simultaneously.
* **Functionality**:
    * Initializes Winsock and sets up a TCP server on port 6969.
    * Uses the `select()` function to manage multiple client connections and I/O operations without blocking.
    * **Client Handling**: Accepts new client connections up to `MAX_CLIENTS`.
    * **Message Broadcasting**: When data is received from a connected client, it prints the message to the server console and then broadcasts this message to all other connected clients.
    * **Connection Management**: Detects client disconnections and closes their sockets.
    * **Data Rate Monitoring**: Includes a simple counter (`data_count`) and timer to print the number of messages received per second.
* **Dependencies**: Windows Sockets (`ws2_32.lib`), `time.h`.

### 5. `text_speak_gesture.py`

* **Purpose**: A Python script for interpreting gesture data and converting it to speech using `pyttsx3`.
* **Functionality**:
    * `interpret_gesture(gesture_values)`: Takes a list of 9 float values (assumed to be pitch, roll, yaw for three fingers like thumb, point, middle). It contains example logic to map these values to predefined text messages (e.g., "Hello", "Goodbye"). This function would need to be customized with actual gesture recognition logic.
    * `speak_text(text)`: Uses the `pyttsx3` engine to convert the input text string to audible speech.
    * The script demonstrates the process by interpreting a sample `gesture_values` list and speaking the result.
* **Dependencies**: `pyttsx3`.

### 6. `tts.py`

* **Purpose**: A Python script demonstrating higher-quality text-to-speech synthesis using the Hugging Face `transformers` library.
* **Functionality**:
    * Uses the "suno/bark-small" text-to-speech model via the `transformers` pipeline.
    * Synthesizes speech from a sample text string ("Hello, my dog is cooler than you!").
    * Plays the generated audio using the `sounddevice` library. Audio data is converted to the appropriate format for playback.
* **Dependencies**: `transformers`, `sounddevice`, `numpy`, and a PyTorch backend for the model.

### 7. `test.py`

* **Purpose**: A simple Python script for UI automation or testing using `pyautogui`.
* **Functionality**:
    * Prints "1" to the console.
    * Waits for 3 seconds.
    * Simulates pressing the 'right' arrow key three times, with a 0.1-second pause between presses.
    * Prints "2" to the console.
    * This script could be used to test if gesture inputs can be translated into keyboard commands for controlling applications.
* **Dependencies**: `pyautogui`.

## Potential Workflow / System Architecture (Conceptual)

1.  **Gesture Input**:
    * The Arduino sketch in `maping.c` runs on a microcontroller with MPU6050 sensors, capturing and processing raw motion data into pitch, roll, and yaw.
    * This data would then be sent to a PC (e.g., via Serial communication, not explicitly implemented in the provided snippets for network sending from Arduino).

2.  **Data Transmission & Server**:
    * On the PC, a C server application (`server.c` or `send.c`) listens for incoming sensor data (if real sensors are used) or generates/streams simulated data (`send.c`).
    * This server transmits the (possibly formatted) PRY data over TCP to a client application (e.g., a Unity environment for 3D hand visualization or a Python script for gesture interpretation).
    * `temp_server.c` could be used if multiple applications need to consume the data stream or for other server-client interactions.

3.  **Gesture Interpretation**:
    * A client application (Python script or integrated into Unity) receives the PRY data.
    * The `text_speak_gesture.py` script (or a more sophisticated model) uses this data in its `interpret_gesture` function to identify a specific gesture.

4.  **Action/Feedback**:
    * **Text-to-Speech**: The interpreted gesture (as text) is converted into speech using `pyttsx3` (from `text_speak_gesture.py`) or a more advanced TTS engine like `suno/bark` (demonstrated in `tts.py`).
    * **UI Control**: Gestures could be mapped to actions like those in `test.py` (e.g., keyboard presses) to control applications.

## Setup & Prerequisites

* **C/C++ Development**:
    * A C/C++ compiler (e.g., GCC, MinGW for Windows).
    * For Windows socket programming (`send.c`, `server.c`, `temp_server.c`): Link against `ws2_32.lib`.
* **Arduino Development**:
    * Arduino IDE.
    * Libraries: `Wire`, `Adafruit_MPU6050`, `Adafruit_Sensor`.
* **Python 3.x**:
    * Install required packages using pip:
        ```bash
        pip install pyttsx3
        pip install transformers torch # Consult PyTorch website for specific CUDA/CPU versions if needed
        pip install sounddevice numpy
        pip install pyautogui
        ```

## How to Use (General Guidance)

1.  **Sensor Module (`maping.c` - Arduino part)**:
    * Connect MPU6050 sensors to your Arduino board.
    * Compile and upload the Arduino sketch using the Arduino IDE.
    * Monitor Serial output for mapped pitch, roll, and yaw values.
    * Modify the sketch to send data over Serial to be read by a PC application, or directly via network if an ESP32/ESP8266 with WiFi is used.

2.  **Data Simulation & Streaming (`send.c`)**:
    * Compile `send.c` using a C compiler on Windows (e.g., MinGW: `gcc send.c -o send.exe -lws2_32`).
    * Run `send.exe`. It will first generate `hand_tracking_data.txt` and then wait for a client to connect on port 7001 to stream the data.

3.  **File-Based Data Streaming (`server.c`)**:
    * Compile `server.c` (e.g., `gcc server.c -o server.exe -lws2_32`).
    * Prepare a data file (e.g., `hand_tracking_data.txt` from `send.c`, or your own sensor logs).
    * Run `server.exe your_data_file.txt`. It will wait for a client on port 7001.

4.  **Multi-Client Server (`temp_server.c`)**:
    * Compile `temp_server.c` (e.g., `gcc temp_server.c -o temp_server.exe -lws2_32`).
    * Run `temp_server.exe`. It will listen on port 6969 for multiple clients.

5.  **Gesture to Speech (`text_speak_gesture.py`)**:
    * Ensure `pyttsx3` is installed.
    * Run `python text_speak_gesture.py`.

6.  **Advanced TTS (`tts.py`)**:
    * Ensure `transformers`, `torch`, `sounddevice`, and `numpy` are installed.
    * Run `python tts.py` to hear the sample sentence synthesized.

7.  **UI Automation Test (`test.py`)**:
    * Ensure `pyautogui` is installed.
    * Run `python test.py`.
    * Be prepared for it to take control of your mouse/keyboard for a moment to press the right arrow key.

This collection of modules provides a flexible toolkit for experimenting with gesture-based interactions and communication systems.
