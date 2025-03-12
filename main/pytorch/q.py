import cv2
import mediapipe as mp
import csv

# Initialize Mediapipe hand detection
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
hands = mp_hands.Hands()
frame_number = 10
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_POS_FRAMES, frame_number)
fps = int(cap.get(5))
print(fps)
output_csv = 'hand_landmarks.csv'

# Open CSV file
with open(output_csv, 'w', newline='') as file:
    writer = csv.writer(file)
    # Write header
    writer.writerow(['Frame', 'Landmark_Data'])  

    frame_number = 1

    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            break

        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        result = hands.process(frame_rgb)

        frame_landmarks = []

        # If hand landmarks are detected
        if result.multi_hand_landmarks:
            for hand_landmarks in result.multi_hand_landmarks:
                for idx, landmark in enumerate(hand_landmarks.landmark):
                    # Format the landmark data
                    landmark_data = f'Landmark_{idx}: ({landmark.x}, {landmark.y}, {landmark.z})'
                    frame_landmarks.append(landmark_data)

                # Draw landmarks on the frame
                mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

        # Join all the landmarks as a single string and write to file
        writer.writerow([frame_number, ' '.join(frame_landmarks)])

        # Show the frame with hand landmarks
        cv2.imshow('MediaPipe Hands', frame)

        frame_number += 1

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# Release video capture and destroy windows
cap.release()
cv2.destroyAllWindows()
