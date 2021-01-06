```
S3 [Super Simple Server]

This file defines the protocol itself. 

The protocol itself is quite simple. 

1. The magic header
- This is a 8 byte magic header that definess the header size and data size (if any). 
- This header is arranged as XX XX XX YZ, where X is the size of the data (6 bytes), 
- Y (1 byte) is the size of the header / 16, and Z(1 byte) is the magic '0xD0'
- The header is intentionally in reverse order

2 The key value header
- This header contains action information for the packet,
- The header is binary serialized with https://github.com/Arsh5620/ssbs
- Other than the action type itself all the other values are optional

3 Data
- After the packet is sent, the client must wait for the server's response
- The server will reply will response code S3_RESPONSE_DATA_SEND(1) when
- the server is expecting the client to send the data
- When sending data the client must send "DATA"(int:0x41544144) appended 
- by the 8 byte length of the data followed by the data
- When the server is ready to send more data to the client, the server will
- send response code S3_RESPONSE_PACKET_DATA_MORE(3) and the server will then 
- wait for the client to reply with "ACCEPT\0\0" before the server will start sending data

Stateless connection
- The protocol is stateless, that is no information about the client's last request is saved

Threading support:
- Currently the server runs in single thread mode with only blocking calls, implenting
- non-blocking IO with multithreading support is the plan for the future
```
