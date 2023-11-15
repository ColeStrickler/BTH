import mgr

mgr.LoadFile("C:\\Windows\\notepad.exe")

strings = mgr.StringScan(5)

with open(".\\strings_out.txt", "w") as f:
	for s in strings:	
		f.write(s + "\n")
			

