import const
from time import sleep, monotonic_ns

def range_map(x, in_min, in_max, out_min, out_max):
    if x <= in_min:
        return out_min

    if x >= in_max:
        return out_max

    return (x - in_min) * (out_max - out_min) // (in_max - in_min) + out_min

def get_adc_value(adc_channel, iters: int = const.AVG_READINGS):
    return sum((adc_channel.value / const.ADC_DIV 
            for _ in range(iters))) / iters


def calibrate_pedals(adc_channel, btn):
    last_t = monotonic_ns()
    t = monotonic_ns()
    while btn.value == 1:
        sleep(0.05)
        t = monotonic_ns()
        if (t - last_t) > 1000:
            print('Waiting for Min value')
            last_t = t
    
    min_value = get_adc_value(adc_channel, const.CALIBRATE_ITER)

    while btn.value == 0:
        sleep(0.05)
        t = monotonic_ns()
        if (t - last_t) > 1000:
            print('Waiting To release btn')
            last_t = t

    while btn.value == 1:
        sleep(0.05)
        t = monotonic_ns()
        if (t - last_t) > 1000:
            print('Waiting for Max value')
            last_t = t

    max_value = get_adc_value(adc_channel, const.CALIBRATE_ITER)

    return min_value, max_value


def get_pedal_value(adc_channel, min_max, offset: int = 0):
    sety = int(range_map(get_adc_value(adc_channel), min_max[0], min_max[1], -127, 127))

    # Use a dead zone
    if abs(sety) <= const.DEAD_ZONE - offset:
        sety = 0

    return sety


def get_steering_value(as5600, center):
    if as5600.MD and not as5600.ML and not as5600.MH:
        setz = int(range_map(
            as5600.ANGLE + center,
            const.DEAD_ZONE_AS5600,
            const.ANGLE_USAGE_AS5600 - const.DEAD_ZONE_AS5600,
            out_min= -127,
            out_max= 127
        ))
        return setz
    elif const.DEBUG:
        print(f"MD {as5600.MD}, ML {as5600.ML}, MH {as5600.MH}")

def wait_to_release_and_press(keys, btn: int):
    key_event = keys.events.get()
    while key_event is None or key_event.key_number != btn or not key_event.released:
        key_event = event if (event := keys.events.get()) else key_event

    while key_event.key_number != btn or not key_event.pressed:
        key_event = event if (event := keys.events.get()) else key_event
    