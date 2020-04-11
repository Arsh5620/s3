This readme txt file will try to explain how the program internally works. 

Setup and Initilization:
The first thing that the program does is the Initilization of the network and other parameters. 
Here are some of the things that the program must assert or it must exit with an error. 
First of all we must be able to secure the required ports and open connections on said ports. 
For now we are only securing two ports, same as FTP the first port is for communication and the 
second port is for data transfer, and the data transfer size must also be set via communication
port. 
But before we go ahead with setting up ports, we must also try to initialize our logging system. 
we must log all the errors to disk as we are working, so that we can use these errors later to
detect any errors and debug them. But unlike port/IO if logging init to a file fails, we must
keep the program going but must then switch over to logging only critical information to stdio/
stderr.
After this is done, since we will be using database system mariaDB to keep track of our files
we will then need to init the mariaDB connection. If the connection setup fails, the program 
must exit and the error information must be written to the logging system. If it succeeds, then 
must try to switch over to the database that we are using with the application, and if the 
database does not exists which is the case if we are starting the program for the first time, 
then must initialize the database. After the database is selected, we will check for if the
table entries that we require exists, like database if the table entries does not exits we will
create them, and initialize them as required. 
For us to be able to connect to our database system, since on each PC we will need to have 
different configuration, we must also have a configuration reader. Which is config.c, it will 
read the file and parse the configuration by using parser for key:value pairs. This is the same
parser that we use to parse the headers for the incoming network IO. 
To make memory debugging easier, the debug version of the program must also include the 
memory.c file and use memory allocation functions appended with "m_". These functions simply
inline to the actual functions when using the release version of the program. 
To implement the memory.c we are using two different data structures namely list and hash table
, and the list is used to store all the allocations that take place in the program. table is
used to only store the key (in this case the address of the allocation), and the list index 
of said allocation. This should somehow help with the allocation or memory leaks as we are 
also tracking the filename and the line number where allocations take place. Along with that
if we modify an allocation such as by realloc or free. It will update the actual allocation. 

Now more about how the network IO is dispatched. 
So first we receive a network packet, we will call this network packet PACKET_A. This packet
consists of the entire information, which includes header, padding, and data. We will need to 
break down this packet and download as required. Now first of all the first thing that we need 
to read will be the packet header, in our case that packet header starts with an 8 byte header. 
You can read more about headers in server/networking/PROTOCOL, but basically the first eight 
bytes are responsible for the {size of the packet, size of the header, and the magic byte}.
We are only using one magic byte but the magic byte instead of being the first byte of the
packet, it is the eighth byte of the eight length header. So the first eight bytes of the
header are really important and thus serve a lot of function. After the first eight bytes are 
processed we now know a lot about what we should be expecting from our network packet. Again
the header must be a multiple of 16, and cannot be more than 4KB long, and the rest of the 6
bytes can then be used to represent the actual size of the data that we should be expecting. 
And now after we know the size of the header we should be expecting, we can now read the header. 
So we allocate the amount of memory required for this header, and then we parse this header 
into key value pairs. The first key value pair that any header should have for it to be 
considered valid is the "action" key. similar to http, where the first key value pair should 
always be either GET, or POST or one other other accepted actions. And once we have the value 
for the "action", we can then check the type of action that the client is requesting. Based
on this we will then have to check what key value pairs are required for the request to be 
processed, and then we have to assert the check for those key value pairs. Until now we have
not downloaded the data that the packet might have, so the first thing we need to do is to 
call the function related to the action that the client is requesting to confirm that all 
the information we have until now is enough or is correct before we proceed with downloading 
the data, if it is not correct or we don't want to accept data, we will communicate this to 
the client and we will not open the channel for downloading the data. And after we have 
downloaded the data that the client sent us we will then again call the function related 
to the action that will handle the data for us, what ever the actions needs to be taken with 
the data. The data that we will download will always be downloaded to a file in a temporary
location to be saved for later use.

*** How dbp_accept_connection_loop handles further delegation.
When we receive a connection request, we will wait for to read the header via blocking request. 
We will block to read the first 4 bytes of the header. 
We will then read the actual header of the request, but at this time if there is an issue
with the header request itself.