#ifndef _HW_CAN_H
#define _HW_CAN_H

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#define CAN_MSG_MAX_LEN (500U)

extern CAN_TxHeaderTypeDef   TxHeader;
extern CAN_RxHeaderTypeDef   RxHeader;
extern uint8_t               TxData[8];
extern uint8_t               RxData[8];
extern uint32_t              TxMailbox;



typedef struct
{
	uint8_t can_msg_buf[CAN_MSG_MAX_LEN];
	uint32_t can_msg_size;
	uint32_t can_msg_cnt;
	uint32_t can_msg_tick;
	uint16_t can_msg_crc;
	uint32_t can_msg_buf_index;
}Can_Msg_t;


void hw_canInit(void);
HAL_StatusTypeDef hw_canTx(void);

uint16_t crc_16_CCITT_False(uint8_t* pucFrame, uint32_t offset, uint16_t initial_value, uint16_t usLen);

void hw_canSendData(uint8_t *data, uint32_t data_len);

void hw_canProcMsg();
void agv_procRTKMsg();

#endif
