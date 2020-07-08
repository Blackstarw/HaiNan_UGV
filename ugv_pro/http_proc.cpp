#include "http_proc.h"
#include "qx_proc.h"
#include <QDebug>
#include <sys/time.h>

timeval curr_time;

HTTP_Proc::HTTP_Proc(QObject *parent) : QObject(parent)
{
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1_0);
    http_flag = 0;
}

void HTTP_Proc::http_login()
{
    qDebug() << "http login";
    QNetworkRequest requset;
    requset.setSslConfiguration(config);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart namePart, pwdPart, imeiPart;
    namePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    namePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"username\""));
    namePart.setBody(agv_sys_config.Login_Usr_Name.toUtf8());
    pwdPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    pwdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"password\""));
    pwdPart.setBody(agv_sys_config.Login_PassWord.toUtf8());
    imeiPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    imeiPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"imei\""));
    imeiPart.setBody("A100005CE05889");
    requset.setRawHeader(QByteArray("X-Requester"), QByteArray("gs"));
    requset.setRawHeader(QByteArray("imei"), QByteArray("A100005CE05889"));
    multiPart->append(namePart);
    multiPart->append(pwdPart);
    multiPart->append(imeiPart);
    requset.setUrl(QUrl(agv_sys_config.Login_Url));
    reply = networkManager.post(requset, multiPart);
    connect(reply, &QNetworkReply::finished, this, &HTTP_Proc::on_finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &HTTP_Proc::http_replyError);
    multiPart->setParent(reply);
}
void HTTP_Proc::http_replyError(QNetworkReply::NetworkError code)
{
    agv_device_status.Internet_OK = false;
    qDebug() << "Http reply error :" << code;
}

void HTTP_Proc::on_finished()
{
    QJsonParseError err;
    QString str = reply->readAll();
    qDebug()<< "HTTP reply finished: " << str;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &err);
    if(!doc.isNull())
    {
        QJsonObject object = doc.object();
        if(object.contains("Status"))
        {
            if(object.value("Status").toString() == "OK")
            {
                qDebug() << "HTTP upload file success.";
                emit http_upload_complete();
            }
        }

        if(object.value("code").toString() == "000000")
        {
            agv_device_status.Internet_OK = true;
            if(object.contains("result"))
            {
                resultObj = object.value("result").toObject();
                if(resultObj.contains("access_token"))
                {
                    Access_Token = resultObj.value("access_token").toString();
                    Refresh_Token = resultObj.value("refresh_token").toString();
                    qDebug() << "HTTP login success.";
                    emit login_success();
                }else if (resultObj.contains("policy"))
                {
                   qDebug() << "HTTP request upload policy success.";
                   emit http_uploadFilePolicyReady();
                }
                else if (resultObj.contains("appkey"))
                {
                    qx_account.qx_key = resultObj.value("appkey").toString();
                    qx_account.qx_secret = resultObj.value("appSecret").toString();
                    qx_account.qx_dev_id = resultObj.value("deviceId").toString();
                    qx_account.qx_dev_type = resultObj.value("deviceType").toString();
                    qx_account.account_is_valid = true;
                    sys_fault.gps_fault &= (~GPS_QX_No_Account);
                    agv_device_status.QX_Account_OK = true;
                    emit http_qxAccountReady(qx_account);
                    qDebug() << "HTTP request qx account success.";
                }
            }
        }
    }
}

