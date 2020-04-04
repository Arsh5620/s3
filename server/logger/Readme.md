# Memory Debug information

### Problem Statement:
We want to be able to log program information as sometime this information will be useful for us when debugging the program. It must be logged as easily readable file(s). 

### How we solve it:
We are going to implement our own logging API. It will be based upon *svprintf* to allow for printf like string output. 

### What needs to be done (TODO): 
Nothing!

### Technical Details:
When the program first start it must initialize the logging sub-system. The logging system will try to open the file, creating the file and the directory if they don't exists. If the program is unable to open the file/directory due to any reason it will fallback to writing all the logging to stdio/stderr.
 
**Example:** 
1. Make a call to **logs_open()** to initialize the logging system. 
2. Then make calls to **logs_write()** as required. *(refer to logs.h for more information)*
3. Then once you are done using, finally call **logs_close()** to close all the open files after flushing.

The logging subsystem does allow for multiple levels of importance as:
* **LOGGER_INFO** : 
Logging is only done for informational purpose, information such as program execution milestones. e.g. client connection information (IP, port).

* **LOGGER_DEBUG** : 
Logging that will be only on debug machine, required for developers only, and sysadmin or end users can ignore this logging type.

* **LOGGER_WARN** :
Information that will help the sysadmin/end users that will be related to misconfiguration or some issues that might need to be addressed.

* **LOGGER_ERROR** :
Messages that might be more serious and requires immediate admin/user intervention. The program will still run, the issues might prevent regular execution flow.

* **LOGGER_CATASTROPHIC** :
Issues that result in program termination, issues such as not being able to create new socket for the port number provided. 

### More Information: 

Not applicable, will update :)
