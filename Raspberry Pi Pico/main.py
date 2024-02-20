#### TEMPERATURE LAMP MAIN CODE

from machine import Pin, PWM, ADC
from utime import sleep

#if kat needs to go outside to water her garden and pull weeds, there is an override switch
#use interrupts :) if switch is OFF, program runs as intended, if switch is ON,
#use interrupt handler until switch is back ON
def toggle_toggled(pin):
    global toggle_sw_state
    toggle_sw.irq(handler=None)
    sleep(.1)
    
    if (toggle_sw.value() == 1) and (toggle_sw_state == 0):
        toggle_sw_state = 1 #switch ON
            
    elif (toggle_sw.value() == 0) and (toggle_sw_state == 1):
        while toggle_sw.value() == 0: #while the switch is in the OFF position
            toggle_sw_state = 0 #switch OFF
            pwm17.duty_u16(65535)
            pwm16.duty_u16(0)
        
    toggle_sw.irq(handler=toggle_toggled)

    
def convert_temp(reading):
    voltage = reading * 3.3 / 65535 # 16-bit resolution on ADC
    celsius = 27 - (voltage - 0.706)/0.001721 # 27C = 0.706V, +1 degree = 1.721mV
    return(celsius)

# initialize variables

# create objects for LEDs
p16 = Pin(16, Pin.OUT)
pwm16 = PWM
pwm16 = PWM(p16)
pwm16.freq(1000)
pwm16.duty_u16(0)

p17 = Pin(17, Pin.OUT)
pwm17 = PWM
pwm17 = PWM(p17)
pwm17.freq(1000)
pwm17.duty_u16(0)
   
#create objects for irl switch    
toggle_sw = Pin(15, Pin.IN, Pin.PULL_DOWN)
toggle_sw.irq(trigger=Pin.IRQ_RISING | Pin.IRQ_FALLING, handler=toggle_toggled)    

#maybe fade between LEDs?
toggle_sw_state = toggle_sw.value()
print("toggle switch state:", toggle_sw_state)

### Ready, Set, Go!
temp = ADC(4)

while True:
    #if temp outside is more than 32C, (gross) the red LED comes on.
    if (convert_temp(temp.read_u16()) > 32.00): #far too warm
        pwm16.duty_u16(65535)
        pwm17.duty_u16(0)
    elif (convert_temp(temp.read_u16()) < 32.00 and convert_temp(temp.read_u16()) > 24.00): #getting there!
        pwm16.duty_u16(12767)
        pwm17.duty_u16(12767)
    #if temp outside is less than 24C, the green LED comes on.
    elif (convert_temp(temp.read_u16()) <= 24.00): #just right
        pwm16.duty_u16(0)
        pwm17.duty_u16(65535)
        
    sleep(.1)

