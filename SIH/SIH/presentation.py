import socket
import time
import pyautogui
def scroll_slow_down(Accel):
    #Accel = 1.5
    tdelay = Accel*0.5
    sdisp = int(-1 * (Accel*15))
    for i in range(0,20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def scroll_slow_up(Accel):
    #Accel = 1.5
    tdelay = Accel*0.5
    sdisp = int(Accel*15)
    for i in range(0,20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def scroll_fast_down(Accel):
    #Accel = 1.5
    tdelay = Accel*0.025
    sdisp = int(-1 * (Accel*25))
    for i in range(0,20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def scroll_fast_up(Accel):
    #Accel = 1.5
    tdelay = Accel*0.025
    sdisp = int(Accel*25)
    for i in range(0,20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

# Independent functions for movement
def move_left(Accel):
    freq = int(Accel*3)
    tdelay = 1/40
    for i in range(0,freq):
        pyautogui.press('left')
        time.sleep(tdelay)

def move_right(Accel):
    freq = int(Accel*3)
    tdelay = 1/40
    for i in range(0,freq):
        pyautogui.press('right')
        time.sleep(tdelay)

# Independent functions for zooming
def zoom_in(Accel):
    #Accel = 1.5
    pyautogui.hotkey('ctrl', '+')  # Zoom in
    pyautogui.hotkey('ctrl', '+')  # Zoom in

def zoom_out(Accel):
    #Accel = 1.5
    pyautogui.hotkey('ctrl', '-')  # Zoom in
    pyautogui.hotkey('ctrl', '-')  # Zoom in

def open_file(file_position):
    pyautogui.doubleClick(file_position)  # Double-click on the file at the given position
    time.sleep(2)

def close_file():
    pyautogui.hotkey('ctrl', 'w')  # Close the file (works in many applications)
    time.sleep(2)  # Wait for the file to close

file_position = (607,368)  # (x, y) coordinates of the file

def DetectGesture(thumb_angle, middle_angle, index_angle):
    if abs(middle_angle + index_angle) > 140:
        print("zoom in")
        zoom_in(max(middle_angle, index_angle))
    
    elif abs(index_angle + thumb_angle) > 100:
        print("zoom out")
        zoom_out(max(index_angle, thumb_angle))
    elif abs(thumb_angle) > 40:
        print("move right")
        # move_right(thumb_angle)
        for i in range(1):
            pyautogui.press('right')

    elif abs(index_angle) > 40:
        print("move left")
        # move_left(index_angle)
        for i in range(1):
            pyautogui.press('left')
            time.sleep(0.1)

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
    port = 6996

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_socket.bind((server_ip, port))

    server_socket.listen(5)
    print("Server listening on {}:{}".format(server_ip, port))

    flag = False
    count = 0
    decoded = []
    d1 = {"Thumb":[] ,"Middle": [], "Point":[],"Ring":[],"Main":[],"Index":[],}
    # time.sleep(1)
    while(True):
        
            
        # Establish a connection with a client
        client_socket, addr = server_socket.accept()
        # Receive the data
        data = client_socket.recv(1024).decode('utf-8')
        client_socket.close()
        s1 = data
        #dtemp = decode(s1)
        # data = "Thumb:10,20,30;Index:40,50,60;Middle:70,80,90;Ring:15,25,35;Pinky:45,55,65;Palm:100,110,120"
        thumb_pry, index_pry, middle_pry, ring_pry, pinky_pry, palm_pry = parse_pry_data(data)
        
        d1["Thumb"] = thumb_pry
        d1["Middle"] = middle_pry
        d1["Point"] = index_pry
        d1["Index"]= index_pry
        d1["Ring"] = ring_pry
        d1["Main"] = palm_pry
        print(d1)
        DetectGesture(d1["Thumb"][0],d1["Middle"][0],d1["Point"][0])