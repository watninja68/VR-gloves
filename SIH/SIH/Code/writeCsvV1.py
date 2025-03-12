import socket
import time
import csv

def parse_pry_data(data):
    # Split the data by semicolons to get each finger's data
    finger_data = data.split(';')

    # Initialize arrays to store PRY values for each finger and the palm
    thumb_pry = [0.0, 0.0, 0.0]
    index_pry = [0.0, 0.0, 0.0]
    middle_pry = [0.0, 0.0, 0.0]
    ring_pry = [0.0, 0.0, 0.0]
    pinky_pry = [0.0, 0.0, 0.0]
    palm_pry = [0.0, 0.0, 0.0]

    for finger in finger_data:
        # Split each finger's data into the finger name and its values
        parts = finger.split(':')
        if len(parts) == 2:
            finger_name = parts[0]
            values = parts[1].split(',')

            if len(values) == 3:
                # Parse pitch, roll, and yaw as floats
                pitch = float(values[0])
                roll = float(values[1])
                yaw = float(values[2])

                # Assign values to the corresponding finger or palm arrays
                if finger_name == "Thumb":
                    thumb_pry[0], thumb_pry[1], thumb_pry[2] = pitch, roll, yaw
                elif finger_name == "Point":
                    index_pry[0], index_pry[1], index_pry[2] = pitch, roll, yaw
                elif finger_name == "Middle":
                    middle_pry[0], middle_pry[1], middle_pry[2] = pitch, roll, yaw
                elif finger_name == "Ring":
                    ring_pry[0], ring_pry[1], ring_pry[2] = pitch, roll, yaw
                elif finger_name == "Pinky":
                    pinky_pry[0], pinky_pry[1], pinky_pry[2] = pitch, roll, yaw
                elif finger_name == "Palm":
                    palm_pry[0], palm_pry[1], palm_pry[2] = pitch, roll, yaw

    return thumb_pry, index_pry, middle_pry, ring_pry, pinky_pry, palm_pry

if __name__ == "__main__":
    #s1 = "Middle:0.60,-0.20,0.00"
    #s2 = "Middle:3.60,1,0.00"
    #s3 = "Middle:0.60,-0.20,0.00"

    server_ip = '0.0.0.0'  # Listen on all interfaces
    port = 6969
    
    # Create a socket object
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind to the port
    server_socket.bind((server_ip, port))

    # Start listening for connections (max 5 queued connections)
    server_socket.listen(5)
    print("Server listening on {}:{}".format(server_ip, port))

    flag = False
    count = 0
    decoded = []
    d1 = {"Thumb":[] ,"Middle": [], "Point":[]}
    # time.sleep(1)
    while(True):
        # Establish a connection with a client
        client_socket, addr = server_socket.accept()
        # Receive the data
        data = client_socket.recv(1024).decode('utf-8')
        
        client_socket.close()
        s1 = data
        time.sleep(1)
        #dtemp = decode(s1)
        # data = "Thumb:10,20,30;Index:40,50,60;Middle:70,80,90;Ring:15,25,35;Pinky:45,55,65;Palm:100,110,120"
        thumb_pry, index_pry, middle_pry, ring_pry, pinky_pry, palm_pry = parse_pry_data(data)
        l=[thumb_pry, index_pry, middle_pry, ring_pry, pinky_pry, palm_pry]
        updatedList = []
        for i in l:
            for j in i:
                updatedList.append(j)
        with open('gyroSensorValues.csv', mode='a', newline='\n') as file:
            csv_writer = csv.writer(file)

            # Write a row of data (you can write multiple rows in a loop)
            csv_writer.writerow(updatedList)