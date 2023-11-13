import serial
import time
import re

# Define the serial port and baud rate
serial_port = '/dev/ttyUSB0'
baud_rate = 9600

# Create a serial object
ser = serial.Serial(serial_port, baud_rate, timeout=1)

# Define the path to your output_array.h file
output_array_path = 'output_array.h'

# Read values from the output_array.h file
def read_values_from_file(file_path):
    values = []
    with open(file_path, 'r') as file:
        content = file.read()
        matches = re.findall(r'\b\d+\b', content)  # Extract all numerical values
        values = [int(match) for match in matches]
    return values

try:
    # Get values from the output_array.h file
    rawData = read_values_from_file(output_array_path)

    for value in rawData:
        # Convert the value to two bytes (uint16_t)
        value_bytes = value.to_bytes(2, byteorder='little')

        # Send the value over the serial port
        ser.write(value_bytes)

        print(f"Sent value: {value}")

        # Wait for a moment before sending the next value (adjust as needed)
        time.sleep(1/1000)

except KeyboardInterrupt:
    print("Script terminated by user.")
finally:
    # Close the serial port connection when done
    ser.close()

