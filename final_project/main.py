import os
import sys
import uos
import struct
import gfx
import uasyncio
import _thread
from utime import sleep
from sdcard import SDCard
from machine import SPI, SoftSPI
from machine import Pin, ADC, I2S, I2C
from ili9341 import ILI9341, color565
import time
import random
from pico_neotrellis import NeoTrellis
from rotary_irq_rp2 import RotaryIRQ

#sLock = _thread.allocate_lock()

SCR_WIDTH = const(320)
SCR_HEIGHT = const(240)
SCR_ROT = const(0)

TFT_CLK_PIN = const(2)
TFT_MOSI_PIN = const(3)
TFT_MISO_PIN = const(0)

TFT_CS_PIN = const(1)
TFT_RST_PIN = const(4)
TFT_DC_PIN = const(5)




##core task
def wav_player():
    
    while True:
        #sLock.acquire()        
        
        cs = Pin(13, machine.Pin.OUT)
        spi = SPI(
            0,
            baudrate=1_000_000,  # this has no effect on spi bus speed to SD Card
            polarity=0,
            phase=0,
            bits=8,
            firstbit=machine.SPI.MSB,
            sck=Pin(2),
            mosi=Pin(3),
            miso=Pin(0))

        sd = SDCard(spi, cs)
        sd.init_spi(25_000_000)  # increase SPI bus speed to SD card
        os.mount(sd, "/sd")

        # ======= I2S CONFIGURATION =======
        SCK_PIN = 16
        WS_PIN = 17
        SD_PIN = 18
        I2S_ID = 0
        BUFFER_LENGTH_IN_BYTES = 40000
        # ======= I2S CONFIGURATION =======

        # ======= AUDIO CONFIGURATION =======
        WAV_FILE = "storm.wav"
        WAV_SAMPLE_SIZE_IN_BITS = 32
        FORMAT = I2S.STEREO
        SAMPLE_RATE_IN_HZ = 48000
        # ======= AUDIO CONFIGURATION =======
        
        global audio_out

        audio_out = I2S(
        I2S_ID,
        sck=Pin(SCK_PIN),
        ws=Pin(WS_PIN),
        sd=Pin(SD_PIN),
        mode=I2S.TX,
        bits=WAV_SAMPLE_SIZE_IN_BITS,
        format=FORMAT,
        rate=SAMPLE_RATE_IN_HZ,
        ibuf=BUFFER_LENGTH_IN_BYTES,
        )

        wav = open("/sd/{}".format(WAV_FILE), "rb")
        _ = wav.seek(44)  # advance to first byte of Data section in WAV file

        # allocate sample array
        # memoryview used to reduce heap allocation
        wav_samples = bytearray(10000)
        wav_samples_mv = memoryview(wav_samples)

        # continuously read audio samples from the WAV file
        # and write them to an I2S DAC
        print("==========  START PLAYBACK ==========")
        
        ##call displayWav to show song title
        setupTFT(WAV_FILE)
        
        try:
            while True:
                num_read = wav.readinto(wav_samples_mv)
                # end of WAV file?
                if num_read == 0:
                    # end-of-file, advance to first byte of Data section
                    _ = wav.seek(44)
                else:
                    ##make call to volume change?
                    r.add_listener(rotaryEncoder())
                    audio_out.shift(buf=wav_samples_mv, bits=WAV_SAMPLE_SIZE_IN_BITS, shift=vol_state)
                    _ = audio_out.write(wav_samples_mv[:num_read])
                    pass
                    
        except (KeyboardInterrupt, Exception) as e:
            print("caught exception {} {}".format(type(e).__name__, e))

        # cleanup
        wav.close()
        os.umount("/")
        spi.deinit()
        audio_out.deinit()
        #sLock.release()
#_thread.start_new_thread(wav_player, ())

def setupTFT(s):    
    spi = SoftSPI(
    #1,
    polarity=1, phase=0,
    baudrate=40_000_000,
    miso=Pin(TFT_MISO_PIN),
    mosi=Pin(TFT_MOSI_PIN),
    sck=Pin(TFT_CLK_PIN))
    
    global display
    
    display = ILI9341(spi, 
    cs=Pin(TFT_CS_PIN),
    dc=Pin(TFT_DC_PIN),
    rst=Pin(TFT_RST_PIN),
    w=SCR_WIDTH,
    h=SCR_HEIGHT,
    r=SCR_ROT)
    
    display.fill_rectangle(0, 0, 240, 320, color=color565(255, 0, 0))
    display.set_pos(20,20)
    display.write(s)
        

def neotrellis():
    #sLock.acquire()
    # create the i2c object for the trellis
    i2c_bus = I2C(0)

    # create the trellis
    trellis = NeoTrellis(i2c_bus)

    # Set the brightness value (0 to 1.0)
    trellis.brightness = 0.02

    # some color definitions
    OFF = (0, 0, 0)
    RED = (255, 0, 0)
    YELLOW = (255, 150, 0)
    GREEN = (0, 255, 0)
    CYAN = (0, 255, 255)
    BLUE = (0, 0, 255)
    PURPLE = (180, 0, 255)

    colors_arr = [RED, YELLOW, GREEN, CYAN, BLUE, PURPLE]

    # this will be called when button events are received
    def blink(event):
        # turn the LED on when a rising edge is detected
        if event.edge == NeoTrellis.EDGE_RISING:
            trellis.pixels[event.number] = random.choice(colors_arr)
            print("button", event.number, "pressed")
        # turn the LED off when a falling edge is detected
        elif event.edge == NeoTrellis.EDGE_FALLING:
            trellis.pixels[event.number] = OFF

    for i in range(16):
        # activate rising edge events on all keys
        trellis.activate_key(i, NeoTrellis.EDGE_RISING)
        # activate falling edge events on all keys
        trellis.activate_key(i, NeoTrellis.EDGE_FALLING)
        # set all keys to trigger the blink callback
        trellis.callbacks[i] = blink

        # cycle the LEDs on startup
        trellis.pixels[i] = random.choice(colors_arr)
        time.sleep(0.05)

    for i in range(16):
        trellis.pixels[i] = OFF
        time.sleep(0.05)

    while True:
        # call the sync function call any triggered callbacks
        trellis.sync()
        # the trellis can only be read every 17 millisecons or so
        time.sleep(0.02)
    #sLock.release()
#_thread.start_new_thread(neotrellis, ())


def volumeChanged(encoder):
    #audio_out.irq(handler=None)
    print(encoder.value())
    vol_state = encoder.value()
    
    #audio_out.irq(handler=volumeChanged)
    
    
    
##### initialize vars and call initial setup functions
a = 13
volume = 0

global vol_state

r = RotaryIRQ(pin_num_clk=20,
              pin_num_dt=19,
              min_val=-5,
              max_val=5,
              incr=0.2,
              reverse=False,
              range_mode=RotaryIRQ.RANGE_BOUNDED)
r.add_listener(volumeChanged)


#audio_out.irq(trigger=Pin.IRQ_RISING | Pin.IRQ_FALLING, handler=toggle_toggled)

wav_player()
setupTFT()


#put code that needs to repeat in here
while True:
    #spLock.acquire()
    #volumeChanged(r)
    sleep(3)
    #neotrellis()

    