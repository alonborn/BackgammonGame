import cv2
import numpy as np

def undistort(img, k1, k2, k3):
    h, w = img.shape[:2]
    K = np.array([[w, 0, w // 2], [0, h, h // 2], [0, 0, 1]], dtype=np.float32)
    D = np.array([k1, k2, 0, k3], dtype=np.float32)
    map1, map2 = cv2.initUndistortRectifyMap(K, D, None, K, (w, h), 5)
    undistorted_img = cv2.remap(img, map1, map2, interpolation=cv2.INTER_LINEAR)
    return undistorted_img

def main():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("Error: Could not open video stream.")
        return

    # Set the resolution to HD (1280x720)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

    k1, k2, k3 = -0.2, 0.0, 0.0
    rect_x, rect_y, rect_w, rect_h = 100, 100, 200, 150

    print("Adjust k1, k2, and k3 using 'i', 'j', 'k', 'l', 'o', 'p' keys.")
    print("Adjust rectangle position using 'w', 'a', 's', 'd' keys.")
    print("Adjust rectangle size using 't', 'g', 'f', 'h' keys. Press 'q' to quit.")

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            break

        undistorted_frame = undistort(frame, k1, k2, k3)

        # Draw the adjustable rectangle
        cv2.rectangle(undistorted_frame, (rect_x, rect_y), (rect_x + rect_w, rect_y + rect_h), (0, 255, 0), 2)
        cv2.putText(undistorted_frame, f'k1: {k1:.2f} k2: {k2:.2f} k3: {k3:.2f}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        cv2.imshow('Undistorted Frame', undistorted_frame)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('i'):
            k1 += 0.01
        elif key == ord('j'):
            k1 -= 0.01
        elif key == ord('k'):
            k2 += 0.01
        elif key == ord('l'):
            k2 -= 0.01
        elif key == ord('o'):
            k3 += 0.01
        elif key == ord('p'):
            k3 -= 0.01
        elif key == ord('t'):
            rect_w += 10
        elif key == ord('g'):
            rect_w -= 10
        elif key == ord('f'):
            rect_h += 10
        elif key == ord('h'):
            rect_h -= 10
        elif key == ord('w'):  # Move rectangle up
            rect_y -= 10
        elif key == ord('s'):  # Move rectangle down
            rect_y += 10
        elif key == ord('a'):  # Move rectangle left
            rect_x -= 10
        elif key == ord('d'):  # Move rectangle right
            rect_x += 10

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
