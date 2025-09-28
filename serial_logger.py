#Serial data collector for arduino/esp etc

import serial 
import csv


#establish connection
port_no = 'COM' + (input("Enter COM number:"))

ser = serial.Serial(port = port_no, baudrate = 115200, timeout = 1)

#check port is open 
if ser.is_open:
    print("Serial port is open!")

#function to only parse data and not error/other serial messages
def parse_msg(line):
    if line.startswith("#DATA:<") and line.endswith('>'):
        try:
            raw = line.split(":")[1].strip().strip("<> ")
            values = list(map(float, raw.split(",")))
            if len(values) == 5:
                return values
        except ValueError:
            ser.write(b"ValueError!")
    return None

#read and write to csv
fieldnames = ['Latitude', 'Longitude', 'Altitude', 'RSSI', 'SNR']
with open('gps_LORA_signal_log.csv', 'w', newline = '') as file:
    writer = csv.DictWriter(file, fieldnames = fieldnames)
    writer.writeheader()
while True:
    dataString = ser.readline().decode('utf-8').strip()
    values = parse_msg(dataString)
    if isinstance(values, list):
        print(values)
        data = dict(zip(fieldnames, values))
        with open('gps_LORA_signal_log.csv', 'a', newline='') as file:
            writer = csv.DictWriter(file, fieldnames = fieldnames)
            writer.writerow(data)
        
