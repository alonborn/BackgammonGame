import win32com.client

def find_device_by_name(device_name):
    try:
        # Access Windows Shell
        shell = win32com.client.Dispatch("Shell.Application")
        
        # Get the namespace for 'My Computer' (This PC)
        my_computer = shell.Namespace(17)  # 17 is the CSIDL for My Computer

        # Search for the device by the given name
        for item in my_computer.Items():
            print (item)
            if item.Name == device_name:
                return True
        
        return False

    except Exception as e:
        print(f"Error: {e}")
        return False

# Define the device name in Hebrew
target_device_name = "ארז"

# Check if the device exists
if find_device_by_name(target_device_name):
    print(f"The device '{target_device_name}' is connected to the computer.")
else:
    print(f"The device '{target_device_name}' is not connected to the computer.")
