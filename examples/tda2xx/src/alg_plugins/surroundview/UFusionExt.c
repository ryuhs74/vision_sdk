/*===================================================================================
Ultrasonic Sensor Calibration (Extrinsic)
- SV_UCalibration_USensorPositions_x:
x coordinates (in SV coordinate system) of ultrasonic sensors (entry i is for sensor with deviceId i)
- SV_UCalibration_USensorPositions_y:
y coordinates (in SV coordinate system) of ultrasonic sensors (entry i is for sensor with deviceId i)
- SV_UCalibration_USensorAzimuthAngles:
azimuth angles of ultrasonic sensors in degrees (entry i is for sensor with deviceId i)
===================================================================================*/
int SV_UCalibration_USensorPositions_x[] =     { 330+20, 440, 550-20,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int SV_UCalibration_USensorPositions_y[] =     { 640, 640, 640, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
short SV_UCalibration_USensorAzimuthAngles[] = { 180, 180, 180, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/*===================================================================================
Ultrasonic Calibration (Intrinsic):
- slope and intercept for line fitting: raw sensor measurement -> physical distance.
- alpha is the azimuthal cone angle (field of view is [-alpha, alpha] degrees)
===================================================================================*/
float SV_UCalibration_intrinsic_slope = 1.0113f;
float SV_UCalibration_intrinsic_intercept = 5.0f; //14/12/24
short SV_UCalibration_intrinsic_alpha = 20; //cone angle in degrees




