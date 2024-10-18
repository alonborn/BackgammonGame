import cv2
import numpy as np
import glob
import os

def calibrate_camera(chessboard_images, chessboard_size, resize_factor=0.25):
    # Prepare object points
    objp = np.zeros((chessboard_size[0] * chessboard_size[1], 3), np.float32)
    objp[:, :2] = np.mgrid[0:chessboard_size[0], 0:chessboard_size[1]].T.reshape(-1, 2)

    objpoints = []  # 3d point in real world space
    imgpoints = []  # 2d points in image plane.

    for fname in chessboard_images:
        img = cv2.imread(fname)
        if img is None:
            print(f"Warning: Could not read image {fname}")
            continue

        # Resize the image
        img = cv2.resize(img, (0, 0), fx=resize_factor, fy=resize_factor)

        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

        # Preprocessing step: adaptive thresholding
        #gray = cv2.adaptiveThreshold(gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, 11, 2)

        ret, corners = cv2.findChessboardCorners(gray, chessboard_size, None)

        if ret:
            objpoints.append(objp)
            corners2 = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
            imgpoints.append(corners2)

            # Draw and display the corners
            cv2.drawChessboardCorners(img, chessboard_size, corners2, ret)
            cv2.imshow('Chessboard corners', img)
            cv2.waitKey(500)
        else:
            cv2.imshow('Chessboard corners', gray)
            cv2.waitKey(500)
            print(f"Warning: Chessboard corners not found in image {fname}")

    cv2.destroyAllWindows()

    if len(objpoints) == 0 or len(imgpoints) == 0:
        raise ValueError("Error: No valid chessboard images found for calibration.")

    ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)
    return mtx, dist

criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
chessboard_size = (13, 13)  # Number of inner corners per chessboard row and column

# List of chessboard images
chessboard_images = glob.glob(os.path.join('chessboard_images', '*.jpg'))

# Debugging: Print the list of found images
print(f"Found {len(chessboard_images)} images for calibration: {chessboard_images}")

# Calibrate camera
camera_matrix, distortion_coefficients = calibrate_camera(chessboard_images, chessboard_size)

# Save the calibration results
np.save('camera_matrix.npy', camera_matrix)
np.save('distortion_coefficients.npy', distortion_coefficients)

print("Camera calibrated successfully.")
print("Camera matrix:\n", camera_matrix)
print("Distortion coefficients:\n", distortion_coefficients)
