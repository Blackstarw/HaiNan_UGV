#include "hw_ublox.h"
#include "usart.h"
#include "string.h"
#include "stdlib.h"
#include "hw_can.h"
#include "agv_ctr.h"

int _gprmc_analysis(Gps_Info_t *p, unsigned char *str);
Ublox_Item_t ublox_item;
Gps_Info_t gps_info;
void hw_ubloxInit(void)
{
	  HAL_UART_Receive_DMA(&huart4, ublox_item.ublox_data_buf, UBLOX_DATA_BUF_LEN);
	  __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
}


void UART4_IDLECallBack(UART_HandleTypeDef *huart)
{
	__HAL_UART_CLEAR_IDLEFLAG(&huart4);
	HAL_UART_DMAStop(&huart4);
	ublox_item.ublox_msg_size = UBLOX_DATA_BUF_LEN - hdma_uart4_rx.Instance->NDTR;

	if((ublox_item.ublox_data_buf[0] == 36) && (ublox_item.ublox_data_buf[ublox_item.ublox_msg_size - 1] == 10))
	{
		ublox_item.ublox_msg_tick = HAL_GetTick();
		_gprmc_analysis(&gps_info, ublox_item.ublox_data_buf);
	}
	HAL_UART_Receive_DMA(&huart4, ublox_item.ublox_data_buf, UBLOX_DATA_BUF_LEN);
}

unsigned char data_buf[20][20];
int _gprmc_analysis(Gps_Info_t *p, unsigned char *str)
{
	//unsigned char data_buf[20];

	if(str[5] != 'C')   //非GPRMC
	{
		return -1;
	}

	p->gps_new_msg = 1;

	if(str[17] != 'A')  //无效数据
	{
		p->gps_positioning_status = Invalid;
		return -2;
	}

	p->gps_positioning_status = Effective;

	memcpy(data_buf[0], str + 19, 9);
	p->latitude = atof((char*)data_buf[0]);
	memset(data_buf[0], 0, 20);

	memcpy(data_buf[1], str + 32, 9);
	p->longitude = atof((char*)data_buf[1]);
	memset(data_buf[1], 0, 20);

	memcpy(data_buf[2], str + 46, 5);
	p->speed = atof((char*)data_buf[2]);
	memset(data_buf[2], 0, 20);
	int idx = 0;
	for(idx = 52; idx < 60; idx ++)
	{
		if(str[idx] == 44)
		{
			p->direction = atof((char*)data_buf[3]);
			memset(data_buf[3], 0, 20);
			break;
		}
		data_buf[3][idx - 52] = str[idx];
	}

	data_buf[4][0] = str[7];
	data_buf[4][1] = str[8];
	p->date_time.hour = (uint8_t)atoi((char*)data_buf[4]) + 8;
	if(gps_info.date_time.hour / 24)
	{
		p->date_time.hour %= 24;
	}
	memset(data_buf[4], 0, 20);

	data_buf[4][0] = str[9];
	data_buf[4][1] = str[10];
	p->date_time.minute = (uint8_t)atoi((char*)data_buf[4]);
	memset(data_buf[4], 0, 20);

	data_buf[4][0] = str[11];
	data_buf[4][1] = str[12];
	p->date_time.second = (uint8_t)atoi((char*)data_buf[4]);
	memset(data_buf[4], 0, 20);

	data_buf[4][0] = str[idx + 1];
	data_buf[4][1] = str[idx + 2];
	p->date_time.day = (uint8_t)atoi((char*)data_buf[4]);
	memset(data_buf[4], 0, 20);

	data_buf[4][0] = str[idx + 3];
	data_buf[4][1] = str[idx + 4];
	p->date_time.month = (uint8_t)atoi((char*)data_buf[4]);
	memset(data_buf[4], 0, 20);

	data_buf[4][0] = str[idx + 5];
	data_buf[4][1] = str[idx + 6];
	p->date_time.year = (uint8_t)atoi((char*)data_buf[4]);
	memset(data_buf[4], 0, 20);

	while (idx ++ < ublox_item.ublox_msg_size)
	{
		if(str[idx] == '$') break;
	}

	p->gps_quality = (Gps_Quality_e)(str[idx + 44] - 48);
	data_buf[5][0] = str[idx + 46];
	data_buf[5][1] = str[idx + 47];
	p->satellite = (uint8_t)atoi((char*)data_buf[5]);
	memset(data_buf[5], 0, 20);

	idx += 47;
	uint8_t num_comma = 0;
	while (idx ++ < ublox_item.ublox_msg_size)
	{
		if(str[idx] == ',')
		{
			num_comma ++;
			if(num_comma == 2) break;
		}
	}
	int idx_c = idx + 1;
	for( idx = idx_c; idx < ublox_item.ublox_msg_size; idx ++)
	{
		if(str[idx] == 44)
		{
			p->height = atof((char*)data_buf[6]);
			memset(data_buf[6], 0, 20);
			break;
		}
		data_buf[6][idx -idx_c] = str[idx];
	}
	return 0;
}


int gpgga_analysis(Gps_Info_t *p, unsigned char *str)
{
	return 0;
}



void gps_uploadData(void)
{
	if(gps_info.gps_new_msg)
	{
		gps_info.gps_new_msg = 0;
		hw_canSendData(ublox_item.ublox_data_buf, ublox_item.ublox_msg_size);
	}
}










