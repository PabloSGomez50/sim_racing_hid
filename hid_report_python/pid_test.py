import hid
import time

# Configuración basada en usb_descriptors.c
VENDOR_ID = 0xCafe
PRODUCT_ID = 0x0002  # PID calculado por _PID_MAP (HID bit 2 = 4)
REPORT_ID = 2        # Coincide con REPORT_ID_GAMEPAD
PID_USAGE_PAGE = 0x000F
PID_USAGE = 0x0001
JOYSTICK_USAGE_PAGE = 0x0001
JOYSTICK_USAGE = 0x0004

try:
    print(f"Buscando dispositivo 0x{VENDOR_ID:04x}:0x{PRODUCT_ID:04x}...")
    selected_path = None
    joystick_fallback_path = None
    for d in hid.enumerate(VENDOR_ID, PRODUCT_ID):
        if d.get("usage_page") == PID_USAGE_PAGE and d.get("usage") == PID_USAGE:
            selected_path = d["path"]
            break

        if d.get("usage_page") == JOYSTICK_USAGE_PAGE and d.get("usage") == JOYSTICK_USAGE:
            joystick_fallback_path = d["path"]

    if selected_path is None:
        if joystick_fallback_path is not None:
            selected_path = joystick_fallback_path
            print("No hay coleccion PID top-level; usando joystick top-level (esperado con descriptor actual).")
        else:
            raise RuntimeError("No se encontro el dispositivo HID de la Pico.")

    device = hid.device()
    device.open_path(selected_path)
    device.set_nonblocking(True)

    print(f"Conectado: {device.get_product_string()}")
    print("Enviando ráfagas de datos de prueba (FFB Simulation)...")

    for i in range(10):
        # El primer byte DEBE ser el Report ID
        # Los siguientes son los datos de "fuerza"
        test_data = [REPORT_ID, i * 20] 
        
        device.write(test_data)
        print(f"Enviado reporte {i}: {test_data}")
        
        # Intentar leer si la Pico nos envió algo (Input Report)
        report = device.read(64)
        if report:
            print(f"Recibido desde Pico: {report}")
            
        time.sleep(0.5)

    device.close()
    print("Test finalizado.")

except Exception as e:
    print(f"Error: {e}")