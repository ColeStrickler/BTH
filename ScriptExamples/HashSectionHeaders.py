import mgr
import hashlib

mgr.LoadFile('C:\\Windows\\notepad.exe')

def hash_bytes(data):
    # Create a new SHA-256 hash object
    md5 = hashlib.md5()
    # Update the hash object with the bytes to be hashed
    md5.update(data)

    # Get the hexadecimal representation of the hash
    hash_result = md5.hexdigest()

    return hash_result




sh = mgr.SectionHeaders()


for dir in sh:
	dir_start = mgr.rvaToRaw(dir.VirtualAddress)
	dir_size = dir.SizeOfRawData
	section_bytes = b''
	# gather all bytes from section
	for i in range(dir_size):
		byte = bytes.fromhex(mgr.GetByte(dir_start + i))
		
		section_bytes += byte
	print(f'{dir.Name}:\t{hash_bytes(section_bytes)}')
	
