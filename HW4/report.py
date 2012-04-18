#!/usr/bin/python 

import socket
import select
import string
import struct
import sys


def print_usage():
	print "Usage: " + str(sys.argv[0]) + " hostname port_no [-k]"
	sys.exit();

msg_type = 0
cur_max_range = 0
perfects_len = 0
processes_len = 0
kill = 0


size = 1024
total_data=[]
data=''
msg=''


if(len(sys.argv) == 4 and str(sys.argv[3]) == "-k"):
        try:
		kill = 1
		host = sys.argv[1]
                port = int(sys.argv[2])
        except ValueError:
		print_usage()
elif(len(sys.argv) == 3):
	try:
                host = sys.argv[1]
                port = int(sys.argv[2])
        except ValueError:
		print_usage()
else:
		print_usage()

connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
connection.connect((host,port)) 


connection.sendall(chr(275))

connection.shutdown(1)

data = connection.recv(4069)
while data:
	total_data.append(data)
	data = connection.recv(4069)
msg = string.join(total_data, '')
if msg[:1] == chr(278):
	msg_type, cur_max_range, perfects_len, processes_len = struct.unpack('!cLLL', msg[:13])
	print "cur_max _range: " + str(cur_max_range)
	print "perfects: " + msg[13:13+perfects_len]
	print "processes: " + msg[13+perfects_len:13+perfects_len+processes_len]
connection.close()
print data

if kill:
	connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	connection.connect((host,port))
	print "terminating servers and client"
	connection.sendall(chr(273))
	connection.shutdown(1)
	connection.close()
