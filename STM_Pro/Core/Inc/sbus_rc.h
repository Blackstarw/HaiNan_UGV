#ifndef _SBUS_RC_H
#define _SBUS_RC_H

#include "stm32f4xx_hal.h"
#include  "agv_def.h"

#define SBUS_MSG_LEN (25U)
#define SBUS_CHANNEL_MAX (16U)
#define SBUS_DATA_BUF_LEN (30U)

typedef enum
{
	Disconnected = 0x00,
    Connected = 0x01
}Sbus_Remote_Control_Status_e;

typedef enum
{
   Button_Pressed = 1793,
   Button_Released = 193
}Sbus_Btn_Opera_e;

typedef enum
{
   Button_Camera_HD = 0x00,
   Button_Camera_Infrared = 0x01,
   Button_Camera_Multispectral = 0x02,
   Button_Max = 0x03
}Sbus_Btn_Type_e;

typedef struct
{
	Sbus_Btn_Type_e btn_type;
	Sbus_Btn_Opera_e btn_status;
	uint32_t btn_pressed_tick;
	uint32_t btn_released_tick;
}Sbus_Btn_Status_t;
typedef struct
{
	uint8_t sbus_data_buf[SBUS_DATA_BUF_LEN];
	uint8_t sbus_msg_raw_data[SBUS_MSG_LEN];
	uint16_t sbus_channel[SBUS_CHANNEL_MAX];
	uint32_t sbus_msg_tick;
	uint8_t sbus_msg_size;
	uint8_t sbus_flag;
	Sbus_Remote_Control_Status_e sbus_remote_control_status;
}Sbus_Item_t;



typedef struct
{
	uint16_t sbus_channel[SBus_Channel_Max];

}SBus_Channel_Status_t;

extern Sbus_Item_t sbus_item;


void sbus_init(void);
void UART7_IDLECallBack(UART_HandleTypeDef *huart);









#endif
