#ifndef _AGV_CTR_H
#define _AGV_CTR_H

#include "stm32f4xx_hal.h"
#include "agv_def.h"

Agv_Device_Status_t agv_device_status;



void cam_takePhoto(Camera_Type_e cam_type);
void agv_ctr_init(void);

void agv_sysProc(void);


void agv_ctrHorizontalPlatform(void);

void agv_ctrVerticalPlatform(void);

void agv_canMsgProc();

void agv_deviceStatus(void);

void agv_ledCtr(Led_Status_e led_status);

#endif
