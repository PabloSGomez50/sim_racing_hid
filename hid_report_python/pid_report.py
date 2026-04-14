import time

import hid

VENDOR_ID = 0xCAFE
PRODUCT_ID = 0x0002
PID_USAGE_PAGE = 0x000F
PID_USAGE = 0x0001
JOYSTICK_USAGE_PAGE = 0x0001
JOYSTICK_USAGE = 0x0004

# Descriptor phase-2
RID_SET_EFFECT = 0x02
RID_SET_CONSTANT_FORCE = 0x03
RID_EFFECT_OPERATION = 0x04
RID_DEVICE_GAIN = 0x05
RID_DEVICE_CONTROL = 0x06


def send_report(dev: hid.device, data: list[int], label: str) -> None:
    dev.write(data)
    print(f"TX {label:<20} -> {data}")


def main() -> None:
    print("Dispositivos HID detectados:")
    selected_path = None
    selected_kind = ""
    joystick_fallback_path = None
    for device in hid.enumerate():
        print(
            f"  0x{device['vendor_id']:04x}:0x{device['product_id']:04x}"
            f" - {device.get('product_string')}"
            f" (usage_page=0x{device.get('usage_page', 0):04x}, usage=0x{device.get('usage', 0):04x})"
        )

        if (
            device["vendor_id"] == VENDOR_ID
            and device["product_id"] == PRODUCT_ID
            and device.get("usage_page") == PID_USAGE_PAGE
            and device.get("usage") == PID_USAGE
        ):
            selected_path = device["path"]
            selected_kind = "PID top-level"

        if (
            device["vendor_id"] == VENDOR_ID
            and device["product_id"] == PRODUCT_ID
            and device.get("usage_page") == JOYSTICK_USAGE_PAGE
            and device.get("usage") == JOYSTICK_USAGE
        ):
            joystick_fallback_path = device["path"]

    if selected_path is None:
        if joystick_fallback_path is not None:
            selected_path = joystick_fallback_path
            selected_kind = "Joystick top-level (fallback esperado)"
        else:
            raise RuntimeError(
                f"No se encontro el dispositivo 0x{VENDOR_ID:04x}:0x{PRODUCT_ID:04x}."
            )

    print(f"\nAbriendo {selected_kind} de 0x{VENDOR_ID:04x}:0x{PRODUCT_ID:04x}...")
    dev = hid.device()
    dev.open_path(selected_path)
    dev.set_nonblocking(True)
    print("Conexión OK. Simulando secuencia FFB de juego...\n")

    try:
        # 1) Inicializacion estilo juego
        send_report(dev, [RID_DEVICE_CONTROL, 1], "DeviceControl Enable")
        time.sleep(0.05)
        send_report(dev, [RID_DEVICE_GAIN, 220], "DeviceGain=220")
        time.sleep(0.05)

        # 2) SetEffect (8 bytes payload)
        # [block, type, durL, durH, triL, triH, gain, trigBtn]
        send_report(dev, [RID_SET_EFFECT, 1, 1, 0xFF, 0x7F, 0x00, 0x00, 200, 0], "SetEffect")
        time.sleep(0.08)

        # 3) Fuerza constante variable
        for magnitude in (3000, 9000, -7000, 0):
            mag_u16 = magnitude & 0xFFFF
            lo = mag_u16 & 0xFF
            hi = (mag_u16 >> 8) & 0xFF
            send_report(dev, [RID_SET_CONSTANT_FORCE, 1, lo, hi], f"ConstantForce={magnitude}")
            time.sleep(0.15)

        # 4) Start/Stop efecto
        send_report(dev, [RID_EFFECT_OPERATION, 1, 1, 1], "EffectStart")
        time.sleep(0.35)
        send_report(dev, [RID_EFFECT_OPERATION, 1, 3, 1], "EffectStop")

        print("\nSecuencia de simulación finalizada.")
    except Exception as e:
        print(f"Error durante la simulación: {e}")
    finally:
        dev.close()


if __name__ == "__main__":
    main()