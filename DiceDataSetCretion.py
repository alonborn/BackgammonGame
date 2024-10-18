import pandas as pd
import os
import cv2
import numpy as np
import json

# Configuration file to store the index of the first image
config_file = "config.json"

def update_excel_file(file_path, df, search_string, number1, number2):
    try:
        if search_string in df['Image'].values:
            df.loc[df['Image'] == search_string, ['Dice1', 'Dice2']] = number1, number2
        else:
            new_row = {'Image': search_string, 'Dice1': number1, 'Dice2': number2}
            df = pd.concat([df, pd.DataFrame([new_row])], ignore_index=True)

        df.to_excel(file_path, index=False)
        print(f"{search_string} - Excel file updated successfully.")
    except Exception as e:
        print(f"Error updating Excel file: {e}")

def display_image_info(image, existing_text, current_image_path, position, cell_size):
    y_offset = 20 + position[1] * cell_size
    x_offset = 10 + position[0] * cell_size
    cv2.putText(image, existing_text, (x_offset, y_offset + cell_size - 70), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
    cv2.putText(image, current_image_path, (x_offset, y_offset + cell_size - 50), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)

def save_config(current_index):
    with open(config_file, "w") as file:
        json.dump({"current_index": current_index}, file)

def load_config():
    if os.path.exists(config_file):
        with open(config_file, "r") as file:
            return json.load(file).get("current_index", 0)
    return 0

db_file = "C:\\temp\\dice_aug24.xlsx"
current_image_index = 0
images_per_row = 3  # Change this value to set the grid size (e.g., 4 for a 4x4 grid)
cell_size = 250
images_per_page = images_per_row * images_per_row
current_index = load_config()
image_files = []
images_buffer = {}
excel_data = None

def mouse_callback(event, x, y, flags, param):
    global current_image_index
    if event == cv2.EVENT_LBUTTONDOWN:
        col = x // cell_size
        row = y // cell_size
        index = row * images_per_row + col
        if index < images_per_page:
            current_image_index = index
            draw_grid(current_index)

def draw_grid(start_index):
    global current_image_index, images_per_page, image_files, images_buffer, excel_data, images_per_row, cell_size

    grid_image = np.zeros((cell_size * images_per_row, cell_size * images_per_row, 3), dtype=np.uint8)

    for i in range(images_per_page):
        idx = start_index + i
        if idx >= len(image_files):
            break

        image_name = image_files[idx]
        image = images_buffer[image_name]
        row = i // images_per_row
        col = i % images_per_row
        grid_image[row*cell_size:(row+1)*cell_size, col*cell_size:(col+1)*cell_size] = image

        if image_name in excel_data['Image'].values:
            existing_numbers = excel_data.loc[excel_data['Image'] == image_name, ['Dice1', 'Dice2']]
            existing_text = f"Ex: {existing_numbers.values.flatten()}"
            display_image_info(grid_image, existing_text, image_name, (col, row), cell_size)

        if i == current_image_index:
            cv2.rectangle(grid_image, (col * cell_size, row * cell_size), (col * cell_size + cell_size, row * cell_size + cell_size), (0, 255, 0), 2)

    cv2.imshow('Image Grid', grid_image)
    cv2.setMouseCallback('Image Grid', mouse_callback)

def browse_images_with_auto_update(folder_path, move_to=""):
    global current_image_index, images_per_page, current_index, image_files, images_buffer, excel_data, cell_size
    try:
        image_files = [f for f in os.listdir(folder_path) if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.bmp'))]

        if not image_files:
            print("No image files found in the folder.")
            return

        for image_name in image_files:
            image_path = os.path.join(folder_path, image_name)
            image = cv2.imread(image_path)
            image = cv2.resize(image, (cell_size, cell_size))
            images_buffer[image_name] = image

        if os.path.exists(db_file):
            excel_data = pd.read_excel(db_file)
        else:
            excel_data = pd.DataFrame(columns=['Image', 'Dice1', 'Dice2'])

        loop_started = False

        while True:
            if loop_started or move_to == "" or (image_files[current_index] == move_to):
                loop_started = True
                draw_grid(current_index)
                key = cv2.waitKey(0) & 0xFF

                if key == ord('p'):
                    if current_index + images_per_page < len(image_files):
                        current_index += images_per_page
                        current_image_index = 0
                        draw_grid(current_index)
                elif key == ord('o'):
                    if current_index - images_per_page >= 0:
                        current_index -= images_per_page
                        current_image_index = 0
                        draw_grid(current_index)
                elif key == ord('w'):
                    if current_image_index - images_per_row >= 0:
                        current_image_index -= images_per_row
                        draw_grid(current_index)
                elif key == ord('s'):
                    if current_image_index + images_per_row < images_per_page:
                        current_image_index += images_per_row
                        draw_grid(current_index)
                elif key == ord('a'):
                    if current_image_index > 0:
                        current_image_index -= 1
                        draw_grid(current_index)
                elif key == ord('d'):
                    if current_image_index < min(images_per_page, len(image_files) - current_index) - 1:
                        current_image_index += 1
                        draw_grid(current_index)
                elif chr(key).isdigit():
                    first_digit = int(chr(key))
                    second_digit = cv2.waitKey(0) & 0xFF

                    if chr(second_digit).isdigit():
                        digits_input = int(str(first_digit) + chr(second_digit))
                        update_excel_file(db_file, excel_data, image_files[current_index + current_image_index], digits_input // 10, digits_input % 10)
                        draw_grid(current_index)
                save_config(current_index)

        cv2.destroyAllWindows()

    except Exception as e:
        print(f"Error browsing images: {e}")

folder_path = 'C:\\dice\\'
browse_images_with_auto_update(folder_path, "")
