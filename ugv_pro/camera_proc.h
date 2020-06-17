#ifndef CAMERA_PROC_H
#define CAMERA_PROC_H

#include <QObject>
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

#include <QDebug>
#include <QImage>
#include "agv_def.h"
#include <QThread>
#include <QDateTime>
#include <QDir>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

class Camera_Proc : public QObject
{
    Q_OBJECT
public:
    explicit Camera_Proc(QObject *parent = nullptr);
    QString get_workDir();
    uint16_t get_cameraPhotoNum(Camera_Type_e cam_type);
    void cam_connect();
    void Camera_Read_Json_File();
    void Camera_Write_Json_File(uint16_t *photo_num);

signals:
    void cam_photoSaveComplete(Camera_Type_e cam_type, QString *PhotoName, int PhotoNum);
    //void cam_deviceStatus(Agv_Device_Status_t agv_device_status);
public slots:
    void cam_takePhoto(Camera_Type_e cam_type);
private:
    cv::VideoCapture cam_hd_capture, cam_infrared_capture_ch_norm, cam_infrared_capture_ch_infr;
    void Camera_HD_Take_Photo();
    void Camera_Infrared_Take_Photo();
    QString CurrDate;
    QDateTime DateTime;
    //QString Work_Dir;
    uint16_t Camera_Photo_num[Camera_Type_Max];
    QString imgName[2];
};

#endif // CAMERA_PROC_H
