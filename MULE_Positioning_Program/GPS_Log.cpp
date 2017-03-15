#include <libgpsmm.h>
#include "GpsPosition.h"
#include "MutexLocker.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

using namespace std;

GPSPosition GPSPosition::m_slast_3d_value(NULL);
GPSPosition GPSPosition::m_slast_2d_value(NULL);
std::mutex GPSPosition::m_slock;

//Controlling the thread flow
bool gReadGps = true;

/// Thread which reads all position form the gpsd
/// server
void read_gpsd_data() {
    gpsmm gps_rec("localhost", DEFAULT_GPSD_PORT);
    
    if (gps_rec.stream(WATCH_ENABLE|WATCH_JSON) == NULL) {
        cerr << "No GPSD running.\n";
        return;
    }
    
    while (gReadGps) {
        struct gps_data_t *newdata;
        
        if (!gps_rec.waiting(50000000))
            continue;
        
        if ((newdata = gps_rec.read()) == NULL) {
            cerr << "Read error.\n";
            return;
        } else {
            GPSPosition dummy(newdata);       
        }
    }   
}

void main(){
    thread gps_thread(read_gpsd_data);
    ofstream GPS_Log;
    while(1)
    {
        this_thread::sleep_for(chrono::milliseconds(500));
        GPSPosition last_pos = GPSPosition::get_last_3d_fix_value();
        if (GPSPosition::Get_mode() == GPSMODE_3D_FIX){
            //open file
            GPS_Log.open("GPS_Log.csv");
            //write
            GPS_Log << last_pos.get_latitude << "," << last_pos.get_longitude << ","
                    << std::chrono::steady_clock::now();
                    << endl;
            //close file
            GPS_Log.close();
            this_thread::sleep_for(chrono::milliseconds(5000));
        }

    }
    //gReadGps = false;
    //gps_thread.join();
    //return 0;
}