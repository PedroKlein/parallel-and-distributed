import os
import sys

def validate_file(file_path):
    """
    Validates the content of a file to ensure:
    1. All strings are 7 characters long.
    2. The strings are sorted in ascending order.
    3. The number of lines matches the number in the filename.
    """
    try:
        # Extract the expected number of inputs from the filename
        file_name = os.path.basename(file_path)
        if not file_name.startswith("in_") or not file_name.endswith(".in.out"):
            print(f"Invalid file name format: {file_name}")
            return False
        
        num_inputs = int(file_name.split("_")[1].split(".")[0])

        with open(file_path, 'r') as file:
            lines = file.read().splitlines()

        # Check if the number of lines matches the expected number of inputs
        if len(lines) != num_inputs:
            print(f"Validation failed: Expected {num_inputs} lines, but found {len(lines)} lines.")
            return False

        # Check if all strings are 7 characters long and sorted
        for i in range(len(lines)):
            if len(lines[i]) != 7:
                print(f"Validation failed: Line {i + 1} ('{lines[i]}') is not 7 characters long.")
                return False
            if i > 0 and lines[i] < lines[i - 1]:
                print(f"Validation failed: Line {i + 1} ('{lines[i]}') is not in ascending order.")
                return False

        print(f"Validation passed for file: {file_name}")
        return True

    except Exception as e:
        print(f"An error occurred while validating the file: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python validate.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    if not os.path.isfile(file_path):
        print(f"File not found: {file_path}")
        sys.exit(1)

    validate_file(file_path)