void HTTP_Proc::requset_UploadPolicy()
{
    qDebug()<<"HTTP requset upload policy";
    QString sysType;
    switch (upload_file_type) {
    case Camera_HD:
        sysType = "hd";
        break;
    case Camera_Infrared:
        sysType = "infrared";
        break;
    default:
        sysType = "hd";
        break;
    }
    QNetworkRequest requset;
    requset.setSslConfiguration(config);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart bizTypePart, sysTypePart, longitudePart, latitudePart, deviceIdPart;

    bizTypePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    bizTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"bizType\""));
    bizTypePart.setBody(QByteArray("UGV"));

    sysTypePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    sysTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"sysType\""));
    sysTypePart.setBody(sysType.toUtf8());

    qDebug()<<sysType;
    longitudePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    longitudePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"longitude\""));
    longitudePart.setBody(gps_positionConvert(gps_info_photo.longitude).toUtf8());
    qDebug()<<gps_positionConvert(gps_info_photo.longitude);

    latitudePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    latitudePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"latitude\""));
    latitudePart.setBody(gps_positionConvert(gps_info_photo.latitude).toUtf8());
    qDebug()<<gps_positionConvert(gps_info_photo.latitude);

    deviceIdPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    deviceIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"deviceId\""));
    deviceIdPart.setBody(QString::number(agv_sys_config.Device_ID).toUtf8());
    qDebug()<<QString::number(agv_sys_config.Device_ID);

    requset.setRawHeader(QByteArray("X-Requester"), QByteArray("gs"));
    requset.setRawHeader(QByteArray("X-Authorization"),QByteArray("Bearer " + Access_Token.toUtf8()));
    multiPart->append(bizTypePart);
    multiPart->append(sysTypePart);
    multiPart->append(longitudePart);
    multiPart->append(latitudePart);
    multiPart->append(deviceIdPart);

    requset.setUrl(QUrl(agv_sys_config.Upload_Policy_Url));
    reply = networkManager.post(requset, multiPart);
    connect(reply, &QNetworkReply::finished, this, &HTTP_Proc::on_finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &HTTP_Proc::http_replyError);
    multiPart->setParent(reply);



}

void HTTP_Proc::upload_file_proc()
{
    QNetworkRequest requset;
    requset.setSslConfiguration(config);
    qDebug()<<"HTTP upload file proc";
    QByteArray data, boundary("-----011000010111000001101001\r\n");
    data.append(boundary);
    QString namePart = QString("Content-Disposition: form-data; name=\"name\"\r\n\r\n%1\r\n").arg(file_info.fileName());
    data.append(namePart.toUtf8());
    data.append(boundary);
    QString keyPart = QString("Content-Disposition: form-data; name=\"key\"\r\n\r\n%1%2\r\n").arg(resultObj.value("dir").toString()).arg(file_info.fileName());
    data.append(keyPart.toUtf8());
    data.append(boundary);
    QString policyPart = QString("Content-Disposition: form-data; name=\"policy\"\r\n\r\n%1\r\n").arg(resultObj.value("policy").toString());
    data.append(policyPart.toUtf8());
    data.append(boundary);
    QString accessIDPart = QString("Content-Disposition: form-data; name=\"OSSAccessKeyId\"\r\n\r\n%1\r\n").arg(resultObj.value("accessid").toString());
    data.append(accessIDPart.toUtf8());
    data.append(boundary);
    QString successActionStatusPart = QString("Content-Disposition: form-data; name=\"success_action_status\"\r\n\r\n%1\r\n").arg("200");
    data.append(successActionStatusPart.toUtf8());
    data.append(boundary);
    QString callbackPart = QString("Content-Disposition: form-data; name=\"callback\"\r\n\r\n%1\r\n").arg(resultObj.value("callback").toString());
    data.append(callbackPart.toUtf8());
    data.append(boundary);
    QString signaturePart = QString("Content-Disposition: form-data; name=\"signature\"\r\n\r\n%1\r\n").arg(resultObj.value("signature").toString());
    data.append(signaturePart.toUtf8());
    data.append(boundary);
    QString filePart = QString("Content-Disposition: form-data; name=\"file\"; filename=\"%1\"\r\nContent-Type: %2\r\n\r\n").arg(file_info.fileName()).arg("image/jpeg");
    data.append(filePart);
    if(file->open(QIODevice::ReadOnly))
    {
        QByteArray file_data(file->readAll());
        data.append(file_data);
    }
    data.append("\r\n-----011000010111000001101001--");
    requset.setHeader(QNetworkRequest::ContentTypeHeader,"multipart/form-data; boundary=---011000010111000001101001");
    requset.setUrl(QUrl(resultObj.value("host").toString()));
    reply = networkManager.post(requset, data);
    connect(reply, &QNetworkReply::finished,this,&HTTP_Proc::on_finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &HTTP_Proc::http_replyError);
}

