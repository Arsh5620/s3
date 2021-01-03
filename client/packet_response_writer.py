import os
import sys
parentdir = os.path.abspath("../server/ssbs/")  # noqa
sys.path.insert(0, parentdir)  # noqa
# pylint: disable=import-error
from python import serializetypes  # noqa
# pylint: disable=import-error
from python import serialize  # noqa


class PacketResponseWriter:

    def __init__(self, socket, action, data_length, username, password, key_value_pairs):
        magic = bytearray()
        magic += (data_length.to_bytes(6, byteorder='little'))

        binary_header = bytearray()
        serialize.serializer_add(binary_header, "action", str(action))
        serialize.serializer_add(binary_header, "username", str(username))
        serialize.serializer_add(binary_header, "secret", str(password))

        if (key_value_pairs != None):
            self.binary_serialize(binary_header, key_value_pairs)

        padding = self.memory_align(len(binary_header)) - len(binary_header)
        binary_header += (' ' * padding).encode()
        magic.append(len(binary_header) >> 4)
        magic.append(0xD0)

        socket.send(magic)
        socket.send(binary_header)

    def binary_serialize(self, serialize_memory, key_value_pairs):
        pairs = key_value_pairs.split("\n")
        for pair in pairs:
            key_value_pair = pair.split("=")
            if (len(key_value_pair) >= 2):
                if (self.is_int(key_value_pair[1])):
                    value = int(key_value_pair[1])
                else:
                    value = key_value_pair[1]
                serialize.serializer_add(
                    serialize_memory, key_value_pair[0], value)
        serialize.serializer_add_eof(serialize_memory)

    def is_int(self, value):
        try:
            int(value)
            return True
        except ValueError:
            return False

    def memory_align(self, value):
        return value if (value & 0xF == 0) else ((value + 0XF) & ~0xF)

    @ staticmethod
    def send_data_with_confirm(socket, data):
        socket.send("DATA".encode("ascii"))
        socket.send(len(data).to_bytes(8, byteorder='little'))
        socket.send(data)

    @ staticmethod
    def accept_incoming_data(socket):
        socket.send("ACCEPT\0\0".encode("ascii"))
