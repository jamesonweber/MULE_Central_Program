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


#include "RTFusionKalman4.h"
#include "RTIMUSettings.h"
#include "../Linux/include/armadillo"

using namespace std;
using namespace arma;

//  The QVALUE affects the gyro response.

#define KALMAN_QVALUE	0.001f

//  The RVALUE controls the influence of the accels and compass.
//  The bigger the value, the more sluggish the response.

#define KALMAN_RVALUE	0.0005f

#define KALMAN_QUATERNION_LENGTH	4

#define	KALMAN_STATE_LENGTH	4								// just the quaternion for the moment


RTFusionKalman4::RTFusionKalman4()
{
    reset();
}

RTFusionKalman4::~RTFusionKalman4()
{
}

void RTFusionKalman4::reset()
{
    m_firstTime = true;
    m_fusionPose = RTVector3();
    m_fusionQPose.fromEuler(m_fusionPose);
    m_gyro = RTVector3();
    m_accel = RTVector3();
    m_compass = RTVector3();
    m_measuredPose = RTVector3();
    m_measuredQPose.fromEuler(m_measuredPose);
    m_Rk.fill(0);
    m_Q.fill(0);

    // initialize process noise covariance matrix

    for (int i = 0; i < KALMAN_STATE_LENGTH; i++)
        for (int j = 0; j < KALMAN_STATE_LENGTH; j++)
            m_Q.setVal(i, i, KALMAN_QVALUE);

    // initialize observation noise covariance matrix


    for (int i = 0; i < KALMAN_STATE_LENGTH; i++)
        for (int j = 0; j < KALMAN_STATE_LENGTH; j++)
            m_Rk.setVal(i, i, KALMAN_RVALUE);
 }

void RTFusionKalman4::predict()
{
    RTMatrix4x4 mat;
    RTQuaternion tQuat;
    RTFLOAT x2, y2, z2;

    //  compute the state transition matrix
	x2 = m_gyro.x() / (RTFLOAT)2.0;
    y2 = m_gyro.y() / (RTFLOAT)2.0;
    z2 = m_gyro.z() / (RTFLOAT)2.0;

    m_Fk.setVal(0, 1, -x2);
    m_Fk.setVal(0, 2, -y2);
    m_Fk.setVal(0, 3, -z2);

    m_Fk.setVal(1, 0, x2);
    m_Fk.setVal(1, 2, z2);
    m_Fk.setVal(1, 3, -y2);

    m_Fk.setVal(2, 0, y2);
    m_Fk.setVal(2, 1, -z2);
    m_Fk.setVal(2, 3, x2);

    m_Fk.setVal(3, 0, z2);
    m_Fk.setVal(3, 1, y2);
    m_Fk.setVal(3, 2, -x2);

    m_FkTranspose = m_Fk.transposed();

    // Predict new state estimate Xkk_1 = Fk * Xk_1k_1

    tQuat = m_Fk * m_stateQ;
    tQuat *= m_timeDelta;
    m_stateQ += tQuat;

//    m_stateQ.normalize();

    // Compute PDot = Fk * Pk_1k_1 + Pk_1k_1 * FkTranspose (note Pkk == Pk_1k_1 at this stage)

    m_PDot = m_Fk * m_Pkk;
    mat = m_Pkk * m_FkTranspose;
    m_PDot += mat;

    // add in Q to get the new prediction

    m_Pkk_1 = m_PDot + m_Q;

    //  multiply by deltaTime (variable name is now misleading though)

    m_Pkk_1 *= m_timeDelta;
}


void RTFusionKalman4::update()
{
    RTQuaternion delta;
    RTMatrix4x4 Sk, SkInverse;

    if (m_enableCompass || m_enableAccel) {
        m_stateQError = m_measuredQPose - m_stateQ;
    } else {
        m_stateQError = RTQuaternion();
    }

    //	Compute residual covariance Sk = Hk * Pkk_1 * HkTranspose + Rk
    //  Note: since Hk is the identity matrix, this has been simplified

    Sk = m_Pkk_1 + m_Rk;

    //	Compute Kalman gain Kk = Pkk_1 * HkTranspose * SkInverse
    //  Note: again, the HkTranspose part is omitted

    SkInverse = Sk.inverted();

    m_Kk = m_Pkk_1 * SkInverse;

    if (m_debug)
        HAL_INFO(RTMath::display("Gain", m_Kk));

    // make new state estimate

    delta = m_Kk * m_stateQError;

    m_stateQ += delta;

    m_stateQ.normalize();

    //  produce new estimate covariance Pkk = (I - Kk * Hk) * Pkk_1
    //  Note: since Hk is the identity matrix, it is omitted

    m_Pkk.setToIdentity();
    m_Pkk -= m_Kk;
    m_Pkk = m_Pkk * m_Pkk_1;

    if (m_debug)
        HAL_INFO(RTMath::display("Cov", m_Pkk));
}


