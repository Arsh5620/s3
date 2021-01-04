# This is a testing implementation for the client

import socket
import time
import ssl
import os
import sys

import inspect
from packet_response_reader import PacketResponseReader
from packet_response_writer import PacketResponseWriter

sock = socket.socket()
hostname = "localhost"

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)

print("Trying to connect to a s3 server on " + str(hostname))
sock.connect((hostname, 4096))

print("Client port number: " + str(sock.getsockname()[1]) + "\n")

file_name = "../../../root/Arshdeep.Jpeg"
username = "arshdeep"
password = "SomeSecretKeyWhichIsReallyLong"
# accept and send are not action types but response action types
action_types = ["create", "update", "delete",
                "request", "server"]
extras_update = "updateat=0\ntrim=1\n"
extras_common = "filename=" + str(file_name) + "\n"
input_file = "./file"


def open_file(file_name):
    file = open(file_name, 'rb')
    file_len = os.stat(file_name).st_size
    return file, file_len


def response_status(response):
    response_code = response.get_headers()["response"]

    print("Server response code is : " + str(response_code))
    print("Server response length : " +
          str(len(response.get_data_byte_array())))
    if (response_code != 4):
        print("Server response said : " + str(response.get_data_string()))
    return response_code


while (True):

    print("Please select of the options to continue: ")
    print("1. Send the file\n"
          "2. Send a file update\n"
          "3. Delete the file\n"
          "4. Request the file\n"
          "5. Request server info\n"
          "Ctrl + C Exit client\n")

    selection = input()  # wait for the client to press enter before sending the packet
    file_reader, file_length = open_file(input_file)
    extra_headers = extras_common

    data_length = 0
    if (selection == "1"):
        selected_action = action_types[0]
        data_length = file_length
    elif (selection == "2"):
        selected_action = action_types[1]
        data_length = file_length
        extra_headers += extras_update
    elif (selection == "3"):
        selected_action = action_types[2]
    elif (selection == "4"):
        selected_action = action_types[3]
    elif (selection == "5"):
        selected_action = action_types[4]
    else:
        print("Not a valid option\n")
        continue

    data_send = PacketResponseWriter(
        sock, selected_action, data_length, username, password, extra_headers)
    # now we have send the request to the server, and must wait for the server response

    response = PacketResponseReader(sock)
    response_code = response_status(response)

    if (response_code == 1):
        PacketResponseWriter.send_data_with_confirm(sock, file_reader.read())

        response = PacketResponseReader(sock)
        response_code = response_status(response)
        if (response_code == 2):
            print("Data sent")
        else:
            print("Server rejected the data")

    elif (response_code == 3):
        PacketResponseWriter.accept_incoming_data(sock)

        print("\nServer should be sending data now:\n")
        response = PacketResponseReader(sock)
        response_code = response_status(response)

        if (response_code == 4):
            if (response.file != None):
                os.rename(response.file.name, os.path.basename(file_name))
            print(response.file.name)
            print(os.path.basename(file_name))

    print("****")
