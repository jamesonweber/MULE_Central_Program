////////////////////////////////////////////////////////////////////////////
//
//  This file is part of RTIMULib
//
//  Copyright (c) 2014-2015, richards-tech, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


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
#include "RTIMULib.h"
#include <iostream>
extern "C"
{
#include <gps.h>
}

void getPos(int sockfd) {
    gps_init();
    loc_t data;
    gps_location(&data);
    double latitude = data.latitude;
    double longitude = data.longitude;

    
    char sendBuff[1024];
    
    int sampleCount = 0;
    int sampleRate = 0;
    uint64_t rateTimer;
    uint64_t displayTimer;
    uint64_t now;
    
    //  Using RTIMULib here allows it to use the .ini file generated by RTIMULibDemo.
    //  Or, you can create the .ini in some other directory by using:
    //      RTIMUSettings *settings = new RTIMUSettings("<directory path>", "RTIMULib");
    //  where <directory path> is the path to where the .ini file is to be loaded/saved
    
    RTIMUSettings *settings1 = new RTIMUSettings("RTIMULib1");
    
    RTIMU *imu1 = RTIMU::createIMU(settings1);
    
    if ((imu1 == NULL) || (imu1->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU1 found\n");
        exit(1);
    }
    
    //  This is an opportunity to manually override any settings before the call IMUInit
    
    //  set up IMU
    
    imu1->IMUInit();
    
    //  this is a convenient place to change fusion parameters
    RTIMUSettings *settings2 = new RTIMUSettings("RTIMULib2");
    
    RTIMU *imu2 = RTIMU::createIMU(settings2);
    
    if ((imu2 == NULL) || (imu2->IMUType() == RTIMU_TYPE_NULL)) {
        printf("No IMU2 found\n");
        exit(1);
    }
    
    //  set up IMU
    
    imu2->IMUInit();
    
    //  set up for rate timer
    
    rateTimer = displayTimer = RTMath::currentUSecsSinceEpoch();
    
    //  now just process data
    RTFusionKalman4 * fusion = new RTFusionKalman4;
    
    
    while (1) {
        //  poll at the rate recommended by the IMU
        
        usleep(imu1->IMUGetPollInterval() * 1000);
        

            imu1->IMURead();
            RTIMU_DATA imuData1 = imu1->getIMUData();
            imu2->IMURead();
            RTIMU_DATA imuData2 = imu2->getIMUData();
            sampleCount++;
            
            now = RTMath::currentUSecsSinceEpoch();
            
            //  display 10 times per second
            
            if ((now - displayTimer) > 100000) {
                //                printf("Sample rate %d: %s\r", sampleRate, RTMath::displayDegrees("", imuData.fusionPose));
                //                fflush(stdout);
                //                displayTimer = now;
                //            }
                
                //RTVector3 accel_resid1= imu1->getAccelResiduals();
                //RTVector3 accel_resid2= imu2->getAccelResiduals();
                fusion->newIMUData(imuData1, imuData2, settings1, sockfd, latitude, longitude);
                //fusion->LSE(imuData1, imuData2, accel_resid1, accel_resid2);
                fflush(stdout);
            }
            
            //  update rate every second
            
            if ((now - rateTimer) > 1000000) {
                sampleRate = sampleCount;
                sampleCount = 0;
                rateTimer = now;
            }
    }
}

int main()
{
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
    
    
    
    getPos(sockfd);
    
    close(sockfd);
    
    return 0;
}



