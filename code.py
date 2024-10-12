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

gp = Gamepad(usb_hid.devices)

# Use for I2C for display
i2c = busio.I2C(scl=board.GP9, sda=board.GP8)
as5600 = AS5600(i2c, AS5600_ID)

# Test values
as5600.scan()
as5600.CONF = 0x64
# print("should be 0x64", hex(as5600.CONF))
# print("Power mode", as5600.PM)
# Example usage: print the magnet status
print(as5600.magnet_status())

# Rest of the code
DEBUG = 0
# CENTER = 25
DEAD_ZONE = 3
ANGLE_USAGE_TH = 65535
ANGLE_USAGE_BK = 65535
ANGLE_USAGE_AS5600 = 4095
DEAD_ZONE_AS5600 = 512
CALIBRATE_ITER = 100

# Setup for Analog Joystick as X and Y
ADC_DIV = 8
AVG_READINGS = 10
throttle = analogio.AnalogIn(board.GP27)
brake = analogio.AnalogIn(board.GP26)
th_values = [2200, 3700]
brk_values = [2000, 4000]
steer_center = 128
last_steer = 0

prev_t = 0
dt = 0

def calibrate_pedals(adc_channel):
    values = [0,0]
    if input('Min: ') == 'y':
        buffer = 0
        for _ in range(CALIBRATE_ITER):
            buffer += adc_channel.value / ADC_DIV
        values[0] = buffer / CALIBRATE_ITER
    print(values)
    
    if input('Max: ') == 'y':
        buffer = 0
        for _ in range(CALIBRATE_ITER):
            buffer += adc_channel.value / ADC_DIV
        values[1] = buffer / CALIBRATE_ITER
    print(values)

    return values

# Equivalent of Arduino's map() function.
def range_map(x, in_min, in_max, out_min, out_max):
    if x <= in_min:
        return out_min

    if x >= in_max:
        return out_max

    return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min


def get_adc_value(adc_channel):
    return sum((adc_channel.value / ADC_DIV for _ in range(AVG_READINGS))) / AVG_READINGS

def get_acc_value():
    setx = int(range_map(get_adc_value(throttle), th_values[0], th_values[1], -127, 127))

    # Use a dead zone
    if abs(setx) <= DEAD_ZONE + 1:
        setx = 0

    return setx

def get_brk_value():
    sety = int(range_map(get_adc_value(brake), brk_values[0], brk_values[1], -127, 127))

    # Use a dead zone
    if abs(sety) <= DEAD_ZONE:
        sety = 0

    return sety

def get_steering_value():
    if as5600.MD and not as5600.ML and not as5600.MH:
        setz = int(range_map(
            as5600.ANGLE + steer_center,
            DEAD_ZONE_AS5600,
            ANGLE_USAGE_AS5600 - DEAD_ZONE_AS5600,
            out_min= -127,
            out_max= 127
        ))
        
        #if abs(setz) <= DEAD_ZONE:
        #    setz = 0

        return setz

if DEBUG:
    print('Acelerador')
    th_values = calibrate_pedals(throttle)
    print('Freno')
    brk_values = calibrate_pedals(brake)

while True:
    # Nanoseconds to seconds
    # dt = (monotonic_ns() / 1_000_000_000)  - prev_t
    # prev_t = monotonic_ns() / 1_000_000_000

    acc = get_acc_value()
    brk = get_brk_value()
    steer = get_steering_value()

    if steer and (steer * last_steer >= 0 or abs(steer) < 30):
        gp.move_joysticks(z=steer)   
        last_steer = steer

    if acc > -110 and brk < -110:
        brk = -127
    if brk > -110 and acc < -110:
        acc = -127
    gp.move_joysticks(
       x=acc,
       y=brk
    )

    if DEBUG:
        print('Volante: ', as5600.ANGLE)
        print(f'ACC: {get_adc_value(throttle)}\t{acc}')
        print(f'BRK: {get_adc_value(brake)}\t {brk}')
        sleep(0.5)
    else:
        sleep(0.01)
 