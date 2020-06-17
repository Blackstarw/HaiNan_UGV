#include "camera_proc.h"
Camera_Proc::Camera_Proc(QObject *parent) : QObject(parent)
{
    CurrDate = DateTime.currentDateTime().toString("yyyy_MM_dd");
    Camera_Photo_num[Camera_Infrared] = 0;
    Camera_Photo_num[Camera_HD] = 0;

}

void Camera_Proc::cam_takePhoto(Camera_Type_e cam_type)
{
    Camera_Read_Json_File();
    switch (cam_type) {
    case Camera_HD:
        qDebug()<<"Camera HD Take Photo.";
        Camera_HD_Take_Photo();
        break;
    case Camera_Infrared:
        qDebug()<<"Camera Infrared Take Photo.";
        Camera_Infrared_Take_Photo();
        break;
    default:
        break;
    }
}

void Camera_Proc::Camera_Write_Json_File(uint16_t *photo_num)
{
    QJsonDocument configJson;
    QJsonObject configObj;
    configObj.insert("HD_Photo_Num", QJsonValue(photo_num[Camera_HD]));
    configObj.insert("Infrared_Photo_Num", QJsonValue(photo_num[Camera_Infrared]));
    configJson.setObject(configObj);
    QByteArray configData= configJson.toJson();
    QFile cam_config(Work_Dir.Dir + "cam_config");
    cam_config.open(QIODevice::WriteOnly);
    cam_config.write(configData);
    cam_config.close();

}

void Camera_Proc::Camera_Read_Json_File()
{
    QFile cam_config(Work_Dir.Dir + "cam_config");
    cam_config.open(QIODevice::ReadOnly);
    QByteArray configData = cam_config.readAll();
    cam_config.close();
    QJsonDocument configJson;
    QJsonObject configObj;
    if(configData.isEmpty())
    {
        Camera_Write_Json_File(Camera_Photo_num);
    }else
    {
        configJson = QJsonDocument::fromJson(configData);
        configObj = configJson.object();

        Camera_Photo_num[Camera_HD] = (uint16_t)configObj.value("HD_Photo_Num").toInt();
        Camera_Photo_num[Camera_Infrared] = (uint16_t)configObj.value("Infrared_Photo_Num").toInt();
    }

}
void Camera_Proc::Camera_HD_Take_Photo()
{
    cam_hd_capture = cv::VideoCapture("rtsp://admin:jimu123456@192.168.100.110:554/Streaming/Channels/101?transportmode=unicast");
    if(cam_hd_capture.isOpened())
    {
        sys_fault.camera_fault[0] &= (~Camera_comm_fault);
        agv_device_status.Camera_HD_Start = true;
        QImage cam_hd_image;
        cv::Mat cam_hd_frame, cam_hd_rgbImage;
        if(cam_hd_capture.grab())
        {
            sys_fault.camera_fault[0] &= (~Camera_image_fault);
            Camera_Photo_num[Camera_HD] ++;
            Camera_Write_Json_File(Camera_Photo_num);
            cam_hd_capture >> cam_hd_frame;
            cv::cvtColor(cam_hd_frame, cam_hd_rgbImage, CV_BGR2RGB);
            cam_hd_image = QImage((const unsigned char*)(cam_hd_rgbImage.data),cam_hd_rgbImage.cols,cam_hd_rgbImage.rows,cam_hd_rgbImage.step,QImage::Format_RGB888);
            imgName[0] = QString(Work_Dir.Dir + QString("IMG_HD_%1.jpg").arg(Camera_Photo_num[Camera_HD]));
            if(cam_hd_image.save(imgName[0], "JPG", -1))
            {
                qDebug()<<"Camera HD Save Photo complete.";
                emit cam_photoSaveComplete(Camera_HD, imgName, 1);
            }
        }else
        {
            qWarning() << "Camera HD grab failed.";
            sys_fault.camera_fault[0] |= Camera_image_fault;
        }
    }else
    {
        qWarning()<<"Camera HD open failed.";
        sys_fault.camera_fault[0] |= Camera_comm_fault;
        agv_device_status.Camera_HD_Start = false;
    }
}
void Camera_Proc::Camera_Infrared_Take_Photo()
{
    cam_infrared_capture_ch_norm = cv::VideoCapture("rtsp://admin:jimu123456@192.168.100.111:554/Streaming/Channels/101?transportmode=unicast");
    cam_infrared_capture_ch_infr = cv::VideoCapture("rtsp://admin:jimu123456@192.168.100.111:554/Streaming/Channels/201?transportmode=unicast");
    if(cam_infrared_capture_ch_norm.isOpened() && cam_infrared_capture_ch_infr.isOpened())
    {
        sys_fault.camera_fault[1] &= (~Camera_comm_fault);
        agv_device_status.Camera_Infrared_Start = true;
        QImage cam_infrared_ch_norm_qimage, cam_infrared_ch_infr_qimage;
        cv::Mat cam_infrared_ch_norm_frame, cam_infrared_ch_norm_rgbImage, cam_infrared_ch_infr_frame, cam_infrared_ch_infr_rgbImage;
        if(cam_infrared_capture_ch_norm.grab() && cam_infrared_capture_ch_infr.grab())
        {
            sys_fault.camera_fault[1] &= (~Camera_image_fault);
            Camera_Photo_num[Camera_Infrared] ++;
            Camera_Write_Json_File(Camera_Photo_num);

            cam_infrared_capture_ch_norm >> cam_infrared_ch_norm_frame;
            cam_infrared_capture_ch_infr >> cam_infrared_ch_infr_frame;

            cv::cvtColor(cam_infrared_ch_norm_frame, cam_infrared_ch_norm_rgbImage, CV_BGR2RGB);
            cv::cvtColor(cam_infrared_ch_infr_frame, cam_infrared_ch_infr_rgbImage, CV_BGR2RGB);

            cam_infrared_ch_norm_qimage = QImage((const unsigned char*)(cam_infrared_ch_norm_rgbImage.data),cam_infrared_ch_norm_rgbImage.cols,cam_infrared_ch_norm_rgbImage.rows,cam_infrared_ch_norm_rgbImage.step,QImage::Format_RGB888);
            cam_infrared_ch_infr_qimage = QImage((const unsigned char*)(cam_infrared_ch_infr_rgbImage.data),cam_infrared_ch_infr_rgbImage.cols,cam_infrared_ch_infr_rgbImage.rows,cam_infrared_ch_infr_rgbImage.step,QImage::Format_RGB888);

            QString imgName_ch_norm = QString(Work_Dir.Dir + QString("IMG_INFR_NORM_%1.jpg").arg(Camera_Photo_num[Camera_Infrared]));
            QString imgName_ch_infr = QString(Work_Dir.Dir + QString("IMG_INFR_INFR_%1.jpg").arg(Camera_Photo_num[Camera_Infrared]));
            //QString imgName[2];
            imgName[0] = imgName_ch_norm;
            imgName[1] = imgName_ch_infr;
            if(cam_infrared_ch_norm_qimage.save(imgName_ch_norm, "JPG", -1) && cam_infrared_ch_infr_qimage.save(imgName_ch_infr, "JPG", -1))
            {
                emit cam_photoSaveComplete(Camera_Infrared, imgName, 2);
            }
        }else
        {
            qWarning() << "Camera infrared grab failed.";
            sys_fault.camera_fault[1] |= Camera_image_fault;
        }
    }else
    {
        qWarning()<<"Camera infrared open failed.";
        sys_fault.camera_fault[1] |= Camera_comm_fault;
        agv_device_status.Camera_Infrared_Start = false;
    }
}


