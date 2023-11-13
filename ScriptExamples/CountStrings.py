import mgr
import os
from pathlib import Path

# This script will count the number of times the string "windows" appears within exe files
# in the C:\Windows\ directory



dir_path = 'C:\\Windows\\'
directory_path = Path(dir_path)

# List all files in the directory
file_list = [f.name for f in directory_path.iterdir() if f.is_file()]

# Optionally, filter the files by file extension, e.g., .txt
exe_files = [dir_path+f for f in file_list if f.endswith('.exe')]
print(exe_files)

i = 0
for path in exe_files[2:9]:
	mgr.LoadFile(path)
	scan = mgr.StringScan(5)
	for s in scan:
		if "windows" in s.lower():
			i += 1

print(f"Windows found {i} times in the Windows directory!") 