############################################################
# References	
############################################################	
# - https://wiki.python.org/moin/TcpCommunication		
#
############################################################

############################################################
# Author:		Jameson Weber
# Date:			December 12 2016
# Description:	Central Program on the MULE. 
#				This program is to deal with the input / ouput of controls to the user
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

# Main function definition
def main():
	if len(sys.argv) != 3:
		print("No connection parameters povided, setting connection to defaults")
		host = "localhost"
		port = 8888
	else:
		host = sys.argv[1]
		port = int(sys.argv[2])
	buff = 1024
	print("Starting Central Program")
	print("Attempting to open server [" + host + "] at port " + str(port))

	runServer(host, port, buff)

# Function to deal with client sending information to server
def runServer(host, port, buff):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((host, port))
	s.listen(1)
	print("Server Started")
	conn, addr = s.accept()
	ser = serial.Serial('/dev/tty.usbserial', 9600)
	print("Controller Ardunio Connected")
	while 1:
		data = conn.recv(buff)
		if not data: break
		print("Received Controls: " + str(data))
		ser.write(data)

	conn.close()



# Call main function
if __name__ == "__main__":
	main()