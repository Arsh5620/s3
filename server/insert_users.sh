#!/bin/bash

database_name=$1

if [ -f $database_name ];
    then sqlite3 $database_name "insert into authentication values('arshdeep','b4a078299bd030306d61e5cea8a3bfa612cc268b72f739dca604c230546e90d5');" && echo "Successfully added user";
fi