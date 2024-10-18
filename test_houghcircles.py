import cv2
import numpy as np

# Callback function for trackbars
def nothing(x):
    pass

# Create a window
cv2.namedWindow('Hough Circles')

# Create trackbars for param1 and param2 with default values
cv2.createTrackbar('param1', 'Hough Circles', 200, 300, nothing)
cv2.createTrackbar('param2', 'Hough Circles', 18, 100, nothing)

# Function to process and display circles
def process_and_display(image):
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    gray_blurred = cv2.GaussianBlur(gray, (9, 9), 2)
    cv2.imshow("gray",gray)
    cv2.imshow("gray_blurred",gray_blurred)

    param1 = cv2.getTrackbarPos('param1', 'Hough Circles')
    param2 = cv2.getTrackbarPos('param2', 'Hough Circles')

    circles = cv2.HoughCircles(gray, cv2.HOUGH_GRADIENT, dp=1, minDist=20,
                               param1=param1, param2=param2, minRadius=20, maxRadius=27)

    output = image.copy()

    if circles is not None:
        circles = np.uint16(np.around(circles))
        for i in circles[0, :]:
            cv2.circle(output, (i[0], i[1]), i[2], (0, 255, 0), 2)
            cv2.circle(output, (i[0], i[1]), 2, (0, 0, 255), 3)

    cv2.imshow('Hough Circles', output)

# Function to load and process image from file
def process_image_file():
    image_path = r'C:\temp\snapshot.jpg'
    image = cv2.imread(image_path, cv2.IMREAD_COLOR)

    if image is None:
        print(f"Error: Could not open or find the image '{image_path}'.")
        return

    process_and_display(image)

    while True:
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# Capture video from the default camera
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Error: Could not open camera.")
    exit()

use_camera = True

while True:
    if use_camera:
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            break
        process_and_display(frame)
    else:
        process_image_file()
        use_camera = True  # Switch back to camera after processing the image file

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('i'):
        use_camera = False

cap.release()
cv2.destroyAllWindows()
