from machine import Pin, ADC, PWM, UART, deepsleep
import time
from notes import mario
import _thread
print("Hello, World!")
buzzer = Pin(19, Pin.OUT)
latest_value = 0
uart1 = UART(1, baudrate=9600, tx=33, rx=32)



def play(pin, melodies, delays, duty):
    while True:
        global done
        print("Playing")
        # Create the pwm object
        pwm = PWM(pin)
        # Loop through the whole list
        for note in melodies:
            if note != 0:
                #print("Playing note: ", note)
                pwm.freq(note)
                pwm.duty(duty)
            time.sleep(delays)
        # Disable the pulse, setting the duty to 0
        pwm.duty(0)
        # Disconnect the pwm driver
        pwm.deinit()
        deepsleep(10000)

def read_uart():
    while True:
        if uart1.any():
            data = uart1.read(2)
            if data is not None:
                print(f"Read {int.from_bytes(data, 'little')}\n")
        


def capture_adc():
    a = _thread.start_new_thread(play, (buzzer, mario, 0.1, 100))
    read = _thread.start_new_thread(read_uart, ())
    adc = ADC(Pin(39))
    while True:
        latest_value = adc.read_u16()
        #print(latest_value)
        uart1.write(latest_value.to_bytes(2, 'little'))

capture_adc()



