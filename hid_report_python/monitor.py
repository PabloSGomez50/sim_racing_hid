import hid

# Replace with your HID device's VID and PID
DEVICE_VID = 0xCAFE  # Example Vendor ID
DEVICE_PID = 0x4004  # Example Product ID

def read_hid_reports(vid, pid):
    try:
        # Open the HID device
        device = hid.Device(vid, pid)
        print(f"Connected to device: {device.manufacturer} {device.product}")

        # Set non-blocking mode
        device.set_nonblocking(True)

        print("Listening for HID reports...")
        while True:
            # Read data from the device
            data = device.read(64)  # Adjust report size as needed
            if data:
                print(f"Received report: {data}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Ensure the device is closed
        try:
            device.close()
        except:
            pass

if __name__ == "__main__":
    read_hid_reports(DEVICE_VID, DEVICE_PID)
