#include "agv_ctr.h"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/photo.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/core/types.hpp"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/imgproc.hpp"
#include <QPicture>
#include <QPixmap>
#include <QDebug>
#include <QImage>
#include "agv_def.h"
#include "string.h"
#include "qxwz_types.h"
#include "qxwz_sdk.h"
#include <QFileInfo>
#include <QFile>

Agv_Device_Status_t agv_device_status;
System_Config_t agv_sys_config;
Sys_Fault_t sys_fault;

AGV_Ctr::AGV_Ctr(QObject *parent) : QObject(parent)
{
    get_agvSysConfig();
    memset(&sys_fault, 0, sizeof(Sys_Fault_t));
    sys_fault.gps_fault = GPS_Comm_Fault |
                          GPS_Msg_Fault |
                          GPS_Low_Sat_Num |
                          GPS_No_Diff |
                          GPS_QX_No_Account |
                          GPS_QX_Service_Fault;

    sys_fault.camera_fault[0] = Camera_comm_fault |
                                Camera_failed;
    sys_fault.camera_fault[1] = Camera_comm_fault |
                                Camera_failed;
    //sys_fault.camera_fault[2] = Camera_comm_fault |
    //                            Camera_failed;

}
void AGV_Ctr::agv_sysInit()
{
    qRegisterMetaType <Camera_Type_e> ("Camera_Type_e");
    qRegisterMetaType <Gps_Info_t> ("Gps_Info_t");
    qRegisterMetaType <Agv_Device_Status_t> ("Agv_Device_Status_t");
    qRegisterMetaType <uint32_t> ("uint32_t");
    qRegisterMetaType <Qx_Account_t> ("Qx_Account_t");
    memset(&agv_device_status, 0, sizeof(Agv_Device_Status_t));
    canbus_obj = new Canbus_Proc;
    http_obj_upload_file = new HTTP_Proc;
    http_obj_upload_info = new HTTP_Proc(this);
    http_obj_get_qx = new HTTP_Proc(this);
    qx_obj = new Qx_Proc;
    sys_status_Timer = new QTimer(this);
    timer1 = new QTimer(this);
    connect(sys_status_Timer, &QTimer::timeout, this, &AGV_Ctr::agv_sysStatusHB);
    Canbus_Thread = new QThread(this);
    canbus_obj->connectDevice();
    canbus_obj->moveToThread(Canbus_Thread);
    Canbus_Thread->start();
    connect(canbus_obj, &Canbus_Proc::gps_msgReceived, this, &AGV_Ctr::gps_parseMsg);
    connect(timer1, &QTimer::timeout, this, &AGV_Ctr::timer1_slot);
    connect(this, &AGV_Ctr::gps_getGGA, this, &AGV_Ctr::qx_sdkHB);

    qx_Thread = new QThread;
    qx_obj->moveToThread(qx_Thread);
    connect(this, &AGV_Ctr::qx_ready, qx_obj, &Qx_Proc::qx_checkNosrData);
    connect(qx_obj, &Qx_Proc::qx_nosrReady, canbus_obj, &Canbus_Proc::can_sendMsg);
    camera_obj = new Camera_Proc;
    camera_obj->cam_connect();
    Camera_Thread = new QThread(this);
    camera_obj->moveToThread(Camera_Thread);
    Camera_Thread->start();
    connect(http_obj_get_qx, &HTTP_Proc::http_qxAccountReady, this, &AGV_Ctr::agv_startQX);
    connect(http_obj_get_qx, &HTTP_Proc::login_success, http_obj_get_qx, &HTTP_Proc::http_getQxAccount);
    connect(this, &AGV_Ctr::gps_infoReady, this, &AGV_Ctr::upload_RealtimeInfo);
    connect(canbus_obj, &Canbus_Proc::cam_takePhoto, camera_obj, &Camera_Proc::cam_takePhoto);
    connect(camera_obj, &Camera_Proc::cam_photoSaveComplete, http_obj_upload_file, &HTTP_Proc::http_procPhotoUpload);
    connect(http_obj_upload_file, &HTTP_Proc::login_success, http_obj_upload_file, &HTTP_Proc::requset_UploadPolicy);
    connect(http_obj_upload_file, &HTTP_Proc::http_uploadFilePolicyReady, http_obj_upload_file, &HTTP_Proc::upload_file_proc);
    connect(http_obj_upload_file, &HTTP_Proc::http_upload_complete, http_obj_upload_file, &HTTP_Proc::http_upload_remain_file);
    sys_status_Timer->start(2000);
}
void AGV_Ctr::get_agvSysConfig()
{
    agv_sys_config.Firmware_Version[0] = MAJOR_VERSION;
    agv_sys_config.Firmware_Version[1] = MINOR_VERSION;
    agv_sys_config.Firmware_Version[2] = INTERNAL_VERSION;

    QFile sys_config_file("/home/linaro/agv_release/agv_config");
    sys_config_file.open(QIODevice::ReadOnly);
    QByteArray configData = sys_config_file.readAll();
    sys_config_file.close();
    QJsonDocument configJson;
    QJsonObject configObj;
    if(configData.isEmpty())
    {
       qDebug() << "Agv configure file not found!";
    }else
    {
        configJson = QJsonDocument::fromJson(configData);
        configObj = configJson.object();
        agv_sys_config.Device_ID = configObj.value("Device_ID").toInt();
        agv_sys_config.Camera_Tpye[0] = configObj.value("Camera_Tpye_1").toInt();
        agv_sys_config.Camera_Tpye[1] = configObj.value("Camera_Tpye_2").toInt();
        agv_sys_config.Camera_Tpye[2] = configObj.value("Camera_Tpye_3").toInt();
        agv_sys_config.Camera_Tpye[3] = configObj.value("Camera_Tpye_4").toInt();
        agv_sys_config.Camera_Tpye[4] = configObj.value("Camera_Tpye_5").toInt();
        agv_sys_config.Camera_Tpye[5] = configObj.value("Camera_Tpye_6").toInt();
        agv_sys_config.Camera_Tpye[6] = configObj.value("Camera_Tpye_7").toInt();
        agv_sys_config.Camera_Tpye[7] = configObj.value("Camera_Tpye_8").toInt();
        agv_sys_config.Get_Qx_Url = configObj.value("Get_Qx_Url").toString();
        agv_sys_config.Login_PassWord = configObj.value("Login_PassWord").toString();
        agv_sys_config.Login_Url = configObj.value("Login_Url").toString();
        agv_sys_config.Login_Usr_Name = configObj.value("Login_Usr_Name").toString();
        agv_sys_config.Upload_Policy_Url = configObj.value("Upload_Policy_Url").toString();
        agv_sys_config.Upload_Real_Info_Url = configObj.value("Upload_Real_Info_Url").toString();
        agv_sys_config.Upload_Sys_Info_Url = configObj.value("Upload_Sys_Info_Url").toString();
        QString Version = QString("AGV_SOFTWARE_VERSION:%1.%2.%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(INTERNAL_VERSION);
        //QString Config = QString("AGV_SYS_CONFIG:");
        qDebug() << Version;
        qDebug() << "AGV Device ID: " << agv_sys_config.Device_ID;
    }
}

void AGV_Ctr::agv_sysStatusHB()
{
    agv_deviceStatus(agv_device_status);
}

void AGV_Ctr::agv_startQX(Qx_Account_t  qx_account)
{
    qDebug() << "Agv start qx sdk.";
    qDebug() << "qx_account.qx_key" << qx_account.qx_key;
    qDebug() << "qx_account.qx_secret" << qx_account.qx_secret;
    qDebug() << "qx_account.qx_dev_id" << qx_account.qx_dev_id;
    qDebug() << "qx_account.qx_dev_type" << qx_account.qx_dev_type;
    qxwz_sdk_config_t sdk_config;
    QByteArray buf[4];
    memset(&sdk_config, 0, sizeof(qxwz_sdk_config_t));
    sdk_config.key_type = QXWZ_SDK_KEY_TYPE_AK;

    buf[0] = qx_account.qx_key.toLatin1();
    buf[1] = qx_account.qx_secret.toLatin1();
    buf[2] = qx_account.qx_dev_id.toLatin1();
    buf[3] = qx_account.qx_dev_type.toLatin1();
    strcpy(sdk_config.key, buf[0].data());
    strcpy(sdk_config.secret, buf[1].data());
    strcpy(sdk_config.dev_id, buf[2].data());
    strcpy(sdk_config.dev_type, buf[3].data());

    sdk_config.status_cb    = qx_obj->qx_on_status;
    sdk_config.data_cb      = qx_obj->qx_on_data;
    sdk_config.auth_cb      = qx_obj->qx_on_auth;
    sdk_config.start_cb     = qx_obj->qx_on_start;

    int ret;
    ret = qx_obj->qx_sdkInit(&sdk_config); 
    qDebug() << "Qx sdk init" << ret;
    if(!ret)
    {
        sys_fault.gps_fault &= (~GPS_QX_Service_Fault);
        ret = qx_obj->qx_sdkAuth();
        if(ret)
        {
            sys_fault.gps_fault |= GPS_QX_Service_Fault;
        }else
        {
            sys_fault.gps_fault &= (~GPS_QX_Service_Fault);
        }

        qDebug() << "Qx sdk Auth" <<ret;
    }else
    {
        sys_fault.gps_fault |= GPS_QX_Service_Fault;
    }
    timer1->setSingleShot(true);
    timer1->start(3000);

}

void AGV_Ctr::timer1_slot()
{
    int ret = qx_obj->qx_sdkStart(QXWZ_SDK_CAP_ID_NOSR);
    if(ret)
    {
        sys_fault.gps_fault |= GPS_QX_Service_Fault;
    }else
    {
        sys_fault.gps_fault &= (~GPS_QX_Service_Fault);
    }
    qx_Thread->start();
    qx_obj->qx_getCoord(qx_obj->qx_get_coord_cb);
    qx_obj->qx_setCoord(2, qx_obj->qx_set_coord_cb);
    //QThread::msleep(500);
    //qx_obj->qx_getCoord(qx_obj->qx_get_coord_cb);
    qDebug() << "qx sdk sdkStart" << ret;
    emit qx_ready();
}

void AGV_Ctr::qx_sdkHB(char* str, uint32_t str_len)
{
    if(!qx_acc.account_is_valid) return;

    int ret = qx_obj->qx_sdkUploadGGA(str, str_len);
    if(!ret)
    {
       sys_fault.gps_fault &= (~GPS_QX_Service_Fault);
       agv_device_status.QX_Server_OK = true;
    }else
    {
        sys_fault.gps_fault |= GPS_QX_Service_Fault;
        agv_device_status.QX_Server_OK = false;
    }
    qDebug() << "Qx sdk hb" << ret;
}


void AGV_Ctr::upload_RealtimeInfo(Gps_Info_t info)
{
    if(!Work_Dir.Time_is_Valid)
    {
        QString SysSetDateCmd = QString("sudo date -s %1-%2-%3").arg(QString::number(info.date_time.year + 2000)).arg(QString::number(info.date_time.month)).arg(QString::number(info.date_time.day));
        QString SysSetTimeCmd = QString("sudo date -s %1:%2:%3").arg(QString::number(info.date_time.hour)).arg(QString::number(info.date_time.minute)).arg(QString::number(info.date_time.second));
        qDebug() << SysSetDateCmd;
        QProcess::execute(SysSetDateCmd);
        qDebug() << SysSetTimeCmd;
        QProcess::execute(SysSetTimeCmd);
        Work_Dir.Time_is_Valid = true;
        QFile tempLog(QString(Work_Dir.Dir + "000.log"));
        tempLog.open(QIODevice::ReadOnly);
        QByteArray tempLogData = tempLog.readAll();
        tempLog.close();
        QProcess::execute("sudo rm -rf " + Work_Dir.Dir);

        QDateTime DateTime;
        QString CurrDate = DateTime.currentDateTime().toString("yyyy-MM-dd");
        Work_Dir.Dir = QString("/home/linaro/Documents/AGV_Works/%1/").arg(CurrDate);
        QDir dir;
        if(!dir.exists(Work_Dir.Dir))
        {
            dir.mkpath(Work_Dir.Dir);
        }
        QFile Log(QString(Work_Dir.Dir + CurrDate + ".log"));
        Log.open(QIODevice::WriteOnly | QIODevice::Append);
        Log.write(tempLogData);
        Log.close();
    }
    http_obj_upload_info->http_uploadRealtimeInfo(info);
}


void AGV_Ctr::agv_deviceStatus(Agv_Device_Status_t agv_device_status)
{
    static uint8_t get_qx_cnt = 0;
    if(!agv_device_status.Internet_OK)
    {
        http_obj_upload_info->http_uploadSysInfo(agv_sys_config);
    }else
    {
        if(get_qx_cnt < 5)
        {
            qx_acc = http_obj_get_qx->get_qxAccount();
            if(!qx_acc.account_is_valid)
            {
                 http_obj_get_qx->http_login();
                 get_qx_cnt ++;
            }
        }
    }

   // qDebug() << "sys faul" << sys_fault.camera_fault << sys_fault.gps_fault;
    QCanBusFrame initframe;
    initframe.setFrameId(0x88);
    QByteArray frame_data;
    char data[7];
    memcpy(data, &agv_device_status, 7);
    frame_data.setRawData(data, 7);
    initframe.setPayload(frame_data);
    canbus_obj->sendFrame(initframe);
}

void AGV_Ctr::gps_parseMsg(char* str, uint32_t str_len)
{
    sys_fault.gps_fault &= (~GPS_Comm_Fault);
    if(str[5] != 'C')   //非GPRMC
    {
        qDebug() << "not GPRMC";
        sys_fault.gps_fault |= GPS_Msg_Fault;
        return;
    }else
    {
        sys_fault.gps_fault &= (~GPS_Msg_Fault);
    }

    if(str[17] != 'A')  //无效数据
    {
        qDebug() << "invalide gps";
        agv_device_status.GPS_OK = false;
        gps_info.gps_positioning_status = (uint8_t)Gps_Invalid;
        return;
    }

    agv_device_status.GPS_OK = true;
    gps_info.gps_positioning_status = (uint8_t)Gps_Effective;

    memcpy(data_buf[0], str + 19, 9);
    gps_info.latitude = atof((char*)data_buf[0]) / 100.0;
    memset(data_buf[0], 0, 20);

    memcpy(data_buf[1], str + 32, 9);
    gps_info.longitude = atof((char*)data_buf[1]) / 100.0;
    memset(data_buf[1], 0, 20);

    memcpy(data_buf[2], str + 46, 5);
    gps_info.speed = atof((char*)data_buf[2]);
    memset(data_buf[2], 0, 20);
    uint32_t idx = 0;
    for(idx = 52; idx < 60; idx ++)
    {
        if(str[idx] == 44)
        {
            gps_info.direction = atof((char*)data_buf[3]);
            memset(data_buf[3], 0, 20);
            break;
        }
        data_buf[3][idx - 52] = str[idx];
    }

    data_buf[4][0] = str[7];
    data_buf[4][1] = str[8];
    gps_info.date_time.hour = (uint8_t)atoi((char*)data_buf[4]) + 8;
    if(gps_info.date_time.hour / 24)
    {
        gps_info.date_time.hour %= 24;
    }
    memset(data_buf[4], 0, 20);

    data_buf[4][0] = str[9];
    data_buf[4][1] = str[10];
    gps_info.date_time.minute = (uint8_t)atoi((char*)data_buf[4]);
    memset(data_buf[4], 0, 20);

    data_buf[4][0] = str[11];
    data_buf[4][1] = str[12];
    gps_info.date_time.second = (uint8_t)atoi((char*)data_buf[4]);
    memset(data_buf[4], 0, 20);

    data_buf[4][0] = str[idx + 1];
    data_buf[4][1] = str[idx + 2];
    gps_info.date_time.day = (uint8_t)atoi((char*)data_buf[4]);
    memset(data_buf[4], 0, 20);

    data_buf[4][0] = str[idx + 3];
    data_buf[4][1] = str[idx + 4];
    gps_info.date_time.month = (uint8_t)atoi((char*)data_buf[4]);
    memset(data_buf[4], 0, 20);

    data_buf[4][0] = str[idx + 5];
    data_buf[4][1] = str[idx + 6];
    gps_info.date_time.year = (uint8_t)atoi((char*)data_buf[4]);
    memset(data_buf[4], 0, 20);

    while (idx ++ < str_len)
    {
        if(str[idx] == '$')
        {
            QByteArray gga;
            gga.setRawData(str + idx, str_len - idx);
            qDebug() << "Get gga " << gga;
            emit gps_getGGA(str + idx, str_len - idx);
            break;
        }
    }

    data_buf[5][0] = str[idx + 44];
    gps_info.gps_quality = (uint8_t)atoi((char*)data_buf[5]);
    memset(data_buf[5], 0, 20);
    if(gps_info.gps_quality < 2)
    {
        sys_fault.gps_fault |= GPS_No_Diff;
    }else
    {
        sys_fault.gps_fault &= (~GPS_No_Diff);
    }
    //qDebug() << "gps_quality" << gps_info.gps_quality;
    data_buf[5][0] = str[idx + 46];
    data_buf[5][1] = str[idx + 47];
    gps_info.satellite = (uint8_t)atoi((char*)data_buf[5]);

    if(gps_info.satellite < 4)
    {
        sys_fault.gps_fault |= GPS_Low_Sat_Num;
    }else
    {
        sys_fault.gps_fault &= (~GPS_Low_Sat_Num);
    }

    memset(data_buf[5], 0, 20);

    idx += 47;
    uint8_t num_comma = 0;
    while (idx ++ < str_len)
    {
        if(str[idx] == ',')
        {
            num_comma ++;
            if(num_comma == 2) break;
        }
    }
    uint32_t idx_c = idx + 1;
    for( idx = idx_c; idx < str_len; idx ++)
    {
        if(str[idx] == 44)
        {
            gps_info.height = atof((char*)data_buf[6]);
            memset(data_buf[6], 0, 20);
            break;
        }
        data_buf[6][idx -idx_c] = str[idx];
    }
    http_obj_upload_file->writeGpsInfo(gps_info);
    emit gps_infoReady(gps_info);
    return;
}


