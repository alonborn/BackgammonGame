import serial
import time

arduino_port = "COM6"
ser = 0
"""
Move:G0 X1.34 Y0.36^
Magnet:2:64^
Move:G0 X1.34 Y0.30^
Magnet:1:87^
MagnetOn^
Magnet:1:64^
Move:G0 X1.55 Y0.89^
Magnet:1:0^
%Move:G0 X1.90 Y0.36%Magnet:1:64:X:1.55%^
Move:G0 X1.90 Y0.22^MagnetOff^
Magnet:2:0^
Move:G0 X0.00 Y0.00
"""
def write_data(data : str):
    global ser
    ser.write(data.encode())
    print(f"Data '{data}' written to {port} at {baudrate} baud.")

def write_to_serial(port, baudrate, data, delay=1):
    global ser
    try:
        # Open serial port
        ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # Wait for the serial connection to initialize

        # Write data to serial port
        write_data("%Home%")

        write_data("%Move:G0 X1.34 Y0.36^Magnet:2:64^Move:G0 X1.34 Y0.30^Magnet:1:87^MagnetOn^Magnet:1:64^Move:G0 X1.55 Y0.89^Magnet:1:0^%Move:G0 X1.90 Y0.36%Magnet:1:64:X:1.55%^Move:G0 X1.90 Y0.22^MagnetOff^Magnet:2:0^Move:G0 X0.00 Y0.00%")
        #write_data("%Magnet:2:64%")

        write_data("%Move:G0 X1.34 Y0.30%")
        write_data("%MagnetOn%")
        write_data("%Magnet:1:87%")
        write_data("%Magnet:1:64%")
        write_data("%Move:G0 X1.55 Y0.89%")
        write_data("%Magnet:1:0%")
        write_data("%Move:G0 X0 Y0%")
        # Wait for the specified delay
        time.sleep(delay)

        # Close the serial port
        ser.close()
        print(f"Closed connection to {port}.")

    except serial.SerialException as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    port = 'COM3'  # Change this to your serial port
    baudrate = 9600  # Change this to your desired baud rate
    data = 'Hello, Serial Port!'  # Change this to the data you want to send

    write_to_serial(arduino_port, 9600, data)
