import socket
import cv2
import mediapipe as mp
import csv
from typing import List, Tuple

mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils

def parse_pry_data(data: str) -> Tuple[List[float], ...]:
    finger_data = data.split(';')
    finger_pry = {
        "Thumb": [0.0, 0.0, 0.0],
        "Point": [0.0, 0.0, 0.0],
        "Middle": [0.0, 0.0, 0.0],
        "Ring": [0.0, 0.0, 0.0],
        "Pinky": [0.0, 0.0, 0.0],
        "Palm": [0.0, 0.0, 0.0]
    }

    for finger in finger_data:
        parts = finger.split(':')
        if len(parts) == 2:
            finger_name, values = parts
            values = [float(v) for v in values.split(',')]
            if len(values) == 3 and finger_name in finger_pry:
                finger_pry[finger_name] = values

    return tuple(finger_pry.values())

def process_frame(frame: cv2.Mat, hands: mp_hands.Hands) -> Tuple[cv2.Mat, List[str]]:
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(frame_rgb)
    frame_landmarks = []

    if result.multi_hand_landmarks:
        for hand_landmarks in result.multi_hand_landmarks:
            for idx, landmark in enumerate(hand_landmarks.landmark):
                landmark_data = f'Landmark_{idx}: ({landmark.x:.4f}, {landmark.y:.4f}, {landmark.z:.4f})'
                frame_landmarks.append(landmark_data)
            mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

    return frame, frame_landmarks

def write_to_csv(filename: str, data: List[float]):
    with open(filename, 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(data)
        file.flush()

def main():
    server_ip = '0.0.0.0'
    port = 6969
    output_csv = 'hand_landmarks.csv'
    landmarks_csv = 'frame_landmarks.csv'

    hands = mp_hands.Hands()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((server_ip, port))
    server_socket.listen(5)
    server_socket.settimeout(0.1)  # Set a timeout for accept()
    print(f"Server listening on {server_ip}:{port}")

    cap = cv2.VideoCapture(0)

    try:
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                print("Failed to capture frame. Exiting...")
                break

            frame, frame_landmarks = process_frame(frame, hands)

            try:
                client_socket, _ = server_socket.accept()
                with client_socket:
                    data = client_socket.recv(1024).decode('utf-8')
                    if data and len(data) > 10:
                        pry_data = parse_pry_data(data)
                        flat_data = [item for sublist in pry_data for item in sublist]
                        write_to_csv(output_csv, flat_data)
                        write_to_csv(landmarks_csv, frame_landmarks)
            except socket.timeout:
                pass  

            cv2.imshow('Hand Tracking', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    finally:
        cap.release()
        cv2.destroyAllWindows()
        server_socket.close()
        hands.close()

if __name__ == "__main__":
    main()