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

# Read values from the output_array.h file and replace 0 with 1
def read_values_from_file(file_path):
    values = []
    with open(file_path, 'r') as file:
        content = file.read()
        # Extract values within curly braces
        matches = re.search(r'\{([^}]*)\}', content)
        if matches:
            values_str = matches.group(1)
            # Replace 0 with 1 and extract all numerical values
            values = [int(value.strip()) if value.strip() and value.strip() != "0" else 1 for value in values_str.split(',')]
    return values

try:
    # Get values from the output_array.h file with 0 replaced by 1
    rawData = read_values_from_file(output_array_path)

    # Send a maximum of 12,000 values
    max_values_to_send = min(len(rawData), 12000)
    for i, value in enumerate(rawData[:max_values_to_send], start=1):
        # Convert the value to two bytes (uint16_t)
        value_bytes = value.to_bytes(2, byteorder='little')

        # Send the value over the serial port
        ser.write(value_bytes)

        print(f"Sent value {i}/{max_values_to_send}: {value}")

        # Wait for a moment before sending the next value (adjust as needed)
        time.sleep(1/1000)

except KeyboardInterrupt:
    print("Script terminated by user.")
finally:
    # Close the serial port connection when done
    ser.close()

