############################################################
# References	
############################################################	
# - https://wiki.python.org/moin/TcpCommunication		
# - https://picamera.readthedocs.io/en/release-1.12/recipes1.html
############################################################

############################################################
# Author:		Jameson Weber
# Date:			January 5 2017
# Description:	Central Program on the MULE. 
#				This program is to deal with the video capture of the picamera
############################################################
# Modification Log
############################################################
# Author:		
# Date:			
# Description:	
############################################################

import socket
import sys
import time
from io import BytesIO
from picamera import PiCamera


def main():
	if len(sys.argv) != 3:
		print("No connection parameters povided, setting connection to defaults")
		host = "localhost"
		port = 8080
	else:
		host = sys.argv[1]
		port = int(sys.argv[2])
	buff = 1024
	print("Starting Central Program Video")
	print("Attempting to open server [" + host + "] at port " + str(port))

	runServer(host, port)

# Function to deal with client sending information to server
def runServer(host, port):
	#TCP Code
	#s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	#s.bind((host, port))
	#s.listen(0)
	#conn = s.accept()[0].makefile('wb')

	#UDP Code
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	s.connect((host, port))
	conn = s.makefile('wb')

	print("Server Started")
	camera = PiCamera()
	print("PiCamera Initialized")
	
	camera.resolution = (640, 480)
	camera.framerate = 24
	camera.start_recording(conn, format='h264', quality=40)
	camera.wait_recording(30)
	camera.stop_recording()

	conn.close()
	s.close()



# Call main function
if __name__ == "__main__":
	main()

