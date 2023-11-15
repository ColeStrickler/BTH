import mgr
import hashlib

# This does not use the mgr module but shows how hashing can be done in BTH

def calculate_file_hash(file_path, algorithm='sha256', buffer_size=65536):
    # Open the file in binary mode
    with open(file_path, 'rb') as file:
        # Create a hash object
        hasher = hashlib.new(algorithm)

        # Read the file in chunks and update the hash
        while chunk := file.read(buffer_size):
            hasher.update(chunk)

    # Return the hexadecimal representation of the hash
    return hasher.hexdigest()

# Example: Calculate SHA-256 hash of a file
file_path = 'C:\\Windows\\notepad.exe'
file_hash = calculate_file_hash(file_path)
print(f'SHA-256 Hash of {file_path}: {file_hash}')