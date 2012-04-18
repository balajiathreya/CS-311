#!/usr/bin/python 

import socket
import select
import string
import struct
import sys
import math

def print_usage():
	print "Usage: " + str(sys.argv[0]) + " PORT_NO";
	sys.exit()

open_sockets = []
processes = []
perfects = []
cur_max_range = 5
endval = 0

result = 0
perf_char = 0
msg_type = 0
n = 0
tb = 0
UINT_MAX = 4294967295
compute_port = 8989

if(len(sys.argv) > 1):
	try:
		port = int(sys.argv[1])
	except ValueError:
		print_usage()
else:
		print_usage()


listening_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM )
listening_socket.bind(('', port))
listening_socket.listen(5)



while True:
	rlist, wlist, xlist = select.select( [listening_socket] + open_sockets, [], [] )
	for connection in rlist:
		if connection is listening_socket:
			new_socket, addr = listening_socket.accept()
			if(not addr[0] in processes):
				processes.append(addr[0])
			open_sockets.append(new_socket)
		else:
			total_data=[]
			data=''
			msg=''
			data = connection.recv(4069)
			while data:
				total_data.append(data)
				data = connection.recv(4069)
			msg = string.join(total_data, '')
			connection.shutdown(0)    
			if msg[:1] == chr(273):
				print "signal-"
				connection.sendall(struct.pack('!cLLL',chr(276), cur_max_range, len(repr(perfects)), len(repr(processes))) + repr(perfects) + repr(processes))
				connection.close()
				print "Terminating Server."
				for process in processes:
					new_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
					new_connection.connect((process,compute_port))
					new_connection.sendall(chr(279))
					new_connection.shutdown(1)
					new_connection.close()
				sys.exit()
			elif msg[:1] == chr(274):
				print "request/result-"
				msg_type, perf_char, result = struct.unpack('!cLL', msg)
				if perf_char != 0:
					if cur_max_range < UINT_MAX:
						endval = int(math.sqrt((cur_max_range+1)**2+(2*(1500*perf_char))))
						print cur_max_range
						print endval
						connection.sendall(struct.pack('!cLL',chr(277),cur_max_range+1, endval))
						cur_max_range = endval
					else:
						connection.sendall(struct.pack('!cLL',chr(277),0, 0))
				connection.shutdown(1)
				connection.close()
				if result != 0:
					print "Recieved: " + str(result)
					perfects.append(result)
				perf_char = 0
			elif msg[:1] == chr(275):
				print "report-"
				connection.sendall(struct.pack('!cLLL',chr(278), cur_max_range, len(repr(perfects)), len(repr(processes))) + repr(perfects) + repr(processes))
				connection.shutdown(1)
				connection.close()
			else:
				print "invalid data"
			msg = ''
			open_sockets.remove(connection)
			print "Connection closed"
