############################################################
# References	
############################################################	
# - https://wiki.python.org/moin/TcpCommunication		
# - http://stackoverflow.com/questions/415511/how-to-get-current-time-in-python
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
#import serial
import os
import sys
from datetime import datetime as dt
import time


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
	runServer(host, port, buff)
	

# Function to deal with client sending information to server
def runServer(host, port, buff):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((host, port))
	s.listen(1)
	print("Server Started")
	conn, addr = s.accept()
#	ser = serial.Serial('/dev/ttyACM0', 9600)
	print("Module Arduino Connected")
	try: 
		while 1:
#			data = ser.readline()
			data = "DP|MULE|DISTANCE|0x1|1|CM|6.2|1.9|102.6|100,102,101,120,100,100,100,100,100,103"
			data = data + "|"
			data = data + "20.0,-158.0,16.0"
			data = data + "|"
			data = data + str(dt.now().strftime('%Y-%m-%d %H:%M:%S'))
			s.send(data.encode('utf-8'))
			print("Received data: " + str(data))
			time.sleep(0.1)
		conn.close()
	except Exception as msg:
		print(msg)
		print("Surface Controller Disconnected")
		conn.close()


# Call main function
if __name__ == "__main__":
	main()
