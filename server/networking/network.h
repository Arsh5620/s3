#define __NETWORK__

#include <sys/socket.h>
#include <arpa/inet.h>

#define APPLICATION_PORT    4096
#define MAX_LISTEN_QUEQUE   8

enum connection_status_enum {
    NETWORK_STATUS_NOINIT = 0
    , NETWORK_STATUS_SOCKETAVAIL    = 1
    , NETWORK_STATUS_LISTENING      = 2
    , NETWORK_STATUS_CONNECTED      = 3
    , NETWORK_STATUS_READYWAIT      = 4
    , NETWORK_STATUS_READING        = 5
    , NETWORK_STATUS_WRITING        = 5
    , NETWORK_STATUS_CLOSED         = 6
    , NETWORK_STATUS_ERROR          = -1
};

// We don't want the errors, we only need the data type
#define DATA_READ_ONLY_TYPE(x) (x & 7)

// We don't want the types such as malloc or spare, we only want errors.
#define DATA_READ_NO_TYPE(x) (x & ~7)

#define DATA_READ_SUCCESS(x) (DATA_READ_NO_TYPE(x) == 0)

enum data_read_status_enum {
    DATA_READ_MALLOC    = 0b0001
    , DATA_READ_SPARE   = 0b0010
    , DATA_READ_TEMPFILE    = 0b0100
    , DATA_READ_EOF     = 0b1000
    , DATA_READ_ERROR   = 0b00010000
};


/**
 * server - server file pointer
 * client - client file pointer
 * server_socket - socket struct sockaddr_in for the server
 * client_socket - socket struct sockaddr_in for the client
 * connection_status - connection status at any given moment
 * error_code - int to indicate the error code 
 * (only indicate network connection errors, not read/write errors)
 */

typedef struct network_connection_information
{
    int server;
    int client;
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;

    enum connection_status_enum connection_status;
    int error_code;
} netconn_info_s;

typedef struct network_connection_data_read
{
    enum data_read_status_enum read_status;
    int error_code;

    void *data_address;
    int data_length;

    /*
    * If the amount of data read is lower than 8 bytes the data will be
    * read into the "spare" variable, if the amount is higher, then the 
    * data will be read into "head" allocated via malloc, and the resp-
    * -onsibility of free-ing the memory is on the caller. 
    * 
    * Also, spare will work as a regular memory buffer, so the byte order
    * will be left to right, if the network stream is "ABCDEFGHI", and 
    * we requested to read first 3 bytes, then the spare value will be
    * spare = "ABC*****";
    */
    long int spare;
} netconn_data_s;


/**
 * netconn_data_basetypes_s
 * wrapper structure for the data returned from network_data_readxbytes
 * this structure is only to make it easier for consumers to read data 
 * from TCP stream
 */
typedef struct network_connection_data_basictypes {
    netconn_data_s data_source;
    union data
    {
        char _char;
        short _short;
        int _int;
        long int _long;
    } data_u;
    enum data_read_status_enum read_status;
} netconn_data_basetypes_s;

netconn_info_s network_connect_init_sync(int);
int network_connect_accept_sync(netconn_info_s*);
void network_free(netconn_data_s conn);

#define NETWORK_DATA_READ_TYPE_DECLARATION(data_type) \
netconn_data_basetypes_s network_data_read_##data_type(netconn_info_s *conn);

NETWORK_DATA_READ_TYPE_DECLARATION(char);
NETWORK_DATA_READ_TYPE_DECLARATION(short);
NETWORK_DATA_READ_TYPE_DECLARATION(int);

/*
 * should read 8 bytes in the order that the bytes are transmitted. 
 * will perform no endian-ness conversion
 * ** will block as read is sync operation. ** 
 */
NETWORK_DATA_READ_TYPE_DECLARATION(long);

netconn_data_s network_data_readxbytes(netconn_info_s *conn, int size);
int network_connection_write(netconn_info_s *conn, char *data, int length);
void *network_netconn_data_address(netconn_data_s *data);