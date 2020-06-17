#include "agv_ctr.h"
#include "hw_can.h"
#include "sbus_rc.h"
#include "string.h"
#include "hw_ublox.h"
#include "tim.h"

#define MIN_TIME_BETWEEN_PHOTO (800)

void _camera_multispectralTakePhoto(void);

Sbus_Btn_Status_t btn_status[Button_Max];


void agv_ctr_init(void)
{
	btn_status[Button_Camera_HD].btn_type = Button_Camera_HD;
	btn_status[Button_Camera_Infrared].btn_type = Button_Camera_Infrared;
	btn_status[Button_Camera_Multispectral].btn_type = Button_Camera_Multispectral;
	HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
}

void agv_sysProc(void)
{
	uint32_t cur_tick = HAL_GetTick();
	agv_ctrHorizontalPlatform();
	agv_ctrVerticalPlatform();

	if(agv_device_status.Agv_Start && agv_device_status.Internet_OK)
	{
		agv_ledCtr(Ready);
	}else
	{
		agv_ledCtr(Preparing);
	}
	if (!agv_device_status.Agv_Start) return;

	if (sbus_item.sbus_channel[SBus_Channel_Camera_HD] == Button_Pressed)
	{
		if (!agv_device_status.Camera_HD_Start ) return;
		if(cur_tick - btn_status[Button_Camera_HD].btn_pressed_tick > MIN_TIME_BETWEEN_PHOTO)
		{
			btn_status[Button_Camera_HD].btn_status = Button_Pressed;
			btn_status[Button_Camera_HD].btn_pressed_tick = HAL_GetTick();
			cam_takePhoto(Camera_HD);
		}
	}else {
		btn_status[Button_Camera_HD].btn_status = Button_Released;
	}
	if (sbus_item.sbus_channel[SBus_Channel_Camera_Infrared] == Button_Pressed)
	{
		if (!agv_device_status.Camera_Infrared_Start ) return;
		if(cur_tick - btn_status[Button_Camera_Infrared].btn_pressed_tick > MIN_TIME_BETWEEN_PHOTO)
		{
			btn_status[Button_Camera_Infrared].btn_status = Button_Pressed;
			btn_status[Button_Camera_Infrared].btn_pressed_tick = HAL_GetTick();
			cam_takePhoto(Camera_Infrared);
		}
	}else {
		btn_status[Button_Camera_Infrared].btn_status = Button_Released;
	}
	if (sbus_item.sbus_channel[SBus_Channel_Camera_Multispectral] == Button_Pressed)
	{
		if(cur_tick - btn_status[Button_Camera_Multispectral].btn_pressed_tick > MIN_TIME_BETWEEN_PHOTO)
		{
			btn_status[Button_Camera_Multispectral].btn_status = Button_Pressed;
			btn_status[Button_Camera_Multispectral].btn_pressed_tick = HAL_GetTick();

			cam_takePhoto(Camera_Multispectral);
		}
	}else {
		btn_status[Button_Camera_Multispectral].btn_status = Button_Released;
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);
	}

}

void agv_canMsgProc()
{
	if(RxHeader.StdId == 0x88)
	{
		 agv_deviceStatus();
	}
	hw_canProcMsg();
}


void agv_deviceStatus(void)
{
	memcpy(&agv_device_status, &RxData, 5);
}

void agv_ledCtr(Led_Status_e led_status)
{
	static uint8_t led_num = 0;
	led_num ++;
	if(led_status == Preparing)
	{
		if(led_num == 50)
		{
			led_num = 0;
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		}
	}else
	{
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	}
}


void cam_takePhoto(Camera_Type_e cam_type)
{
	TxHeader.StdId = Camera_Operation_ID;
	TxHeader.DLC = 0x08;
	memset(TxData, 0, 8);
	switch (cam_type) {
		case Camera_HD:
			TxData[Camera_HD] = Camera_Take_Photo;
			break;
		case Camera_Infrared:
			TxData[Camera_Infrared] = Camera_Take_Photo;
			break;
		case Camera_Multispectral:
			_camera_multispectralTakePhoto();
			break;
		default:
			break;
	}
	hw_canTx();
	memset(TxData, 0, 8);
	return;
}

void _camera_multispectralTakePhoto(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);
	return;
}
uint32_t Value = 145;
void agv_ctrHorizontalPlatform(void)
{

	if(sbus_item.sbus_channel[8] < 1) return;
	static uint16_t ctr_num_H = 0;

	if(sbus_item.sbus_channel[8] > 993)
	{
		ctr_num_H ++;
		if(ctr_num_H == 5)
		{
			ctr_num_H = 0;
			Value ++;
			if(Value > 180) Value = 180;
		}
	}else if(sbus_item.sbus_channel[8] < 993)
	{
		ctr_num_H ++;
		if(ctr_num_H == 5)
		{
			ctr_num_H = 0;
			Value --;
			if(Value < 115) Value = 115;
		}

	}
	__HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, Value);

}
uint8_t verOpera = 0;
HAL_StatusTypeDef stat;
uint32_t ttic;
void agv_ctrVerticalPlatform(void)
{
	if(sbus_item.sbus_channel[11] < 1) return;
	static uint16_t ctr_num_V = 0;

	TxHeader.DLC = 0x04;
	if(sbus_item.sbus_channel[11] > 993)
	{
		ctr_num_V ++;
		if(ctr_num_V == 50)
		{
			verOpera = 1;
			ctr_num_V = 0;
			TxHeader.StdId = 0x188;
			//TxHeader.DLC = 0x04;
			TxData[0] = 0xAA;
			TxData[1] = 0xE8;
			TxData[2] = 0x03;
			TxData[3] = 0x00;
			ttic = HAL_GetTick();
			stat = hw_canTx();
		}
	}else if(sbus_item.sbus_channel[11] < 993)
	{
		ctr_num_V ++;
		if(ctr_num_V == 50)
		{
			verOpera = 1;
			ctr_num_V = 0;
			TxHeader.StdId = 0x188;
			//TxHeader.DLC = 0x04;
			TxData[0] = 0xAA;
			TxData[1] = 0xE8;
			TxData[2] = 0x0C;
			TxData[3] = 0x00;
			hw_canTx();
		}
	}else if((sbus_item.sbus_channel[11] == 993) && verOpera)
	{
		verOpera = 0;
		TxHeader.StdId = 0x188;

		TxData[0] = 0xAA;
		TxData[1] = 0xE8;
		TxData[2] = 0x0A;
		TxData[3] = 0x00;
		hw_canTx();
	}
	TxHeader.DLC = 0x08;
}