void RTFusionKalman4::newIMUData(RTIMU_DATA& data, RTIMU_DATA& data2, const RTIMUSettings *settings, const RTIMUSettings *settings2)
{
	RTVector3 test_gyro1, test_gyro2, test_accel1, test_accel2, test_compass1, test_compass2;
	if (m_enableGyro){
		test_gyro1 = data.gyro;
		test_gyro2 = data2.gyro;
		m_gyro.setX((test_gyro1.x() + test_gyro2.x()) / (RTFLOAT)2.0);
		m_gyro.setY((test_gyro1.y() + test_gyro2.y()) / (RTFLOAT)2.0);
		m_gyro.setZ((test_gyro1.z() + test_gyro2.z()) / (RTFLOAT)2.0);
	}
		
    else
        m_gyro = RTVector3();

	test_accel1 = data.accel;
	test_accel2 = data2.accel;
	m_accel.setX((test_accel1.x() + test_accel2.x()) / (RTFLOAT)2.0);
	m_accel.setY((test_accel1.y() + test_accel2.y()) / (RTFLOAT)2.0);
	m_accel.setZ((test_accel1.z() + test_accel2.z()) / (RTFLOAT)2.0);

	test_compass1 = data.compass;
	test_compass2 = data2.compass;
	m_compass.setX((test_compass1.x() + test_compass2.x()) / (RTFLOAT)2.0);
	m_compass.setY((test_compass1.y() + test_compass2.y()) / (RTFLOAT)2.0);
	m_compass.setZ((test_compass1.z() + test_compass2.z()) / (RTFLOAT)2.0);


    m_compassValid = data.compassValid;
	m_compassValid2 = data2.compassValid;

    if (m_firstTime) {
        m_lastFusionTime = data.timestamp;
        calculatePose(m_accel, m_compass, settings->m_compassAdjDeclination);
        m_Fk.fill(0);

        //  init covariance matrix to something

        m_Pkk.fill(0);
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m_Pkk.setVal(i,j, 0.5);

        // initialize the observation model Hk
        // Note: since the model is the state vector, this is an identity matrix so it won't be used

        //  initialize the poses

        m_stateQ.fromEuler(m_measuredPose);
        m_fusionQPose = m_stateQ;
        m_fusionPose = m_measuredPose;
        m_firstTime = false;
    } else {
        m_timeDelta = (RTFLOAT)(data.timestamp - m_lastFusionTime) / (RTFLOAT)1000000;
        m_lastFusionTime = data.timestamp;
        if (m_timeDelta <= 0)
            return;

        if (m_debug) {
            HAL_INFO("\n------\n");
            HAL_INFO1("IMU update delta time: %f\n", m_timeDelta);
        }

        calculatePose(m_accel, m_compass, settings->m_compassAdjDeclination);

        predict();
        update();
        m_stateQ.toEuler(m_fusionPose);
        m_fusionQPose = m_stateQ;

        if (m_debug) {
            HAL_INFO(RTMath::displayRadians("Measured pose", m_measuredPose));
            HAL_INFO(RTMath::displayRadians("Kalman pose", m_fusionPose));
            HAL_INFO(RTMath::displayRadians("Measured quat", m_measuredPose));
            HAL_INFO(RTMath::display("Kalman quat", m_stateQ));
            HAL_INFO(RTMath::display("Error quat", m_stateQError));
         }
    }
    data.fusionPoseValid = true;
    data.fusionQPoseValid = true;
    data.fusionPose = m_fusionPose;
    data.fusionQPose = m_fusionQPose;

}

