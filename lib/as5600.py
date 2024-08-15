from adafruit_bus_device.i2c_device import I2CDevice
from struct import unpack, pack
from collections import namedtuple

AS5600_ID = 0x36  # Device ID
M12 = (1 << 12) - 1  # 0xFFF

REGS = namedtuple('REGS', 'ZMCO ZPOS MPOS MANG CONF RAWANGLE ANGLE STATUS AGC MAGNITUDE BURN')
r = REGS(0, 1, 3, 5, 0x07, 0xC, 0xE, 0xB, 0x1A, 0x1B, 0xFF)

class RegDescriptor:
    "Read and write a bit field from a register"
    
    def __init__(self, reg, shift, mask, buffsize=2):
        "Initialize with specific identifiers for the bit field"
        self.reg = reg
        self.shift = shift
        self.mask = mask
        self.buffsize = buffsize
        self.writeable = (r.ZMCO, r.ZPOS, r.MPOS, r.MANG, r.CONF, r.BURN)
        
    def get_register(self, obj):
        "Read an I2C register"
        if self.reg in obj.cache:
            return obj.cache[self.reg]
        
        with obj.i2c_device as i2c:
            buff = bytearray(self.buffsize)
            register_address = bytearray([self.reg])
            i2c.write_then_readinto(register_address, buff)
        
        if self.reg == r.CONF:
            print(buff)
        if self.buffsize == 2:
            v = unpack(">H", buff)[0]  # 2 bytes big endian
        else:
            v = unpack(">B", buff)[0]
        
        if self.reg in self.writeable:
            obj.cache[self.reg] = v
            
        return v
        
    def __get__(self, obj, objtype):
        "Get the register then extract the bit field"
        v = self.get_register(obj)
        v >>= self.shift
        v &= self.mask
        return v
    
    def __set__(self, obj, value):
        "Insert a new value into the bit field of the old value then write it back"
        if self.reg not in self.writeable:
            raise AttributeError('Register is not writable')
        oldvalue = self.get_register(obj)
        insertmask = 0xffff - (self.mask << self.shift)
        oldvalue &= insertmask
        value &= self.mask
        value <<= self.shift
        oldvalue |= value
        if self.buffsize == 2:
            buff = pack(">H", oldvalue)
        else:
            buff = pack(">B", oldvalue)
        
        with obj.i2c_device as i2c:
            register_address = bytearray([self.reg])
            i2c.write(register_address)
            
        obj.cache[self.reg] = oldvalue

class AS5600:
    def __init__(self, i2c, device_address):
        self.i2c_device = I2CDevice(i2c, device_address)
        self.cache = {}  # cache register values
        self.i2c_bus = i2c
        
    #Use descriptors to read and write a bit field from a register
    #1. we read one or two bytes from i2c
    #2. We shift the value so that the least significant bit is bit zero
    #3. We mask off the bits required  (most values are 12 bits hence M12)
    ZMCO=      RegDescriptor(r.ZMCO,shift=0,mask=3,buffsize=1) #2 bit
    ZPOS=      RegDescriptor(r.ZPOS,0,M12) #zero position
    MPOS=      RegDescriptor(r.MPOS,0,M12) #maximum position
    MANG=      RegDescriptor(r.MANG,0,M12) #maximum angle (alternative to above)
    #Dummy example how how to make friendlier duplicate names if you want to
    #max_angle = RegDescriptor(r.MANG,0,M12) #maximum angle (alternative to above)
    CONF=      RegDescriptor(r.CONF,0,(1<<14)-1) # this register has 14 bits (see below)
    #CONF=      RegDescriptor(r.CONF,0,M12) # this register has 14 bits (see below)
    
    RAWANGLE=  RegDescriptor(r.RAWANGLE,0,M12) 
    ANGLE   =  RegDescriptor(r.ANGLE,0,M12) #angle with various adjustments (see datasheet)
    TEST_ANG = RegDescriptor(0x0C,0,(1<<4) - 1, 1)
    TEST2_ANG = RegDescriptor(0x0D,0,(1<<8) - 1, 1)
    STATUS=    RegDescriptor(r.STATUS,0,M12) #basically strength of magnet
    AGC=       RegDescriptor(r.AGC,0,0xF,1) #automatic gain control
    MAGNITUDE= RegDescriptor(r.MAGNITUDE,0,M12) #? something to do with the CORDIC for atan RTFM
    BURN=      RegDescriptor(r.BURN,0,0xF,1)

    #Configuration bit fields
    PM =      RegDescriptor(r.CONF,0,0x3) #2bits Power mode
    HYST =    RegDescriptor(r.CONF,2,0x3) # hysteresis for smoothing out zero crossing
    OUTS =    RegDescriptor(r.CONF,4,0x3) # HARDWARE output stage ie analog (low,high)  or PWM
    PWMF =    RegDescriptor(r.CONF,6,0x3) #pwm frequency
    SF =      RegDescriptor(r.CONF,8,0x3) #slow filter (?filters glitches harder) RTFM
    FTH =     RegDescriptor(r.CONF,10,0x7) #3 bits fast filter threshold. RTFM
    WD =      RegDescriptor(r.CONF,13,0x1) #1 bit watch dog - Kicks into low power mode if nothing changes
    
    #status bit fields. ?having problems getting these to make sense
    MH =      RegDescriptor(r.STATUS,3,0x1,1) #2bits  Magnet too strong (high)
    ML =      RegDescriptor(r.STATUS,4,0x1,1) #2bits  Magnet too weak (low)
    MD =      RegDescriptor(r.STATUS,5,0x1,1) #2bits  Magnet detected
    
    
    def scan(self):
        "Debug utility function to check your I2C bus"
        while not self.i2c_bus.try_lock():
            pass
        try:
            devices = self.i2c_bus.scan()
            print([hex(device) for device in devices])
            if AS5600_ID in devices:
                print(f'Found AS5600 (id = {hex(AS5600_ID)})')
            #print(self.CONF)
        finally:
            self.i2c_bus.unlock()
            
        print("Finish scan")
        
    def burn_angle(self):
        "Burn ZPOS and MPOS - (can only do this 3 times)"
        #self.BURN = 0x80
        
    def burn_setting(self):
        "Burn config and MANG - (can only do this once)"
        #self.BURN = 0x40
    
    def magnet_status(self):
        "Magnet status - this does not seem to make sense? Why"
        s = "Magnet "
        if self.MD == 1:
            s += "detected"
        else:
            s += "not detected"
        if self.ML == 1:
            s += " (magnet too weak)"
        if self.MH == 1:
            s += " (magnet too strong)"
        return s