void HTTP_Proc::http_procPhotoUpload(Camera_Type_e cam_type, QString *PhotoName, int PhotoNum)
{
    gps_info_photo = gps_info_buf;
    file_num_uploaded = 0;
    file_num_to_upload = PhotoNum;
    UploadPhotoName = PhotoName;
    upload_file_type = cam_type;
    file_info = QFileInfo(UploadPhotoName[file_num_uploaded]);
    qDebug()<<file_info.filePath();
    qDebug()<<file_info.fileName();
    file = new QFile(UploadPhotoName[file_num_uploaded]);
    http_login();
}

void HTTP_Proc::http_upload_remain_file()
{
    delete file;
    file_num_uploaded ++;
    file_num_to_upload--;
    if(file_num_to_upload)
    {
        file_info = QFileInfo(UploadPhotoName[file_num_uploaded]);
        qDebug()<<file_info.filePath();
        qDebug()<<file_info.fileName();
        file = new QFile(UploadPhotoName[file_num_uploaded]);
        http_login();
    }
}

void HTTP_Proc::http_uploadRealtimeInfo(Gps_Info_t gps_info)
{
    if(!agv_device_status.Internet_OK) return;
    QNetworkRequest requset;
    qDebug()<<"HTTP upload Realtime Info" << "latitude: " << gps_positionConvert(gps_info.latitude)
           << "longitude: " << gps_positionConvert(gps_info.longitude)
           << "height: "<< QString::number(gps_info.height, 'f', 6)
           << "speed: " << QString::number(gps_info.speed, 'f', 6)
           << "heading: " << QString::number(gps_info.direction, 'f', 6)
           << "numSatellites: " << QString::number(gps_info.satellite);
    requset.setSslConfiguration(config);
    requset.setUrl(QUrl(agv_sys_config.Upload_Real_Info_Url));
    requset.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QJsonObject SysInfo_json;
    SysInfo_json.insert("deviceId", QString::number(agv_sys_config.Device_ID));
    SysInfo_json.insert("latitude", gps_positionConvert(gps_info.latitude));
    SysInfo_json.insert("longitude", gps_positionConvert(gps_info.longitude));
    SysInfo_json.insert("altitude", QString::number(gps_info.height, 'f', 6));
    SysInfo_json.insert("speed", QString::number(gps_info.speed, 'f', 6));
    SysInfo_json.insert("heading", QString::number(gps_info.direction, 'f', 6));
    SysInfo_json.insert("numSatellites", QString::number(gps_info.satellite));
    SysInfo_json.insert("gpsPosType", QString::number(0));
    SysInfo_json.insert("diffAge", QString::number(0));
    SysInfo_json.insert("operationStatus", QString::number(0));
    SysInfo_json.insert("closestObstacleDistance", QString::number(0));
    SysInfo_json.insert("gpsFault", QString::number(sys_fault.gps_fault));
    SysInfo_json.insert("qxErrorCode", QString::number(0));
    SysInfo_json.insert("proximitySensorFault", QString::number(0));
    SysInfo_json.insert("remoteControlFault", QString::number(0));
    SysInfo_json.insert("camera1Fault", QString::number(sys_fault.camera_fault[0]));
    SysInfo_json.insert("camera2Fault", QString::number(sys_fault.camera_fault[1]));
    SysInfo_json.insert("camera3Fault", QString::number(0));
    SysInfo_json.insert("camera4Fault", QString::number(0));
    SysInfo_json.insert("camera5Fault", QString::number(0));
    SysInfo_json.insert("camera6Fault", QString::number(0));
    SysInfo_json.insert("camera7Fault", QString::number(0));
    SysInfo_json.insert("camera8Fault", QString::number(0));
    gettimeofday(&curr_time, nullptr);
    SysInfo_json.insert("reportTime", QString::number(curr_time.tv_sec * 1000));
    SysInfo_json.insert("status", 1);
    SysInfo_json.insert("createAt", QString::number(curr_time.tv_sec * 1000));
    SysInfo_json.insert("updateAt", QString::number(curr_time.tv_sec * 1000));
    qDebug() << "currtime" << QString::number(curr_time.tv_sec);
    QJsonArray jsonArray;
    jsonArray.append(SysInfo_json);
    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray post_SysInfoArray = doc.toJson(QJsonDocument::Compact);
    reply = networkManager.post(requset,post_SysInfoArray);
    connect(reply, &QNetworkReply::finished, this, &HTTP_Proc::on_finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &HTTP_Proc::http_replyError);
}

