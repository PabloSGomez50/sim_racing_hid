from as5600 import AS5600, AS5600_ID
import busio
import board
import time

# Initialize I2C
i2c = busio.I2C(scl=board.GP9, sda=board.GP8)

# Create AS5600 object
as5600 = AS5600(i2c, AS5600_ID)

# Example usage: scan the bus and print the configuration register
as5600.scan()
as5600.CONF = 0x64
print("should be 0x64", hex(as5600.CONF))
print("Power mode", as5600.PM)
# Example usage: print the magnet status
print(as5600.magnet_status())


# Loop to continuously read and print angle data
while True:
    angle = as5600.ANGLE
    if as5600.MD and not as5600.ML and not as5600.MH:
            
        print(f"Angle:\t\t{angle}")
        print(f"Raw Angle:\t{as5600.RAWANGLE}")
        #print(f"Fix angle: {fix_angle}")
        print()
        time.sleep(1)
    else:
        print(as5600.magnet_status())
        time.sleep(1.5)
        
