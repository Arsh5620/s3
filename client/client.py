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
    length |= 0xD004000000000000

    notification = input() * 100
    notification_len = len(notification)
    
    length += (notification_len)
    
    string = "action=create\nfilename=\"filename 2 what the fuck\"\ncrc=00565423\n"
    # string = "action=notification\n"
    string += ' ' * (64 - len(string))
    
    sock.send(length.to_bytes(8, byteorder = "little"))
    sock.send(string.encode())

    sock.send(notification.encode())
    # data = sock.recv(4096)
    # print(data)