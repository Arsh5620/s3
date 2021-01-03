# This is client program for the chat server, please refer to the documentation for the server program.

import socket
import time
import ssl
import os
import sys
parentdir = os.path.abspath("../server/ssbs/")  # noqa
sys.path.insert(0, parentdir)  # noqa
# pylint: disable=import-error
from python import serializetypes  # noqa
# pylint: disable=import-error
from python import serialize  # noqa
from python import deserialize  # noqa
import inspect

sock = socket.socket()

hostname = "127.0.0.1"

print("Do you want to enable ssl?")

# want_ssl = input()
want_ssl = False

ssock = 0
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)

if (want_ssl == "Y"):
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    context.check_hostname = False
    context.verify_mode = ssl.CERT_NONE
    ssock = context.wrap_socket(sock, server_hostname=hostname)
else:
    ssock = sock

print("Client started, and will try to connect to s3 server")

ssock.connect((hostname, 4096))

print("The sock port number is : " + str(ssock.getsockname()[1]))
print("")
print("Connected to the server")
print("")


class PacketResponseReader:
    dictionary = dict()
    return_data = ""
    data_length = 0

    def __init__(self):
        recv_packet_header = ssock.recv(8)

        recv_packet_magic = recv_packet_header[7]
        recv_header_size = recv_packet_header[6] * 16
        recv_data_size = recv_packet_header[0:5]

        if (recv_packet_magic != 0xD0):
            return

        print("Server response received")

        recv_data_size_int = int.from_bytes(recv_data_size, byteorder='little')
        recv_header_keys = ssock.recv(recv_header_size)
        recv_data = ssock.recv(recv_data_size_int)
        self.data_length = recv_data_size_int

        header_list = deserialize.deserialize_all(recv_header_keys)
        self.return_data = recv_data

        for pair in header_list:
            self.dictionary[pair.key] = pair.value

    def getDictionary(self):
        return(self.dictionary)

    def getData(self):
        return(self.return_data.decode("ascii"))

    def getDataBinary(self):
        return (self.return_data)

    def getDataLength(self):
        return(self.data_length)


def binary_serialize(pair_string):
    pairs = pair_string.split("\n")
    serialize_memory = bytearray()
    for pair in pairs:
        key_value = pair.split("=")
        print(key_value)
        if (len(key_value) >= 2):
            serialize.serializer_add(
                serialize_memory, key_value[0], key_value[1] if not is_int(key_value[1]) else int(key_value[1]))
    serialize.serializer_add_eof(serialize_memory)
    return serialize_memory


def is_int(s):
    try:
        int(s)
        return True
    except ValueError:
        return False


while(1):
    magic0 = 0xD010000000000000
    magic_invalid0 = 0x0008000000000000
    magic_invalid1 = 0x00D1000000000000

    file_name_1 = "root/sdcard/storage/DCIM/Arshdeep.Jpeg"
    auth = "username=arshdeep\nsecret=SomeSecretKeyWhichIsReallyLong\n"

    key_value_pairs = ("action=create\n"
                       "filename=" + file_name_1 + "\n"
                       "crc=00565423\n" + auth)
    key_value_pair_nonparseable = ("==create\n"
                                   "filename~ " + file_name_1 + "\n"
                                   "folder\n"
                                   "crc=jkhkjhkk\n")
    key_value_pair_onlykeys = ("action=create\n"
                               "filename=\n"
                               "folder=\n"
                               "crc=1234\n")
    key_value_pair_update = ("action=update\n"
                             "filename=" + file_name_1 + "\n"
                             "updateat=20000\n"
                             "trim=0\n" + auth)
    key_value_pair_delete = ("action=delete\n"
                             "filename=" + file_name_1 + "\n" + auth)
    key_value_pair_request = ("action=request\n"
                              "filename=" + file_name_1 + "\n" + auth)

    key_value_pair_thin = ("action=create\n")
    key_value_pair_invalidaction = ("action=whatever\n")
    key_value_pair_empty = ("\n")
    key_value_pair_server = "action=server\n"

    print("Please select of the options to continue: ")
    print("1. Send corrupted packet with incorrect magic\n"
          "2. Send corrupted data packet with incorrect magic\n"
          "3. Send non-parse-able headers key/value pairs\n"
          "4. Send empty header key/value pairs\n"
          "5. Send invalid action key value pair\n"
          "6. Send thin attribs for the action\n"
          "7. Send correct/valid packet\n"
          "8. Send packet with keys only, and no values\n"
          "9. Send file update packet\n"
          "A. Delete the file\n"
          "B. Request the file\n"
          "C. Request server info\n"
          "Ctrl + C Exit client\n")

    data_entered = input()  # wait for the client to press enter before sending the packet

    magic_packet = magic0
    magic_data = "SEND"
    header_pairs = key_value_pairs
    if (data_entered == "1"):
        magic_packet = magic_invalid0
    elif (data_entered == "2"):
        magic_data = "NOSEND"
    elif (data_entered == "3"):
        header_pairs = key_value_pair_nonparseable
    elif (data_entered == "4"):
        header_pairs = key_value_pair_empty
    elif (data_entered == "5"):
        header_pairs = key_value_pair_invalidaction
    elif (data_entered == "6"):
        header_pairs = key_value_pair_thin
    elif (data_entered == "7"):
        # do nothing, this is the default
        header_pairs = key_value_pairs
    elif (data_entered == "8"):
        header_pairs = key_value_pair_onlykeys
    elif (data_entered == "9"):
        header_pairs = key_value_pair_update
    elif (data_entered == "A"):
        header_pairs = key_value_pair_delete
    elif (data_entered == "B"):
        header_pairs = key_value_pair_request
    elif (data_entered == "C"):
        header_pairs = key_value_pair_server
    else:
        print("Not a valid option\n")
        continue

    filename = './file'
    fin = open(filename, 'rb')  # open file one.pdf to send

    file_len = os.stat(filename).st_size

    if (data_entered != "A" and data_entered != "B" and data_entered != "C"):
        magic_packet += file_len

    header_keys = binary_serialize(header_pairs)
    header_keys += (' ' * (256 - len(header_keys))).encode()

    ssock.send(magic_packet.to_bytes(8, byteorder="little"))
    ssock.send(header_keys)

    # now we have send the request to the server, and must wait for the server response

    packet_response = PacketResponseReader()
    response_data_code = packet_response.getDictionary()["response"]
    print("Server response code is : " + str(response_data_code))
    print("Server response said : " + packet_response.getData())
    if (response_data_code == 1):
        ssock.send(magic_data.encode("ascii"))
        ssock.send(file_len.to_bytes(8, byteorder=sys.byteorder))
        file_data = fin.read()

        ssock.send(file_data)
        print("Server data sent")
        packet_response2 = PacketResponseReader()
        print("")
        print("Response for data received")
        data_response_code = packet_response2.getDictionary()["response"]
        print("Response code for data sent : " + str(data_response_code))
        print("Response data for data sent : " + packet_response2.getData())
        if (data_response_code == 2):
            print("Server accepted the data that was sent")
        else:
            print("Server rejected the data that was sent")
    elif (response_data_code == 3):
        ssock.send("ACCEPT\0\0".encode("ascii"))
        print("Server is sending data now:")
        packet_data = PacketResponseReader()
        data_response_code = packet_data.getDictionary()["response"]
        print("Response code for data sent : " + str(data_response_code))
    elif (response_data_code == 4):
        binary_file = open("binary", "wb")
        binary_file.write(packet_response.getDataBinary())
        binary_file.flush()
        binary_file.close()
        pass
    print("****")
