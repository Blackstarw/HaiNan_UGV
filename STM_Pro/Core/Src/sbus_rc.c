#include "sbus_rc.h"
#include "usart.h"
#include "string.h"

Sbus_Item_t sbus_item;
//SBus_Channel_Status_t sbus_ch_stutas;
void sbus_init(void)
{
	  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_4, GPIO_PIN_SET);
	  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET);
	  HAL_UART_Receive_DMA(&huart7, sbus_item.sbus_data_buf, SBUS_DATA_BUF_LEN);
	  __HAL_UART_ENABLE_IT(&huart7, UART_IT_IDLE);
}
void UART7_IDLECallBack(UART_HandleTypeDef *huart)
{
	__HAL_UART_CLEAR_IDLEFLAG(&huart7);
	HAL_UART_DMAStop(&huart7);
	sbus_item.sbus_msg_size = SBUS_DATA_BUF_LEN - hdma_uart7_rx.Instance->NDTR;
	if(sbus_item.sbus_msg_size == SBUS_MSG_LEN)
	{
		memcpy(sbus_item.sbus_msg_raw_data, sbus_item.sbus_data_buf, SBUS_MSG_LEN);
		if((sbus_item.sbus_msg_raw_data[0] == 0x0f) && (sbus_item.sbus_msg_raw_data[24] == 0x00))
		{
			sbus_item.sbus_msg_tick = HAL_GetTick();
			sbus_item.sbus_flag = sbus_item.sbus_msg_raw_data[23];
			sbus_item.sbus_channel[0]  = ((sbus_item.sbus_msg_raw_data[1]     | sbus_item.sbus_msg_raw_data[2]<<8)                                        & 0x07FF);
			sbus_item.sbus_channel[1]  = ((sbus_item.sbus_msg_raw_data[2]>>3  | sbus_item.sbus_msg_raw_data[3]<<5)                                        & 0x07FF);
			sbus_item.sbus_channel[2]  = ((sbus_item.sbus_msg_raw_data[3]>>6  | sbus_item.sbus_msg_raw_data[4]<<2  | sbus_item.sbus_msg_raw_data[5]<<10)  & 0x07FF);
			sbus_item.sbus_channel[3]  = ((sbus_item.sbus_msg_raw_data[5]>>1  | sbus_item.sbus_msg_raw_data[6]<<7)                                        & 0x07FF);
			sbus_item.sbus_channel[4]  = ((sbus_item.sbus_msg_raw_data[6]>>4  | sbus_item.sbus_msg_raw_data[7]<<4)                                        & 0x07FF);
			sbus_item.sbus_channel[5]  = ((sbus_item.sbus_msg_raw_data[7]>>7  | sbus_item.sbus_msg_raw_data[8]<<1  | sbus_item.sbus_msg_raw_data[9]<<9)   & 0x07FF);
			sbus_item.sbus_channel[6]  = ((sbus_item.sbus_msg_raw_data[9]>>2  | sbus_item.sbus_msg_raw_data[10]<<6)                                       & 0x07FF);
			sbus_item.sbus_channel[7]  = ((sbus_item.sbus_msg_raw_data[10]>>5 | sbus_item.sbus_msg_raw_data[11]<<3)                                       & 0x07FF);
			sbus_item.sbus_channel[8]  = ((sbus_item.sbus_msg_raw_data[12]    | sbus_item.sbus_msg_raw_data[13]<<8)                                       & 0x07FF);
			sbus_item.sbus_channel[9]  = ((sbus_item.sbus_msg_raw_data[13]>>3 | sbus_item.sbus_msg_raw_data[14]<<5)                                       & 0x07FF);
			sbus_item.sbus_channel[10] = ((sbus_item.sbus_msg_raw_data[14]>>6 | sbus_item.sbus_msg_raw_data[15]<<2 | sbus_item.sbus_msg_raw_data[16]<<10) & 0x07FF);
			sbus_item.sbus_channel[11] = ((sbus_item.sbus_msg_raw_data[16]>>1 | sbus_item.sbus_msg_raw_data[17]<<7)                                       & 0x07FF);
			sbus_item.sbus_channel[12] = ((sbus_item.sbus_msg_raw_data[17]>>4 | sbus_item.sbus_msg_raw_data[18]<<4)                                       & 0x07FF);
			sbus_item.sbus_channel[13] = ((sbus_item.sbus_msg_raw_data[18]>>7 | sbus_item.sbus_msg_raw_data[19]<<1 | sbus_item.sbus_msg_raw_data[20]<<9)  & 0x07FF);
			sbus_item.sbus_channel[14] = ((sbus_item.sbus_msg_raw_data[20]>>2 | sbus_item.sbus_msg_raw_data[21]<<6)                                       & 0x07FF);
			sbus_item.sbus_channel[15] = ((sbus_item.sbus_msg_raw_data[21]>>5 | sbus_item.sbus_msg_raw_data[22]<<3)                                       & 0x07FF);
		}
		memset(sbus_item.sbus_msg_raw_data, 0, SBUS_MSG_LEN);
	}
	HAL_UART_Receive_DMA(&huart7, sbus_item.sbus_data_buf, SBUS_DATA_BUF_LEN);
}
