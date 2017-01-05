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
from io import BytesIO
#from picamera import PiCamera


def main():
	if len(sys.argv) != 3:
		print("No connection parameters povided, setting connection to defaults")
		host = "localhost"
		port = 8888
	else:
		host = sys.argv[1]
		port = int(sys.argv[2])
	buff = 1024
	print("Starting Central Program Video")
	print("Attempting to open server [" + host + "] at port " + str(port))

	runServer(host, port, buff)

# Function to deal with client sending information to server
def runServer(host, port, buff):
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((host, port))
	s.listen(1)
	print("Server Started")
	conn, addr = s.accept()
	stream = b"hello world"
	#camera = PiCamera()
	#camera.resolution = (640, 480)
	#camera.start_recording(stream, format='h264', quality=23)
	#camera.wait_recording(5)
	#camera.stop_recording()
	conn.send(stream)

	conn.close()



# Call main function
if __name__ == "__main__":
	main()

