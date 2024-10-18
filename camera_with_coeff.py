import cv2
import numpy as np

def undistort_live_feed(camera_matrix, distortion_coefficients):
    cap = cv2.VideoCapture(0)  # Capture from the default camera
    if not cap.isOpened():
        print("Error: Could not open video capture")
        return

    zoom_factor = 1.0
    zoom_step = 0.1

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame")
            break

        h, w = frame.shape[:2]
        
        # New camera matrix for undistortion and rectification
        new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(camera_matrix, distortion_coefficients, (w, h), 1, (w, h))

        # Undistort
        undistorted_frame = cv2.undistort(frame, camera_matrix, distortion_coefficients, None, new_camera_matrix)

        # Apply zoom
        center_x, center_y = w // 2, h // 2
        new_w, new_h = int(w / zoom_factor), int(h / zoom_factor)
        top_left_x, top_left_y = center_x - new_w // 2, center_y - new_h // 2
        zoomed_frame = undistorted_frame[top_left_y:top_left_y + new_h, top_left_x:top_left_x + new_w]
        zoomed_frame = cv2.resize(zoomed_frame, (w, h))

        cv2.imshow('Undistorted Live Feed with Zoom', zoomed_frame)

        key = cv2.waitKey(1) & 0xFF

        if key == ord('q'):
            break
        elif key == ord('i'):  # Zoom in
            zoom_factor = max(1.0, zoom_factor - zoom_step)
        elif key == ord('o'):  # Zoom out
            zoom_factor += zoom_step

    cap.release()
    cv2.destroyAllWindows()

# Load the calibration results
camera_matrix = np.load('camera_matrix.npy')
distortion_coefficients = np.load('distortion_coefficients.npy')

# Run the undistortion with live video feed
undistort_live_feed(camera_matrix, distortion_coefficients)
