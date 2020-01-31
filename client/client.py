# This is client program for the chat server, please refer to the documentation for the server program. 

import socket
import time

sock = socket.socket()
sock.connect((socket.gethostname(), 4096))

while(1):
    print("The sock port number is :" + str(sock.getsockname()[1]))
    print("Send something to the server: ")
    # string = input()
    # length = 1#len(string)
    length = 0
    length |= 0xD040000000000008
    
    string = "action=create\nfilename=howareyou\n"
    string += ' ' * (64 - len(string))
    string += "ARSHDEEP"
    sock.send(length.to_bytes(8, byteorder = "little"))
    sock.send(string.encode())

    time.sleep(1)
    # data = sock.recv(4096)
    # print(data)