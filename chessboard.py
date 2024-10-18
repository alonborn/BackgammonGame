import cv2
import numpy as np

def create_chessboard_image(chessboard_size, square_size, image_size):
    """
    Creates an image of a chessboard.

    Parameters:
    chessboard_size (tuple): Number of squares along the width and height (width, height).
    square_size (int): Size of each square in pixels.
    image_size (tuple): Size of the output image (width, height).

    Returns:
    numpy.ndarray: Image of the chessboard.
    """
    chessboard_image = np.ones((image_size[1], image_size[0]), dtype=np.uint8) * 255

    for y in range(chessboard_size[1]):
        for x in range(chessboard_size[0]):
            if (x + y) % 2 == 0:
                cv2.rectangle(
                    chessboard_image,
                    (x * square_size, y * square_size),
                    ((x + 1) * square_size, (y + 1) * square_size),
                    0,
                    -1
                )

    return chessboard_image

def main():
    chessboard_size = (14, 14)  # Number of squares (width, height)
    square_size = 70  # Size of each square in pixels
    image_size = (chessboard_size[0] * square_size, chessboard_size[1] * square_size)  # Image size (width, height)

    chessboard_image = create_chessboard_image(chessboard_size, square_size, image_size)

    # Save the chessboard image
    cv2.imwrite('chessboard_14x14.png', chessboard_image)

    print(f"Chessboard image 'chessboard_14x14.png' saved.")

if __name__ == "__main__":
    main()
