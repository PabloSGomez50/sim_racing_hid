#import board support libraries, including HID.
import board
import digitalio
import analogio
import usb_hid
import busio
import asyncio
import keypad

from time import sleep

#library for communicating as a gamepad
from hid_gamepad import Gamepad
from as5600 import AS5600, AS5600_ID
import const
import display
from utils import get_pedal_value, get_steering_value, get_adc_value, \
    wait_to_release_and_press

# Neopixel lib
from rainbowio import colorwheel
from neopixel import NeoPixel

gp = Gamepad(usb_hid.devices)

# Use for I2C for display
i2c = busio.I2C(scl=const.I2C_SCL, sda=const.I2C_SDA)
as5600 = AS5600(i2c, AS5600_ID)
as5600.scan()
# Example usage: print the magnet status
print(as5600.magnet_status())
as5600_dir = digitalio.DigitalInOut(const.AS5600_DIR)
as5600_dir.direction = digitalio.Direction.OUTPUT
as5600_dir.value = True

class MultiData:
    def __init__(self):
        self.leds = NeoPixel(const.PIXEL_PIN, const.NUM_LEDS, brightness=0.3, auto_write=False)

        self.thr = 0
        self.thr_ch = analogio.AnalogIn(const.THROTTLE_PIN)
        self.thr_values = [2600, 3700]
        self.thr_offset = 1

        self.brk = 0
        self.brk_ch = analogio.AnalogIn(const.BRAKE_PIN)
        self.brk_values = [2000, 3700]
        self.brk_offset = 0

        self.steer = 0
        self.last_steer = 0
        self.as5600center = const.STEER_CENTER


async def read_main_data(data: MultiData):
    while True:
        data.thr = get_pedal_value(data.thr_ch, data.thr_values, data.thr_offset)
        data.brk = get_pedal_value(data.brk_ch, data.brk_values, data.brk_offset)
        data.steer = get_steering_value(as5600, data.as5600center)

        if data.steer and (data.steer * data.last_steer >= 0 or abs(data.steer) < 30):
            print("Move steer: ", data.steer)
            gp.move_joysticks(z=data.steer)  
        else:
            print(f"MD {as5600.MD}, ML {as5600.ML}, MH {as5600.MH}") 
        data.last_steer = data.steer
            
        # print(data.steer, data.last_steer)

        if data.thr > const.PEDAL_MIN and data.brk < const.PEDAL_MIN:
            data.brk = -127
        if data.thr < const.PEDAL_MIN and data.brk > const.PEDAL_MIN:
            data.thr = -127
        gp.move_joysticks(
            x=data.thr,
            y=data.brk
        )

        if const.DEBUG:
            print('Volante: ', as5600.ANGLE)
            print(f'ACC: {get_adc_value(data.thr_ch)}\t{data.thr}')
            print(f'BRK: {get_adc_value(data.brk_ch)}\t {data.brk}')
            await asyncio.sleep(0.5)
        else:
            await asyncio.sleep(0.005)

async def read_btns(data: MultiData):
    with keypad.Keys(
        const.BTNS, value_when_pressed=False, pull=True
    ) as keys:
        while True:
            key_event = keys.events.get()
            if key_event:
                print('Keys:', key_event, key_event.key_number, key_event.pressed, key_event.released)
            if key_event and key_event.pressed:
            
                if key_event.key_number == 0:
                    print('Min th')
                    data.thr_values[0] = get_adc_value(data.thr_ch, const.CALIBRATE_ITER) + 100
                    wait_to_release_and_press(keys, btn=0)
                    print('Max th')
                    data.thr_values[1] = get_adc_value(data.thr_ch, const.CALIBRATE_ITER)

                    wait_to_release_and_press(keys, btn=0)
                    print('Min brk')
                    data.brk_values[0] = get_adc_value(data.brk_ch, const.CALIBRATE_ITER) + 100
                    wait_to_release_and_press(keys, btn=0)
                    print('Max brk')
                    data.brk_values[1] = get_adc_value(data.brk_ch, const.CALIBRATE_ITER)
                
            # Let another task run.
            await asyncio.sleep(0.001)

async def show_content(data: MultiData):
    display_obj = display.init_display_obj()
    splash = display.init_display_background(display_obj)
    pedal_group = display.init_display_pedals(splash)

    text_group = display.init_display_text(splash)
    display.draw_text(text_group, "Williams Racing Sim")
    display.draw_text(text_group, "F1 2020", line=1)

    while True:
        display.draw_pedals(pedal_group, data.brk, data.thr)
        display.draw_text(text_group, f"El acelerador esta en {data.thr + 127}", line=2)
        await asyncio.sleep(0.2)


async def main():
    data = MultiData()
    
    main_task = asyncio.create_task(read_main_data(data))
    #btn_task = asyncio.create_task(read_btns(data))
    #diplay_task = asyncio.create_task(show_content(data))
    # This will run forever, because neither task ever exits.
    await asyncio.gather(
        main_task,
        # btn_task,
        #diplay_task
    )

asyncio.run(main())