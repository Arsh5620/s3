# This is client program for the chat server, please refer to the documentation for the server program. 

import socket

sock = socket.socket()
sock.connect((socket.gethostname(), 4096))

while(1):
    print("Send something to the server: ")
    string = input()
    sock.send(len(string).to_bytes(8, byteorder = "little"))
    sock.send(string.encode())
    data = sock.recv(4096)
    print(data)