#ifndef AGV_CTR_H
#define AGV_CTR_H

#include <QObject>
#include <QThread>
#include "camera_proc.h"
#include "canbus_proc.h"
#include "agv_def.h"
#include "http_proc.h"
#include "qx_proc.h"
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class AGV_Ctr : public QObject
{
    Q_OBJECT
public:
    explicit AGV_Ctr(QObject *parent = nullptr);
    void get_agvSysConfig();
    void agv_sysInit();
signals:

    void cam_takePhoto(Camera_Type_e cam_type);
    void connect_canbus();
    void http_request_policy();
    void http_login();
    void rtk_ready(QByteArray msg_data, uint32_t msg_len);
    void qx_ready();
    void gps_getGGA(char* str, uint32_t str_len);
    void gps_infoReady(Gps_Info_t gps_info);
    void gps_deviceStatus(Agv_Device_Status_t agv_device_status);
public slots:
    void upload_RealtimeInfo(Gps_Info_t info);
    void agv_deviceStatus(Agv_Device_Status_t agv_device_status);
    void qx_sdkHB(char* str, uint32_t str_len);
    void timer1_slot();
    void gps_parseMsg(char* str, uint32_t str_len);
    void agv_startQX(Qx_Account_t  qx_account);
    void agv_sysStatusHB();


private:


    QThread *Camera_Thread;
    QThread *Canbus_Thread;
    QThread *HTTP_Thread;
    HTTP_Proc *http_obj_upload_file;
    HTTP_Proc *http_obj_upload_info;
    HTTP_Proc *http_obj_get_qx;
    Camera_Proc *camera_obj;
    Canbus_Proc *canbus_obj;



    Qx_Proc *qx_obj;
    QThread *qx_Thread;

    QTimer *sys_status_Timer, *timer1;

    Canbus_Proc *canbus_send_rtk;
    QThread *Canbus_Rtk_Thread;
    Gps_Info_t gps_info;

    unsigned char data_buf[20][20];

    Qx_Account_t qx_acc;
};

#endif // AGV_CTR_H
