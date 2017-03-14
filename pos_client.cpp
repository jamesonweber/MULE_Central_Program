//
//  pos_client.cpp
//  
//
//  Created by Jameson Weber on 2017-03-14.
//
//  https://www.codeproject.com/Articles/586000/Networking-and-Socket-programming-tutorial-in-C

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int main(void) {
    
    int sockfd = 0,n = 0;
    char sendBuff[1024];
    struct sockaddr_in serv_addr;
    
    memset(sendBuff, '0' ,sizeof(sendBuff));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8890);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    while(1) {
        
        /* ben code to get pos */
        
        /* nick calc code */
        
        /* convert to string code */
        
        /* call this to send across network */
        strcpy(sendBuff, "126.0,23.54,15.4");
        write(sockfd, sendBuff, strlen(sendBuff));
    }
    
    close(sockfd);
    
    return 0;
    
}

