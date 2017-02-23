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
import threading
import os
import sys
import subprocess

# Multithreading class for running the picamera
#class cameraThread(threading.Thread):
#	def __init__(self, threadID, name):
#		threading.Thread.__init__(self)
#		self.threadID = threadID
#		self.name = name
#	def run(self):
#		print("Starting PiCamera Stream at port 8090")
#		os.system("raspivid -o - -t 0 -hf -w 640 -h 360 -fps 25|cvlc -vvv stream:///dev/stdin --sout '#standard{access=http,mux=ts,dst=:8090}' :demux=h264")


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
	#ct = cameraThread(1, "cameraThread")
	#ct.setDaemon(True)
	#ct.start()
	ct = subprocess.Popen("raspivid -o - -t 0 -hf -w 640 -h 360 -fps 25|cvlc -vvv stream:///dev/stdin --sout '#standard{access=http,mux=ts,dst=:8090}' :demux=h264")
	runServer(host, port, buff)
	ct.terminate()
	sys.exit()

# Function to deal with client sending information to server
def runServer(host, port, buff):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((host, port))
	s.listen(1)
	print("Server Started")
	conn, addr = s.accept()
	ser = serial.Serial('/dev/ttyUSB0', 115200)
	print("Controller Ardunio Connected")
	try: 
		while 1:
			data = conn.recv(buff)
			if not data: break
			print("Received Controls: " + str(data))
			ser.write(data)
		conn.close()
	except socket.error:
		print("Surface Controller Disconnected")
		conn.close()


# Call main function
if __name__ == "__main__":
	main()
