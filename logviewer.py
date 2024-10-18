import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk, ImageDraw, ImageFont
import os

# Function to read log file content
def read_log_file(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as file:
            return file.read()
    else:
        print(f"Log file {file_path} not found.")
        return ""

# Parse log file content
def parse_logs(log_content):
    logs = log_content.strip().split("\n")
    entries = []
    entry = {}
    for log in logs:
        if "GenBoard:" in log:
            entry['GenBoard'] = log.split("GenBoard:")[1].strip()
        elif "Board:" in log:
            if entry:
                entries.append(entry)
                entry = {}
            entry['Board'] = log.split("Board:")[1].strip()
        else:
            key, value = log.split(" - INFO - ")[1].split(":", 1)
            entry[key] = value.strip()
    if entry:
        entries.append(entry)
    return entries

# Tkinter setup
class LogViewer(tk.Tk):
    def __init__(self, entries):
        super().__init__()
        self.entries = entries
        self.images_visible = True

        # Start from the last 4 images if available
        self.index = max(0, len(entries) - 2)

        self.title("Log Viewer")
        self.geometry("1700x950")

        # Configure grid layout
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(0, weight=3)  # Text frame now takes 30% of height
        self.grid_rowconfigure(1, weight=7)  # Image frame now takes 70% of height

        # Text box for log information
        self.info_text = tk.Text(self, wrap=tk.WORD, height=9)
        self.info_text.grid(row=0, column=0, sticky='nsew', padx=10, pady=10)

        # Frame for image labels
        self.image_frame = tk.Frame(self)
        self.image_frame.grid(row=1, column=0, sticky='nsew', padx=10, pady=0)

        self.image_frame.grid_columnconfigure(0, weight=1)
        self.image_frame.grid_columnconfigure(1, weight=1)
        self.image_frame.grid_rowconfigure(0, weight=1)  # Allow the image row to expand

        # Image labels
        self.img_label_1 = ttk.Label(self.image_frame)
        self.img_label_1.grid(row=0, column=0, padx=5, pady=5, sticky='nsew')

        self.img_label_2 = ttk.Label(self.image_frame)
        self.img_label_2.grid(row=0, column=1, padx=5, pady=5, sticky='nsew')

        self.img_label_3 = ttk.Label(self.image_frame)
        self.img_label_3.grid(row=1, column=0, padx=5, pady=5, sticky='nsew')

        self.img_label_4 = ttk.Label(self.image_frame)
        self.img_label_4.grid(row=1, column=1, padx=5, pady=5, sticky='nsew')

        # Bind keys for navigation
        self.bind("<s>", self.show_next)
        self.bind("<a>", self.show_previous)
        self.bind("<h>", self.toggle_images)

        # Bind resize event
        self.bind("<Configure>", self.on_resize)

        self.show_images()

    def show_images(self):
        self.clear_images()
        if self.index < len(self.entries):
            entry = self.entries[self.index]
            if 'Board' in entry:
                self.show_image(entry['Board'], self.img_label_1)
            if 'GenBoard' in entry:
                self.show_image(entry['GenBoard'], self.img_label_3)
        if self.index + 1 < len(self.entries):
            entry = self.entries[self.index + 1]
            if 'Board' in entry:
                self.show_image(entry['Board'], self.img_label_2)
            if 'GenBoard' in entry:
                self.show_image(entry['GenBoard'], self.img_label_4)
        self.show_info()

    def clear_images(self):
        self.img_label_1.config(image='')
        self.img_label_2.config(image='')
        self.img_label_3.config(image='')
        self.img_label_4.config(image='')

    def show_image(self, img_path, label):
        if img_path:
            # Adjust the image size based on the visibility of the second row
            if self.images_visible:
                new_height = self.image_frame.winfo_height() // 2 - 10
            else:
                new_height = self.image_frame.winfo_height() - 10
            new_width = self.image_frame.winfo_width() // 2 - 10

            # Check if the dimensions are valid
            if new_width > 0 and new_height > 0:
                image = Image.open(img_path)
                image = image.resize((new_width, new_height), Image.Resampling.LANCZOS)
                draw = ImageDraw.Draw(image)

                # Define font size and font
                font_size = int(new_height * 0.03)
                try:
                    font = ImageFont.truetype("arialbd.ttf", font_size)
                except IOError:
                    font = ImageFont.load_default()

                # Draw the numbers 12-1 at the top
                offset = new_width / 2.5
                for i in range(12):
                    x_position = offset + (i + (0.5 if i > 5 else 0)) * (new_width // 22)
                    draw.text((x_position, 35), str(12 - i), fill="black", font=font)

                # Draw the numbers 13-24 at the bottom
                for i in range(12):
                    x_position = offset + (i + (0.5 if i > 5 else 0)) * (new_width // 23)
                    draw.text((x_position, new_height - font_size - 10), str(i + 13), fill="black", font=font)

                photo = ImageTk.PhotoImage(image)
                label.config(image=photo)
                label.image = photo

    def show_info(self):
        info = ""
        for i in range(self.index, self.index + 2):
            if i < len(self.entries):
                entry = self.entries[i]
                for key, value in entry.items():
                    if key not in ['Board', 'GenBoard']:
                        info += f"{key}: {value}\n"
                info += "\n"
        self.info_text.delete("1.0", tk.END)
        self.info_text.insert(tk.END, info)

    def show_next(self, event):
        if self.index < len(self.entries) - 1:
            self.index += 1
            self.show_images()

    def show_previous(self, event):
        if self.index > 0:
            self.index -= 1
            self.show_images()

    def on_resize(self, event):
        self.show_images()  # Reshow images to adjust their size

    def toggle_images(self, event):
        if self.images_visible:
            self.img_label_3.grid_remove()
            self.img_label_4.grid_remove()
        else:
            self.img_label_3.grid()
            self.img_label_4.grid()
        self.images_visible = not self.images_visible
        self.grid_rowconfigure(1, weight=7 if self.images_visible else 10)  # Adjust the grid row configuration
        self.show_images()  # Adjust image sizes when toggling visibility

# Read and parse the log file
log_file_path = "C:\\temp\\bg_log.log"
log_content = read_log_file(log_file_path)
entries = parse_logs(log_content)

# Run the application
if __name__ == "__main__":
    app = LogViewer(entries)
    app.mainloop()