void RTFusionKalman4::LSE(RTIMU_DATA& data, RTIMU_DATA& data2, const RTIMUSettings *settings, const RTIMUSettings *settings2)
{
	// averaging out IMU data
	RTVector3 test_accel1, test_accel2;
	test_accel1 = data.accel;
	test_accel2 = data2.accel;
	m_accel.setX((test_accel1.x() + test_accel2.x()) / (RTFLOAT)2.0);
	m_accel.setY((test_accel1.y() + test_accel2.y()) / (RTFLOAT)2.0);
	m_accel.setZ((test_accel1.z() + test_accel2.z()) / (RTFLOAT)2.0);

	// initializing matrices
	mat x_hat;
	x_hat.zeros(6, 1);
	mat delta;
	delta.zeros(6, 1);
	for (int i = 0; i < 6; i++){
		delta(i, 0) = 1;
	}
	mat Cl;
	Cl.zeros(9, 9);
	mat Cx;
	


	if (m_firstTime) {
		delta_time = data.timestamp;
		m_firstTime = false;
		position_ini.setX(0);
		position_ini.setY(0);
		position_ini.setZ(0);
		velocity_ini.setX(0);
		velocity_ini.setY(0);
		velocity_ini.setZ(0);
		Cl(3, 3) = 0;
		Cl(4, 4) = 0;
		Cl(5, 5) = 0;
	}
	else{
		delta_time = (RTFLOAT)(data.timestamp - last_time) / (RTFLOAT)1000000;
		last_time = data.timestamp;
		Cl(3, 3) = vel_error.x();
		Cl(4, 4) = vel_error.y();
		Cl(5, 5) = vel_error.z();
	}

	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			Cl(i, j) = 0.1; //accelerometer error here
		}
	}

	for (int i = 6; i < 9; i++){
		for (int j = 6; j < 9; j++){
			Cl(i, j) = 0.1; //accelerometer error here
		}
	}

	double a_post = 1;
	while (delta.max() > 0.0000001){
		mat A;
		A.zeros(9, 6);
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				A(i, j) = 2 / (delta_time*delta_time);
			}
		}
		for (int i = 3; i < 6; i++){
			for (int j = 3; j < 6; j++){
				A(i, j) = -1;
			}
		}

		for (int i = 3; i < 9; i++){
			for (int j = 0; j < 6; j++){
				A(i, j) = (1 / delta_time);
			}
		}

		mat w;
		w.zeros(9, 1);
		w(0, 0) = ((2 * (x_hat(0, 0) - velocity_ini.x()*delta_time)) / (delta_time*delta_time)) - m_accel.x();
		w(1, 0) = ((2 * (x_hat(1, 0) - velocity_ini.y()*delta_time)) / (delta_time*delta_time)) - m_accel.y();
		w(2, 0) = ((2 * (x_hat(2, 0) - velocity_ini.z()*delta_time)) / (delta_time*delta_time)) - m_accel.z();
		w(3, 0) = (((x_hat(0, 0) - position_ini.x()) / delta_time) - x_hat(3, 0));
		w(4, 0) = (((x_hat(1, 0) - position_ini.y()) / delta_time) - x_hat(4, 0));
		w(5, 0) = (((x_hat(2, 0) - position_ini.z()) / delta_time) - x_hat(5, 0));
		w(6, 0) = (((x_hat(3, 0) - velocity_ini.x()) / delta_time) - m_accel.x());
		w(7, 0) = (((x_hat(4, 0) - velocity_ini.y()) / delta_time) - m_accel.y());
		w(8, 0) = (((x_hat(5, 0) - velocity_ini.z()) / delta_time) - m_accel.z());



		mat P;
		P.zeros(9, 9);
		P = a_post*Cl.i();

		mat N, u;
		N = A.t()*P*A;
		u = A.t()*P*w;

		delta = -1 * N.i()*u;
		x_hat = x_hat + delta;
		mat v;
		v = A*delta + w;

		//l=l+v????
		mat temp;
		temp = (v.t()*P*v) / (9 - 6);
		a_post = temp(0, 0);
		Cx = a_post*N.i();
	}

	//x_hat.print_mat("Estimated_observations.txt", 5, 12, "First three values are X, Y, Z position, next three are X, Y, Z for velocity.", " ");
	position_ini.setX((RTFLOAT)(x_hat(0, 0)));
	position_ini.setY((RTFLOAT)(x_hat(1, 0)));
	position_ini.setZ((RTFLOAT)(x_hat(2, 0)));
	velocity_ini.setX((RTFLOAT)(x_hat(3, 0)));
	velocity_ini.setY((RTFLOAT)(x_hat(4, 0)));
	velocity_ini.setZ((RTFLOAT)(x_hat(5, 0)));
	vel_error.setX((RTFLOAT)(sqrt(Cx(3, 3))));
	vel_error.setY((RTFLOAT)(sqrt(Cx(4, 4))));
	vel_error.setZ((RTFLOAT)(sqrt(Cx(5, 5))));


}
