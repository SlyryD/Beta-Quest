def get_original_logo(rom):
	writeAddress = 0x01795300
	originalBytes = rom.original.buffer[writeAddress: writeAddress+ 102400]
	return originalBytes

def diff(arr1, arr2):
	return bytearray([a ^ b for a, b in zip(arr1, arr2 )])


def create_logo_diff(rom, new_logo_file, output_diff):
	new_logo_file = open(new_logo_file, 'rb')
	original_logo_bytes = get_original_logo(rom)
	new_logo_bytes = new_logo_file.read()
	logo_bytes_diff = diff(original_logo_bytes, new_logo_bytes )
	diff_file = open(output_diff, 'wb')
	diff_file.write(logo_bytes_diff)
	diff_file.close()

def create_logo_from_diff(rom, diff_file, output_file):
	diff_logo_file = open(diff_file, 'rb')
	diff_bytes = diff_logo_file.read()
	diff_logo_file.close()
	original_logo_bytes = get_original_logo(rom)
	new_logo_bytes = diff(original_logo_bytes, diff_bytes)
	output_file = open(output_file, 'wb')
	output_file.write(new_logo_bytes)
	output_file.close()

