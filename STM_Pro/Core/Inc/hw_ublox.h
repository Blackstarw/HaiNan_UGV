#ifndef _HW_UBLOX_H
#define _HW_UBLOX_H


#include "stm32f4xx_hal.h"
#include "agv_def.h"

#define UBLOX_DATA_BUF_LEN (170U)


typedef enum
{
	Invalid = 0x00,
	Effective = 0x01

}GPS_Positioning_Status_e;

typedef struct{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}__attribute__((__packed__)) Date_Time_t;


typedef enum
{
	Gps_Invalid = 0,
	Gps_2D_3D = 1,
	Gps_DGNSS = 2,
	Gps_Fixed_RTK = 4,
	Gps_Float_RTK = 5,
	Gps_Dead_Reckoning = 6
}Gps_Quality_e;

typedef struct
{
	uint8_t gps_new_msg;
	GPS_Positioning_Status_e gps_positioning_status;
	double  latitude; //经度
	double  longitude; //纬度
	double  speed; //速度
	double  direction; //航向
	double  height; //海拔高度
	int     satellite; //卫星编号
	Gps_Quality_e gps_quality;
	Date_Time_t date_time;
}__attribute__((__packed__)) Gps_Info_t;




typedef struct
{
	uint8_t ublox_data_buf[UBLOX_DATA_BUF_LEN];
	//uint8_t ublox_msg_raw_data[SBUS_MSG_LEN];

	uint32_t ublox_msg_tick;
	uint8_t ublox_msg_size;
	uint8_t ublox_flag;
}Ublox_Item_t;


typedef enum
{
	Gps_Can_ID_Latitude = 0x50,
	Gps_Can_ID_Longitude = 0x51,
	Gps_Can_ID_Speed = 0x52,
	Gps_Can_ID_Direction = 0x53,
	Gps_Can_ID_Height = 0x54,
	Gps_Can_ID_Satellite = 0x56,
	Gps_Can_ID_Date_Time = 0x57

}Gps_Upload_Can_ID_e;




void hw_ubloxInit(void);
void gps_uploadData(void);

void UART4_IDLECallBack(UART_HandleTypeDef *huart);
















#endif
