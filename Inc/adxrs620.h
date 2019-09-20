#ifndef __adxrs620__
#define __adxrs620__
#include "FreeRTOS.h"
#include "semphr.h"

extern void GyroTaskCreate(void);
extern void calibrate_gyro(int n);

extern SemaphoreHandle_t Gyro_sem;

#endif