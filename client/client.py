# This is client program for the chat server, please refer to the documentation for the server program. 

import socket
import time

sock = socket.socket()
print("Client started, and will try to connect to dbp server")

sock.connect((socket.gethostname(), 4096))

print("The sock port number is : " + str(sock.getsockname()[1]))
print("")
print("Connected to the server")
print("")


class PacketResponseReader:
	dictionary	= dict()
	return_data	= ""

	def __init__(self):
		recv_packet_header	= sock.recv(8)	

		recv_packet_magic	= recv_packet_header[7]
		recv_header_size	= recv_packet_header[6] * 16
		recv_data_size		= recv_packet_header[0:5]

		if (recv_packet_magic != 0xD0):
			return

		print("Server response received")

		recv_data_size_int	= int.from_bytes(recv_data_size, byteorder='little')
		recv_header_keys	= sock.recv(recv_header_size)
		recv_data 			= sock.recv(recv_data_size_int)

		header_data	= recv_header_keys.decode("ascii").rstrip(' \0')
		self.return_data	= recv_data.decode("ascii")

		header_list	= header_data.split("\r\n")

		for pair in header_list:
			pair_split = pair.split('=')
			if (len(pair_split) == 2):
				self.dictionary[pair_split[0].strip()] = pair_split[1].strip()
	
	def getDictionary(self):
		return(self.dictionary)

	def getData(self):
		return(self.return_data)

while(1):
	magic0	= 0xD008000000000000
	magic1	= 0xD0D1000000000000
	magic_invalid0	= 0x0008000000000000
	magic_invalid1	= 0x00D1000000000000

	key_value_pairs	= ("action=create\n"
		"filename=\"filename\"\n"
		"folder=\".\"\n"
		"crc=00565423\n")
	key_value_pair_nonparseable	= ("action.==create\n"
		"filename~\"filename\"\n"
		"folder\n"
		"crc=jkhkjhkk\n")
	key_value_pair_onlykeys	= ("action=create\n"
		"filename=\"\"\n"
		"folder=\"\"\n"
		"crc=1234\n")

	key_value_pair_thin		= ("action=create\n")
	key_value_pair_invalidaction	= ("action=whatever\n")
	key_value_pair_empty	= ("\n")

	print("Please select of the options to continue: ")
	print("1. Send corrupted packet with incorrect magic\n"
		"2. Send corrupted data packet with incorrect magic\n"
		"3. Send non-parse-able headers key/value pairs\n"
		"4. Send empty header key/value pairs\n"
		"5. Send invalid action key value pair\n"
		"6. Send thin attribs for the action\n"
		"7. Send correct/valid packet\n"
		"8. Send packet with keys only, and no values\n")

	data_entered	= input() # wait for the client to press enter before sending the packet

	magic_packet	= magic0
	magic_data		= magic1
	header_pairs	= key_value_pairs
	if (data_entered == "1"):
		magic_packet	= magic_invalid0
	elif (data_entered == "2"):
		magic_data		= magic_invalid1
	elif (data_entered == "3"):
		header_pairs	= key_value_pair_nonparseable
	elif (data_entered == "4"):
		header_pairs	= key_value_pair_empty
	elif (data_entered == "5"):
		header_pairs	= key_value_pair_invalidaction
	elif (data_entered == "6"):
		header_pairs	= key_value_pair_thin
	elif (data_entered == "7"):
		# do nothing, this is the default
		header_pairs	= key_value_pairs
	elif (data_entered == "8"):
		header_pairs	= key_value_pair_onlykeys
	else:
		print("Incorrect option selected, kill "
			"program with Ctrl + C if does not exit")
		sock.recv(1) # just to check if the sock is open
		exit()

	fin = open('one.pdf', 'rb') # open file one.pdf to send 

	file_data = fin.read()
	file_len = len(file_data)

	magic_packet += file_len

	header_keys = header_pairs
	header_keys += ' ' * (128 - len(header_keys))

	sock.send(magic_packet.to_bytes(8, byteorder = "little"))
	sock.send(header_keys.encode())

	# now we have send the request to the server, and must wait for the server response

	packet_response	= PacketResponseReader()
	response_data_code	= packet_response.getDictionary()["response"]
	print("Server response code is : " + response_data_code)
	print("Server response said : " + packet_response.getData())
	if (response_data_code == "1"):
		magic_data += file_len
		sock.send(magic_data.to_bytes(8, byteorder = "little"))
		sock.send(file_data)
		print("Server data sent")
		packet_response2	= PacketResponseReader()
		print("")
		print("Response for data received")
		data_response_code	= packet_response2.getDictionary()["response"]
		print("Response code for data sent : " + data_response_code)
		print("Response data for data sent : " + packet_response2.getData())
		if (data_response_code == "2"):
			print("Server accepted the data that was sent")
		else:
			print("Server rejected the data that was sent")
	else:
		print("Server rejected data")
	
	print("****")
