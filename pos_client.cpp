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
#include "RTIMULib/RTIMULib.h"

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
    
    //init IMUs with individual calibration files
    RTIMUSettings *settings1 = new RTIMUSettings("RTIMULib1");
    RTIMUSettings *settings2 = new RTIMUSettings("RTIMULib2");
    
    RTIMU *imu1 = RTIMU::createIMU(settings1);
    RTIMU *imu2 = RTIMU::createIMU(settings2);
    
    if ((imu1 == NULL) || (imu1->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU1 found\n");
        exit(1);
    }
    if ((imu2 == NULL) || (imu2->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU2 found\n");
        exit(1);
    }
    
    imu1->IMUInit();
    imu2->IMUInit();
    
    imu1->setSlerpPower(0.02);   //Nick, what does this do?
    imu1->setGyroEnable(true);
    imu1->setAccelEnable(true);
    imu1->setCompassEnable(true);
    
    imu2->setSlerpPower(0.02);
    imu2->setGyroEnable(true);
    imu2->setAccelEnable(true);
    imu2->setCompassEnable(true);
    
    RTIMU_DATA imuData1;
    RTIMU_DATA imuData2;
    
    while(1) {
        //slow things down so we're polling at the rate the IMUs enjoy most
        usleep(imu2->IMUGetPollInterval() * 1000);
        
        /* ben code to get pos */
        //this first set takes the measurements from the IMUs and updates imuData privately
        //This also performs fusion filter processing. We can comment out that line (updateFusion();) in RTIMULSM9DS1.cpp if Nick needs more control
        imu1->IMURead();
        imu2->IMURead();
        
        //This second set makes the data available to the user
        imuData1 = imu1->getIMUData();
        imuData2 = imu2->getIMUData();
        
        
        /* nick calc code */
        
        /* convert to string code */
        
        /* call this to send across network */
        strcpy(sendBuff, "126.0,23.54,15.4");
        //write(sockfd, sendBuff, strlen(sendBuff));
    }
    
    close(sockfd);
    
    return 0;
    
}

