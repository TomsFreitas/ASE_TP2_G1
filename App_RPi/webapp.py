# DHT11

import time
import board
import adafruit_dht
import statistics
import smbus
import RPi.GPIO as GPIO
import collections
from datetime import datetime

GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.IN, pull_up_down = GPIO.PUD_OFF)
# Defines the sensor and it's pin
dhtDevice = adafruit_dht.DHT11(board.D4)

humidity_values = collections.deque(maxlen=5)
temperature_values = collections.deque(maxlen=5)
full_lux = None
vis_lux = None
ir_lux = None
last_noise_timestamp = None


def read_temperatures():
    global humidity_values
    global temperature_values
    while True:
        try:
            # Read 5 times and supply with median
            # Try to read the sensor
            if dhtDevice.humidity is not None and dhtDevice.temperature is not None:
                humidity_values.append(dhtDevice.humidity)
                temperature_values.append(dhtDevice.temperature)
            time.sleep(0.2)
        except:
            continue

def read_light():
    global full_lux
    global vis_lux
    global ir_lux
    while True:
        try:
            bus = smbus.SMBus(1)

            # TSL2561 address, 0x39(57)
            # Select control register, 0x00(00) with command register, 0x80(128)
            #		0x03(03)	Power ON mode
            bus.write_byte_data(0x39, 0x00 | 0x80, 0x03)
            # TSL2561 address, 0x39(57)
            # Select timing register, 0x01(01) with command register, 0x80(128)
            #		0x02(02)	Nominal integration time = 402ms
            bus.write_byte_data(0x39, 0x01 | 0x80, 0x02)

            time.sleep(0.5)

            # Read data back from 0x0C(12) with command register, 0x80(128), 2 bytes
            # ch0 LSB, ch0 MSB
            data = bus.read_i2c_block_data(0x39, 0x0C | 0x80, 2)

            # Read data back from 0x0E(14) with command register, 0x80(128), 2 bytes
            # ch1 LSB, ch1 MSB
            data1 = bus.read_i2c_block_data(0x39, 0x0E | 0x80, 2)

            # Convert the data
            full_lux = data[1] * 256 + data[0]
            ir_lux = data1[1] * 256 + data1[0]
            vis_lux = full_lux - ir_lux
            time.sleep(0.5)
        except:
            continue

def read_mic():
    # read digital pin 17
    global last_noise_timestamp
    while True:
        try:
            if GPIO.input(17) == 0:
                last_noise_timestamp = time.time()
            time.sleep(0.5)
        except:
            continue
    



# Add webserver
from flask import Flask
app = Flask(__name__)
@app.route('/values')
def values():
    global humidity_values
    global temperature_values
    global vis_lux
    global ir_lux
    global last_noise_timestamp
    
    median_temperature = statistics.median(temperature_values)
    median_humidity = statistics.median(humidity_values)
        # Return the values if not None
    if median_temperature is not None and median_humidity is not None:
        return "local_temp {}\nlocal_humidity {}\nvis_lux {}\nir_lux {}\nactivity_timestamp {}\n".format(median_temperature, median_humidity, vis_lux, ir_lux, last_noise_timestamp ), 200, {'Content-Type': 'text/plain; charset=utf-8'}

#spawn threads
import threading
threading.Thread(target=read_temperatures).start()
threading.Thread(target=read_light).start()
threading.Thread(target=read_mic).start()

app.run(host='0.0.0.0', port=5000)
