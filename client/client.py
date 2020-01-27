# This is client program for the chat server, please refer to the documentation for the server program. 

import socket

sock = socket.socket()
sock.connect((socket.gethostname(), 4096))

while(1):
    print("The sock port number is :" + str(sock.getsockname()[1]))
    print("Send something to the server: ")
    # string = input()
    length = 1#len(string)
    length |= 0xD020000000000000
    
    string = "actin=notification"
    string += ' ' * (32 - len(string))
    sock.send(length.to_bytes(8, byteorder = "little"))
    sock.send(string.encode())
    # data = sock.recv(4096)
    # print(data)