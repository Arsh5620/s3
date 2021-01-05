import os
import sys
import struct
parentdir = os.path.abspath("../server/ssbs/")  # noqa
sys.path.insert(0, parentdir)  # noqa
# pylint: disable=import-error
from python import serializetypes  # noqa
# pylint: disable=import-error
from python import deserialize  # noqa


class PacketResponseReader:
    dictionary = dict()
    file = None
    buffer = None

    def __init__(self, socket):
        magic_header = socket.recv(8)

        if magic_header[7] != 0xD0:
            return

        header_size = magic_header[6] * 16

        size_array = magic_header[0:6]
        data_size = int.from_bytes(size_array, byteorder='little')

        print(str(size_array))
        print("Server response received")

        binary_header = socket.recv(header_size)
        header_list = deserialize.deserialize_all(binary_header)

        if (data_size > 1000):
            self.file = open("download.tmp", "wb+")
        else:
            self.buffer = bytearray()

        downloaded_size = 0
        while (downloaded_size != data_size):
            data = socket.recv(data_size - downloaded_size)
            if self.file != None:
                self.file.write(data)
            else:
                self.buffer += data

            downloaded_size += len(data)

        if (self.file != None):
            self.file.flush()

        self.length = data_size

        for pair in header_list:
            self.dictionary[pair.key] = pair.value

    def get_headers(self):
        return (self.dictionary)

    def get_data_internal(self):
        if self.file != None:
            return self.file.read()
        else:
            return self.buffer

    def get_data_string(self):
        return(self.get_data_internal().decode("ascii"))

    def get_data_byte_array(self):
        return (self.get_data_internal())

    def get_data_length(self):
        return(self.length)
