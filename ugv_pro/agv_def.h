#ifndef AGV_DEF_H
#define AGV_DEF_H

#include <QString>
#include <QByteArray>
#include <QUrl>
#include <QProcess>

#define SIT_MODEL         (1)

#define MAJOR_VERSION     (1)
#define MINOR_VERSION     (0)
#define INTERNAL_VERSION  (5)

//#define AGV_DEVICE_ID (11111111)

#define STM_DATA_SUMMARY_ID  (0x11)
#define STM_DATA_TX_ID  (0x12)
#define STM_DATA_END_ID  (0x13)

#define DATA_SUMMARY_ID  (0x14)
#define DATA_TX_ID  (0x15)
#define DATA_END_ID  (0x16)

typedef enum
{
    Camera_HD = 0x00,
    Camera_Infrared = 0x01,
    Camera_Type_Max = 0x02
}Camera_Type_e;

typedef enum
{
    GPS_Comm_Fault = 0x00000001,
    GPS_Msg_Fault = 0x00000002,
    GPS_Low_Sat_Num = 0x00000004,
    GPS_No_Diff = 0x00000008,
    GPS_QX_No_Account = 0x01000000,
    GPS_QX_Service_Fault = 0x02000000

}GPS_Fault_Mask_e;

typedef enum
{
    Camera_comm_fault = 0x00000001,
    Camera_image_fault = 0x00000002,
    Camera_failed = 0x00000004
}Camera_Fault_Mask_e;


typedef struct
{
    uint32_t gps_fault;
    uint32_t camera_fault[8];
}__attribute__((__packed__))Sys_Fault_t;


typedef enum
{
    Sys_Camera_None = 0x00,
    Sys_Camera_HD = 0x01,
    Sys_Camera_MultiSpectral = 0x02,
    Sys_Camera_Infrared = 0x03
}Sys_Camera_Type_e;

typedef struct
{
    bool Agv_Start;
    bool Camera_HD_Start;
    bool Camera_Infrared_Start;
    bool GPS_OK;
    bool Internet_OK;
    bool QX_Account_OK;
    bool QX_Server_OK;

}__attribute__((__packed__))Agv_Device_Status_t;

typedef struct
{
    QByteArray can_msg_buf;
    uint32_t can_msg_size;
    uint16_t can_msg_crc;
    uint32_t can_msg_buf_index;
}Can_Msg_t;

typedef enum
{
   Camera_Operation_ID = 0x55,
   Camera_Take_Photo = 0x66,
}Camera_Operation_e;

//QString Work_Path;

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



typedef enum
{
    Gps_Invalid = 0x00,
    Gps_Effective = 0x01

}GPS_Positioning_Status_e;

typedef struct{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}Date_Time_t;

typedef struct
{
    uint8_t gps_new_msg;
    uint8_t gps_positioning_status;
    double  latitude; //经度
    double  longitude; //纬度
    double  speed; //速度
    double  direction; //航向
    double  height; //海拔高度
    int     satellite; //卫星编号
    uint8_t gps_quality;
    //unsigned char     ns;
    //unsigned char     ew;
    Date_Time_t date_time;
}Gps_Info_t;

typedef struct
{
    uint32_t Device_ID;
    uint8_t Firmware_Version[3];
    uint8_t Camera_Tpye[8];
    QString Login_Usr_Name;
    QString Login_PassWord;
    QString Login_Url;
    QString Upload_Policy_Url;
    QString Upload_Real_Info_Url;
    QString Upload_Sys_Info_Url;
    QString Get_Qx_Url;
}System_Config_t;

typedef struct
{
   char* nosr_data;
   uint32_t data_len;
   bool new_msg;
}Nosr_Msg_t;

typedef struct
{
    QString qx_key;
    QString qx_secret;
    QString qx_dev_id;
    QString qx_dev_type;
    bool account_is_valid;
}Qx_Account_t;


typedef struct
{
    QString Dir;
    bool Time_is_Valid;
}Work_Dir_t;

extern Work_Dir_t Work_Dir;
extern Agv_Device_Status_t agv_device_status;
extern Sys_Fault_t sys_fault;
extern System_Config_t agv_sys_config;
#endif // AGV_DEF_H
