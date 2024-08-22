import usb_hid

# Corrected Gamepad Descriptor
GAMEPAD_REPORT_DESCRIPTOR = bytes((
    0x05, 0x01,  # Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,  # Usage (Game Pad)
    0xA1, 0x01,  # Collection (Application)
    0x85, 0x04,  #   Report ID (4)
    # Buttons (16 buttons)
    0x05, 0x09,  #   Usage Page (Button)
    0x19, 0x01,  #   Usage Minimum (Button 1)
    0x29, 0x10,  #   Usage Maximum (Button 16)
    0x15, 0x00,  #   Logical Minimum (0)
    0x25, 0x01,  #   Logical Maximum (1)
    0x75, 0x01,  #   Report Size (1)
    0x95, 0x10,  #   Report Count (16)
    0x81, 0x02,  #   Input (Data,Var,Abs)
    # Axes (X, Y, Z)
    0x05, 0x01,  #   Usage Page (Generic Desktop Ctrls)
    0x15, 0x81,  #   Logical Minimum (-127)
    0x25, 0x7F,  #   Logical Maximum (127)
    0x09, 0x30,  #   Usage (X)
    0x09, 0x31,  #   Usage (Y)
    0x09, 0x32,  #   Usage (Z)
    0x75, 0x08,  #   Report Size (8)
    0x95, 0x03,  #   Report Count (3) 
    0x81, 0x02,  #   Input (Data,Var,Abs)
    
    # Hat Switch
    0x09, 0x39,  #   Usage (Hat switch)
    0x15, 0x00,  #   Logical Minimum (0)
    0x25, 0x07,  #   Logical Maximum (7)
    0x35, 0x00,  #   Physical Minimum (0)
    0x46, 0x3B, 0x01,  #   Physical Maximum (315)
    0x65, 0x14,  #   Unit (Eng Rot:Angular Pos)
    0x75, 0x04,  #   Report Size (4)
    0x95, 0x01,  #   Report Count (1)
    0x81, 0x42,  #   Input (Data,Var,Abs,Null)
    
    0x75, 0x04,  #   Report Size (4)
    0x95, 0x01,  #   Report Count (1)
    0x81, 0x03,  #   Input (Cnst,Var,Abs)

    0xC0,        # End Collection
))


gamepad = usb_hid.Device(
    report_descriptor=GAMEPAD_REPORT_DESCRIPTOR,
    usage_page=0x01,           # Generic Desktop Control
    usage=0x05,                # Gamepad
    report_ids=(4,),           # Descriptor uses report ID 4.
    in_report_lengths=(6,),    # This gamepad sends 6 bytes in its report.
    out_report_lengths=(0,),   # It does not receive any reports.
)

usb_hid.enable(
    (usb_hid.Device.KEYBOARD,
     usb_hid.Device.MOUSE,
     usb_hid.Device.CONSUMER_CONTROL,
     gamepad)
)