void HTTP_Proc::http_uploadSysInfo(System_Config_t sys_info)
{
    QNetworkRequest requset;
    qDebug()<<"HTTP upload system Info.";
    requset.setSslConfiguration(config);
    requset.setUrl(QUrl(sys_info.Upload_Sys_Info_Url));
    requset.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    QJsonObject SysInfo_json;
    SysInfo_json.insert("deviceId", QString::number(sys_info.Device_ID));
    SysInfo_json.insert("firmwareVersion", QString("%1.%2.%3").arg(sys_info.Firmware_Version[0]).arg(sys_info.Firmware_Version[1]).arg(sys_info.Firmware_Version[2]));

    SysInfo_json.insert("camera1Type", QString::number(sys_info.Camera_Tpye[0]));
    SysInfo_json.insert("camera2Type", QString::number(sys_info.Camera_Tpye[1]));
    SysInfo_json.insert("camera3Type", QString::number(sys_info.Camera_Tpye[2]));
    SysInfo_json.insert("camera4Type", QString::number(sys_info.Camera_Tpye[3]));
    SysInfo_json.insert("camera5Type", QString::number(sys_info.Camera_Tpye[4]));
    SysInfo_json.insert("camera6Type", QString::number(sys_info.Camera_Tpye[5]));
    SysInfo_json.insert("camera7Type", QString::number(sys_info.Camera_Tpye[6]));
    SysInfo_json.insert("camera8Type", QString::number(sys_info.Camera_Tpye[7]));

    gettimeofday(&curr_time, nullptr);
    SysInfo_json.insert("reportTime", QString::number(curr_time.tv_sec * 1000));
    SysInfo_json.insert("status", 1);
    SysInfo_json.insert("createAt", QString::number(curr_time.tv_sec * 1000));
    SysInfo_json.insert("updateAt", QString::number(curr_time.tv_sec * 1000));
    QJsonArray jsonArray;
    jsonArray.append(SysInfo_json);
    QJsonDocument doc;
    doc.setArray(jsonArray);
    QByteArray post_SysInfoArray = doc.toJson(QJsonDocument::Compact);
    http_flag = 0;
    reply = networkManager.post(requset, post_SysInfoArray);
    connect(reply, &QNetworkReply::finished,this,&HTTP_Proc::on_finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &HTTP_Proc::http_replyError);
}

void HTTP_Proc::http_getQxAccount()
{

    qDebug() << "Http get qx account.";
    QNetworkRequest requset;
    requset.setSslConfiguration(config);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart reqPart, imeiPart;
    reqPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    reqPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"requester\""));
    reqPart.setBody(QByteArray("gs"));
    imeiPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
    imeiPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data;name=\"imei\""));
    imeiPart.setBody("A100005CE05889");
    requset.setRawHeader(QByteArray("X-Requester"), QByteArray("gs"));
    requset.setRawHeader(QByteArray("X-Authorization"),QByteArray("Bearer " + Access_Token.toUtf8()));
    multiPart->append(imeiPart);
    multiPart->append(reqPart);

    requset.setUrl(QUrl(agv_sys_config.Get_Qx_Url));
    reply = networkManager.post(requset, multiPart);
    connect(reply, &QNetworkReply::finished, this, &HTTP_Proc::on_finished);
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &HTTP_Proc::http_replyError);
    multiPart->setParent(reply);
}

QString HTTP_Proc::gps_positionConvert(double num)
{
    QString num_1 = QString::number(num, 'f', 7);
    int idx = num_1.indexOf(".");
    QString num_2 = num_1.mid(idx + 1);
    double num_3 = num_2.toDouble() * 100.0 / 60.0;
    QString num_4 = QString::number((int) num_3);
    QString num_5 = QString::number((int) num);
    return QString(num_5 + "." + num_4);
}

Qx_Account_t HTTP_Proc::get_qxAccount()
{
    return qx_account;
}

void HTTP_Proc::writeGpsInfo(Gps_Info_t info)
{
    gps_info_buf = info;
}
