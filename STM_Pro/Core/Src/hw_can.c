#include "hw_can.h"
#include "can.h"
#include <stdio.h>
#include "usart.h"
#include "string.h"
#include "agv_ctr.h"
#include "cmsis_os.h"

CAN_TxHeaderTypeDef   TxHeader;
CAN_RxHeaderTypeDef   RxHeader;
uint8_t               TxData[8];
uint8_t               RxData[8];
uint32_t              TxMailbox;

#define Data_Summary_ID  (0x11)
#define Data_Tx_ID  (0x12)
#define Data_End_ID  (0x13)

#define CPU_Data_Summary_ID  (0x14)
#define CPU_Data_Tx_ID  (0x15)
#define CPU_Data_End_ID  (0x16)

Can_Msg_t can_msg, can_msg_rtk;

/* Definitions for Q_Can_Msg_Rtk */
osMessageQueueId_t Q_Can_Msg_RtkHandle;
const osMessageQueueAttr_t Q_Can_Msg_Rtk_attributes = {
  .name = "Q_Can_Msg_Rtk"
};

static void _hw_canFilterConfig(void);

void hw_canInit(void)
{
    _hw_canFilterConfig();

    if (HAL_CAN_Start(&hcan2) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }

    TxHeader.ExtId = 0x01;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;
    TxHeader.TransmitGlobalTime = DISABLE;
    /* creation of Q_Can_Msg_Rtk */
    Q_Can_Msg_RtkHandle = osMessageQueueNew (5, sizeof(Can_Msg_t), &Q_Can_Msg_Rtk_attributes);

}

static void _hw_canFilterConfig(void)
{

    CAN_FilterTypeDef  sFilterConfig;
    sFilterConfig.FilterBank = 14;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

HAL_StatusTypeDef hw_canTx(void)
{
	HAL_StatusTypeDef status;
	status = HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox);
	return status;

}

uint16_t crc_16_CCITT_False(uint8_t* pucFrame, uint32_t offset, uint16_t initial_value, uint16_t usLen)
{
    uint32_t start = offset;
    uint32_t end = offset + usLen;
    uint16_t crc = initial_value; // initial value 0xffff
    const uint16_t  polynomial = 0x1021; // poly value

    for (uint32_t index = start; index < end; index++)
    {
        uint8_t b = pucFrame[index];

        for (uint32_t i = 0; i < 8; i++)
        {
            uint8_t bit = ((b >> (7 - i) & 1) == 1);
            uint8_t c15 = ((crc >> 15 & 1) == 1);
            crc <<= 1;
            if (c15 ^ bit) crc ^= polynomial;
        }
    }
    crc &= 0xffff;
    return crc;
}


void hw_canSendData(uint8_t *data, uint32_t data_len)
{
	uint32_t frame_delay = 10;
	uint16_t Data_CRC = crc_16_CCITT_False(data, 0, 0xffff, data_len);

	memcpy(TxData, &data_len, 4);
	memcpy((TxData + 4), &Data_CRC, 2);
	TxHeader.StdId = Data_Summary_ID;
	TxHeader.DLC = 6;
	hw_canTx();
	osDelay(frame_delay);
	uint32_t count = data_len / 8;
	uint32_t mod = data_len % 8;
	uint32_t idx;

	for(idx = 0; idx < count; idx ++)
	{
		memcpy(TxData, data + idx * 8, 8);
		TxHeader.StdId = Data_Tx_ID;
		TxHeader.DLC = 8;
		hw_canTx();
		osDelay(frame_delay);
	}

	memcpy(TxData, data + idx * 8, mod);
	TxHeader.StdId = Data_Tx_ID;
	TxHeader.DLC = mod;
	hw_canTx();
	osDelay(frame_delay);
	TxHeader.StdId = Data_End_ID;
	TxHeader.DLC = 1;
	hw_canTx();
	TxHeader.DLC = 8;
	osDelay(frame_delay);

}


void hw_canProcMsg()
{
	static uint32_t msg_cnt = 0;
	if((RxHeader.StdId == CPU_Data_Summary_ID) && (RxHeader.DLC == 6))
	{
		memset(&can_msg, 0, sizeof(Can_Msg_t));
		memcpy(&can_msg.can_msg_size, RxData, sizeof(uint32_t));
		memcpy(&can_msg.can_msg_crc, (RxData + 4), sizeof(uint16_t));
	}
	if(RxHeader.StdId == CPU_Data_Tx_ID)
	{
		memcpy((can_msg.can_msg_buf + can_msg.can_msg_buf_index), RxData, RxHeader.DLC);
		can_msg.can_msg_buf_index += RxHeader.DLC;
	}
	if((RxHeader.StdId == CPU_Data_End_ID) && (RxHeader.DLC == 1))
	{
		if(can_msg.can_msg_buf_index == can_msg.can_msg_size)
		{
			if(can_msg.can_msg_crc == crc_16_CCITT_False(can_msg.can_msg_buf, 0, 0xffff, can_msg.can_msg_size))
			{
				can_msg.can_msg_cnt = ++msg_cnt;
				can_msg.can_msg_tick = HAL_GetTick();
				osMessageQueuePut(Q_Can_Msg_RtkHandle, (void *)&can_msg, 0, 0U);
			}
		}
	}
}

void agv_procRTKMsg()
{
	osStatus_t status = osMessageQueueGet(Q_Can_Msg_RtkHandle, (void *)&can_msg_rtk, 0, osWaitForever);
	if(osOK == status)
	{
		HAL_UART_Transmit(&huart4, can_msg_rtk.can_msg_buf, can_msg_rtk.can_msg_size, 100);
	}
}


