#import board support libraries, including HID.
import board
import digitalio
import analogio
import usb_hid
import busio

from time import sleep, monotonic_ns

#library for communicating as a gamepad
from hid_gamepad import Gamepad
from as5600 import AS5600, AS5600_ID

print(dir(board))

gp = Gamepad(usb_hid.devices)

# Use for I2C for display
i2c = busio.I2C(scl=board.GP9, sda=board.GP8)
as5600 = AS5600(i2c, AS5600_ID)

# Test values
as5600.scan()
as5600.CONF = 0x64
print("should be 0x64", hex(as5600.CONF))
print("Power mode", as5600.PM)
# Example usage: print the magnet status
print(as5600.magnet_status())

# Rest of the code
# CENTER = 25
DEAD_ZONE = 0
ANGLE_USAGE_TH = 65535
ANGLE_USAGE_BK = 65535
ANGLE_USAGE_AS5600 = 4095

# Setup for Analog Joystick as X and Y
throttle = analogio.AnalogIn(board.GP26)
brake = analogio.AnalogIn(board.GP27)
setz = 0

prev_t = 0
dt = 0

# Equivalent of Arduino's map() function.
def range_map(x, in_min, in_max, out_min, out_max):
    if x <= in_min:
        return out_min

    if x >= in_max:
        return out_max

    return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min


while True:
    # Nanoseconds to seconds
    # dt = (monotonic_ns() / 1_000_000_000)  - prev_t
    # prev_t = monotonic_ns() / 1_000_000_000

    setx = range_map(throttle.value, 0, ANGLE_USAGE_TH, -127, 127)
    sety = range_map(brake.value, 0, ANGLE_USAGE_BK, -127, 127)

    if as5600.MD and not as5600.ML and not as5600.MH:
        #print(as5600.ANGLE)
        last_setz = setz
        setz = range_map(as5600.ANGLE, 0, ANGLE_USAGE_AS5600, -127, 127)
        
        if abs(setz) <= DEAD_ZONE:
            setz = 0
        if last_setz * setz >= 0:
            gp.move_joysticks(z=int(setz))
    
    # Use a dead zone
    if abs(setx) <= DEAD_ZONE + 1:
        setx = 0
    if abs(sety) <= DEAD_ZONE:
        sety = 0
        
    #gp.move_joysticks(
    #    x=int(setx),
    #    y=int(sety)
    #)
 
    sleep(0.01)
    #sleep(0.5)