void Camera_Proc::cam_connect()
{

    cam_hd_capture = cv::VideoCapture("rtsp://admin:jimu123456@192.168.100.110:554/Streaming/Channels/101?transportmode=unicast");
    cam_infrared_capture_ch_norm = cv::VideoCapture("rtsp://admin:jimu123456@192.168.100.111:554/Streaming/Channels/101?transportmode=unicast");
    cam_infrared_capture_ch_infr = cv::VideoCapture("rtsp://admin:jimu123456@192.168.100.111:554/Streaming/Channels/201?transportmode=unicast");
    if(cam_hd_capture.isOpened())
    {
        sys_fault.camera_fault[0] &= (~Camera_comm_fault);
        sys_fault.camera_fault[0] &= (~Camera_failed);

        agv_device_status.Camera_HD_Start = true;
        qDebug()<<"Camera HD open success.";
    }else {
        sys_fault.camera_fault[0] |= Camera_comm_fault;
        sys_fault.camera_fault[0] |= Camera_failed;
        agv_device_status.Camera_HD_Start = false;
        qDebug()<<"Camera HD open failed.";
    }
    if(cam_infrared_capture_ch_norm.isOpened() && cam_infrared_capture_ch_infr.isOpened())
    {
        sys_fault.camera_fault[1] &= (~Camera_comm_fault);
        sys_fault.camera_fault[1] &= (~Camera_failed);
        agv_device_status.Camera_Infrared_Start = true;
        qDebug()<<"Camera infrared open success.";
    }else
    {
        sys_fault.camera_fault[1] |= Camera_comm_fault;
        sys_fault.camera_fault[1] |= Camera_failed;
        agv_device_status.Camera_Infrared_Start = false;
        qDebug()<<"Camera infrared open failed.";
    }
}

QString Camera_Proc::get_workDir()
{
    return Work_Dir.Dir;
}
uint16_t Camera_Proc::get_cameraPhotoNum(Camera_Type_e cam_type)
{
    return Camera_Photo_num[cam_type];
}
