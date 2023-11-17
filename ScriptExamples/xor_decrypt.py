import mgr

def xor_crypt(start, length, key):
	key = int(key, 16)
	for i in range(length):
		byte = int(mgr.GetByte(start + i), 16)
		byte = byte ^ key
		hex_string = ''.join(format(b, '02x') for b in byte.to_bytes())
		print(hex_string)
		mgr.SetByte(start + i, hex_string)

key = '0b'

xor_crypt(0, 8, key)