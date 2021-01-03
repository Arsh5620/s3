import os
import sys
parentdir = os.path.abspath("../server/ssbs/")  # noqa
sys.path.insert(0, parentdir)  # noqa
# pylint: disable=import-error
from python import serializetypes  # noqa
# pylint: disable=import-error
from python import deserialize  # noqa


class PacketResponseReader:
    dictionary = dict()

    def __init__(self, socket):
        magic_header = socket.recv(8)

        if magic_header[7] != 0xD0:
            return

        header_size = magic_header[6] * 16
        data_size = int.from_bytes(magic_header[0:5], byteorder='little')

        print("Server response received")

        binary_header = socket.recv(header_size)
        data = socket.recv(data_size)
        self.length = data_size

        header_list = deserialize.deserialize_all(binary_header)
        self.data = data

        for pair in header_list:
            self.dictionary[pair.key] = pair.value

    def getHeaders(self):
        return(self.dictionary)

    def getDataString(self):
        return(self.data.decode("ascii"))

    def getDataByteArray(self):
        return (self.data)

    def getDataLength(self):
        return(self.length)
