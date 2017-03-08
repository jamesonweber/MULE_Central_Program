//
//  nav.h
//  
//
//  Created by Benjamin Hilborn on 2017-02-23.
//
//

#ifndef nav_h
#define nav_h

#include "RTIMULib/RTIMULib.h"
#include <stdio.h>
#include <gps.h>
#include <sys/types.h>
#include <libgpsmm.h>

static RTIMU *imu1;
static RTIMU *imu2;
static RTIMU *imu3;

static RTIMU_DATA imu1Data;
static RTIMU_DATA imu2Data;
static RTIMU_DATA imu3Data;

void setup();
int main();
RTIMU_DATA sampleIMU(RTIMU imu);

#endif /* nav_h */
