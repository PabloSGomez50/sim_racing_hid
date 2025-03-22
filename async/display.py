import const
import busio, displayio
from adafruit_display_text import label
import terminalio
import adafruit_ili9341
import time
import vectorio

displayio.release_displays()

IMG_SIZE = 80
PEDAL_WIDTH = 30
PEDAL_HEIGHT = const.D_HEIGHT - const.D_BORDER * 3
willimas_bmp = displayio.OnDiskBitmap("/Logo_Williams_F1.bmp")

black_palette = displayio.Palette(1)
black_palette[0] = 0x000000  # Black
green_palette = displayio.Palette(1)
green_palette[0] = 0x00FF00
red_palette = displayio.Palette(1)
red_palette[0] = 0xFF0000

def init_display_obj():
    display_spi = busio.SPI(clock=const.D_CLK_PIN, MOSI=const.D_MOSI_PIN)

    display_bus = displayio.FourWire(display_spi,
        command=const.D_DC_PIN,
        chip_select=const.D_CS_PIN,
        reset=const.D_RST_PIN
    )

    display = adafruit_ili9341.ILI9341(display_bus,
        width=const.D_WIDTH,
        height=const.D_HEIGHT,
        rotation=const.D_ROTATION
    )
    return display

def init_display_background(display):

    splash = displayio.Group()
    display.show(splash)

    # Draw background
    color_bitmap = displayio.Bitmap(const.D_WIDTH, const.D_HEIGHT, 1)
    bg_color = displayio.Palette(1)
    bg_color[0] = 0x00A0DD  # Williams blue
    bg_sprite = displayio.TileGrid(color_bitmap, pixel_shader=bg_color, x=0, y=0)
    splash.append(bg_sprite)

    # Draw a smaller inner rectangle
    inner_bitmap = displayio.Bitmap(const.D_WIDTH - const.D_BORDER, const.D_HEIGHT - const.D_BORDER, 1)

    inner_sprite = displayio.TileGrid(
        inner_bitmap,
        pixel_shader=black_palette,
        x=const.D_BORDER // 2,
        y=const.D_BORDER // 2
    )
    splash.append(inner_sprite)

    williams_sprite = displayio.TileGrid(
        willimas_bmp,
        pixel_shader=willimas_bmp.pixel_shader,
        x= (const.D_WIDTH - IMG_SIZE) // 2,
        y= const.D_BORDER
    )
    splash.append(williams_sprite)

    return splash

def init_display_pedals(display):
    pedal_group = displayio.Group(scale=1, x=const.D_BORDER * 3 // 2, y=const.D_BORDER)
    display.append(pedal_group)
    pedal_group.append(vectorio.Rectangle(
        width=PEDAL_WIDTH,
        height=PEDAL_HEIGHT,
        pixel_shader=red_palette,
        x=0,
        y=const.D_BORDER // 2
    ))
    pedal_group.append(vectorio.Rectangle(
        width=PEDAL_WIDTH,
        height=PEDAL_HEIGHT,
        pixel_shader=green_palette,
        x=PEDAL_WIDTH+4,
        y=const.D_BORDER // 2
    ))
    draw_pedals(pedal_group, -127, -127)
    return pedal_group

def init_display_text(splash):
    text_x = 2 * const.D_BORDER + 2 * PEDAL_WIDTH + 8
    text_y = IMG_SIZE + const.D_BORDER * 3 // 2
    
    text_group = displayio.Group(scale=1, x=text_x, y=text_y)
    splash.append(text_group)

    return text_group

def draw_text(
    group, 
    content,
    line: int = 0,
    x: int = 0,
    y: int = 0
):
    text_group = displayio.Group(scale=1, x=x, y=y + 12 * line)
    text_area = label.Label(terminalio.FONT, text=content, color=0xFFFFFF, scale = 1)
    text_group.append(text_area)
    if len(group) > line:
        group.pop(line)
        group.insert(line, text_group)
    else:
        group.append(text_group)
    
    return group.index(text_group)

def draw_pedals(group, brk: int, thr: int):
    MARGIN = 4
    brk_height = (PEDAL_HEIGHT - MARGIN) * (1 - (brk + 127) / 255)
    if brk_height > 1:
        rect = vectorio.Rectangle(
            width=PEDAL_WIDTH - MARGIN,
            height=int(brk_height),
            pixel_shader=black_palette,
            x=MARGIN // 2,
            y=const.D_BORDER // 2 + MARGIN // 2
        )
        if len(group) >= 3:
            group.pop(2)
            group.insert(2, rect)
        else:
            group.append(rect)
        
        
    thr_height = (PEDAL_HEIGHT - MARGIN) * (1 - (thr + 127) / 255)
    if thr_height > 1:
        rect = vectorio.Rectangle(
            width=PEDAL_WIDTH - MARGIN,
            height=int(thr_height),
            pixel_shader=black_palette,
            x=PEDAL_WIDTH + MARGIN // 2 + 4,
            y=const.D_BORDER // 2 + MARGIN // 2
        )
        if len(group) >= 4:
            group.pop(3)
            group.insert(3, rect)
        else:
            group.append(rect)
        

def main():
    display_obj = init_display_obj()
    splash = init_display_background(display_obj)
    pedal_group = init_display_pedals(display_obj)
    text_group = init_display_text(splash)


    draw_text(text_group, "Williams Racing Sim")
    draw_text(text_group, "F1 2020", line=1)
    time.sleep(0.5)
    draw_text(text_group, "Lap 1: 01:34:15\n", line=2)
    time.sleep(1)
    draw_text(text_group, "Lap 2: 01:04:24\n", line=2)
    
    for i in range(-127, 128):
        print("Value:", i)
        draw_pedals(pedal_group, i, -i)
        #time.sleep(0.01)

if __name__ == '__main__':
    main()