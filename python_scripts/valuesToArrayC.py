def separate_into_columns(input_file, output_file, array_name="rawData", column_size=50, shift_bits=6):
    with open(input_file, 'r') as file:
        data = file.read().strip()

        # Split the values based on commas and filter out empty strings
        values = [int(value.strip()) << shift_bits for value in data.split(',') if value.strip()]

        # Separate into columns
        columns = [values[i:i + column_size] for i in range(0, len(values), column_size)]

    # Write the C array declaration to the output file
    with open(output_file, 'w') as output:
        output.write(f"const uint16_t {array_name}[{len(values)}] = {{\n")
        
        # Write the values in columns
        for column in columns:
            output.write('\t' + ', '.join(map(str, column)) + ',\n')

        output.write("};\n")

if __name__ == "__main__":
    input_file = "pacman.txt"  # Change this to the path of your input text file
    output_file = "output_array.h"  # Change this to the desired output file
    array_name = "rawData"  # Change this to the desired array name
    column_size = 50  # Change this to the desired number of values per column
    shift_bits = 0  # Change this to the desired number of bits to shift

    separate_into_columns(input_file, output_file, array_name, column_size, shift_bits)

