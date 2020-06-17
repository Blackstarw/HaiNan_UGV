#ifndef _AGV_DEF_H
#define _AGV_DEF_H

#define DEBUG_MODEL (1)

#include "stm32f4xx_hal.h"

typedef enum
{
    Camera_HD = 0x00,
    Camera_Infrared = 0x01,
	Camera_Multispectral = 0x02,
    Camera_Type_Max = 0x03
}Camera_Type_e;

typedef enum
{
   Camera_Operation_ID = 0x55,
   Camera_Take_Photo = 0x66
}Camera_Operation_e;

typedef enum
{
	Preparing = 0x00,
	Ready = 0x01
}Led_Status_e;


typedef struct
{
    uint8_t Agv_Start;
    uint8_t Camera_HD_Start;
    uint8_t Camera_Infrared_Start;
    uint8_t GPS_OK;
    uint8_t Internet_OK;
}__attribute__((__packed__))Agv_Device_Status_t;



typedef enum
{
	SBus_Channel_Elevator = 8,
	SBus_Channel_Camera_Multispectral = 12,
    SBus_Channel_Camera_HD = 13,
	SBus_Channel_Camera_Infrared = 14,

	SBus_Channel_Max = 16
}SBus_Channel_Type_e;














#endif
