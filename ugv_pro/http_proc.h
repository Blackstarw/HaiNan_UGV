#ifndef HTTP_PROC_H
#define HTTP_PROC_H

#include <QObject>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QTimer>
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QString>
#include <QHttpPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileInfo>
#include "agv_def.h"
#include "qx_proc.h"

class HTTP_Proc : public QObject
{
    Q_OBJECT
public:
    explicit HTTP_Proc(QObject *parent = nullptr);
    Qx_Account_t get_qxAccount();
    void writeGpsInfo(Gps_Info_t info);

signals:
    void login_success();
    void http_upload_complete();
    void http_qxAccountReady(Qx_Account_t  qx_account);
    void http_uploadFilePolicyReady();
public slots:
    void on_finished();
    void http_login();
    void requset_UploadPolicy();
    void upload_file_proc();
    void http_procPhotoUpload(Camera_Type_e cam_type, QString *PhotoName, int PhotoNum);
    void http_upload_remain_file();
    void http_uploadRealtimeInfo(Gps_Info_t gps_info);
    void http_uploadSysInfo(System_Config_t sys_info);
    void http_replyError(QNetworkReply::NetworkError code);
    void http_getQxAccount();


private:
    QSslConfiguration config;
    QNetworkAccessManager networkManager;
    QNetworkReply *reply;
    QJsonObject resultObj;
    QString Access_Token;
    QString Refresh_Token;

#if SIT_MODEL

    QByteArray Login_Usr_Name = QByteArray("13000000001");
    QByteArray Login_PassWord = QByteArray("cBabYzzzK/YcCd9CDzQpSKaa+pp4GDba1VOBevVazofQR9SMMgzOqOhRvpZONd+wDnu7szudvIfEWoJw62DHtkyecC3jkReOAX7sOW6m4ucgZ0DRzyybqR6NMdk4M2Q2IqAUpOUB1qXsJsU3eB+4dhEnkN3VHGO1kx+GMXQa5t4=");
    QString Login_Url = QString("http://www.sit.eavchina.com/ms/auth/login");
    QString Upload_Policy_Url = QString("http://www.sit.eavchina.com/ms/common/v1/requestUploadFile");
    QString Upload_Real_Info_Url = QString("http://www.sit.eavchina.com/cs/gs/v3/open/addUgvRealInfo");
    QString Upload_Sys_Info_Url = QString("http://www.sit.eavchina.com/cs/gs/v3/open/addUgvSysInfo");
    QString Get_Qx_Url = QString("http://www.sit.eavchina.com/rtk/gs/yun/getQXWZAccount");

#else
    QByteArray Login_Usr_Name = QByteArray("13000000001");
    QByteArray Login_PassWord = QByteArray("cBabYzzzK/YcCd9CDzQpSKaa+pp4GDba1VOBevVazofQR9SMMgzOqOhRvpZONd+wDnu7szudvIfEWoJw62DHtkyecC3jkReOAX7sOW6m4ucgZ0DRzyybqR6NMdk4M2Q2IqAUpOUB1qXsJsU3eB+4dhEnkN3VHGO1kx+GMXQa5t4=");
    QString Login_Url = QString("https://projhn.eavchina.com/ms/auth/login");
    QString Upload_Policy_Url = QString("https://projhn.eavchina.com/ms/common/v1/requestUploadFile");
    //QString Upload_Real_Info_Url = QString("https://projhn.eavchina.com/cs/gs/v3/open/addChargerRealInfo");
    QString Upload_Real_Info_Url = QString("http://www.sit.eavchina.com/cs/gs/v3/open/addUgvRealInfo");
    QString Upload_Sys_Info_Url = QString("https://projhn.eavchina.com/cs/gs/v3/open/addChargerSysInfo");
    QString Get_Qx_Url = QString("https://projhn.eavchina.com/rtk/gs/yun/getQXWZAccount");
#endif
    QFile *file;
    QFileInfo file_info;
    QString file_path;
    int file_num_to_upload;
    int file_num_uploaded;
    QString *UploadPhotoName;

    bool http_flag;

    QString gps_positionConvert(double num);

    Camera_Type_e upload_file_type;

    Gps_Info_t gps_info_photo, gps_info_buf;

    Qx_Account_t  qx_account;

};

#endif // HTTP_PROC_H
