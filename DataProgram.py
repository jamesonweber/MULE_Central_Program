############################################################
# References	
############################################################	
# - https://wiki.python.org/moin/TcpCommunication		
#
############################################################

############################################################
# Author:		Jameson Weber
# Date:			Feburary 23 2017
# Description:	Data Program on the MULE. 
#				This program is to deal with the output of the sensors
############################################################
# Modification Log
############################################################
# Author:		
# Date:			
# Description:	
############################################################


# All required imports
import sys
import socket
import serial
import os
import sys


# Main function definition
def main():
	if len(sys.argv) != 3:
		print("No connection parameters povided, setting connection to defaults")
		host = "localhost"
		port = 8889
	else:
		host = sys.argv[1]
		port = int(sys.argv[2])
	buff = 1024
	print("Starting Data Program")
	print("Attempting to open server [" + host + "] at port " + str(port))
	while 1:
		runServer(host, port, buff)
	

# Function to deal with client sending information to server
def runServer(host, port, buff):
#	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#	s.bind((host, port))
#	s.listen(1)
	print("Server Started")
#	conn, addr = s.accept()
	ser = serial.Serial('/dev/ttyACM0', 9600)
	print("Module Arduino Connected")
	try: 
		while 1:
			data = ser.readline()
			print("Received data: " + str(data))
#		conn.close()
	except serial.error:
		print("Surface Controller Disconnected")
#		conn.close()


# Call main function
if __name__ == "__main__":
	main()
