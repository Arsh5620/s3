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
	magic = 0xD008000000000000

	print("Press enter to send the packet: ")
	data_entered	= input() # wait for the client to press enter before sending the packet

	fin = open('one.pdf', 'rb') # open file one.pdf to send 

	file_data = fin.read()
	file_len = len(file_data)

	magic += file_len

	header_keys = "action=create\nfilename=\"filename\"\nfolder=\".\"\ncrc=00565423\n"
	header_keys += ' ' * (128 - len(header_keys))

	sock.send(magic.to_bytes(8, byteorder = "little"))
	sock.send(header_keys.encode())

	# now we have send the request to the server, and must wait for the server response

	packet_response	= PacketResponseReader()
	response_data_code	= packet_response.getDictionary()["response"]
	print("Server response said : " + packet_response.getData())
	print("Server response code is : " + response_data_code)
	if (response_data_code == "1"):
		length = 0xD0D1000000000000
		length += file_len
		sock.send(length.to_bytes(8, byteorder = "little"))
		sock.send(file_data)
		print("Server data sent")
		packet_response2	= PacketResponseReader()
		print("")
		print("Response for data received")
		data_response_code	= packet_response2.getDictionary()["response"]
		print("Response code for data sent : " + data_response_code)
		print("Response data for data sent : " + packet_response2.getData())
		if (packet_response2 == "2"):
			print("Server accepted the data that was sent")
		else:
			print("Server rejected the data that was sent")
	else:
		print("Server rejected data")
	
	print("****")
