#ifndef CANBUS_PROC_H
#define CANBUS_PROC_H

#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QCanBusDeviceInfo>
#include <QString>
#include <QByteArray>
#include "agv_def.h"
#include <QThread>
class Canbus_Proc : public QObject
{
    Q_OBJECT
public:
    explicit Canbus_Proc(QObject *parent = nullptr);

signals:
    void cam_takePhoto(Camera_Type_e cam_type);
    void gps_infoReceived(Gps_Info_t gps_info);
    void gps_msgReceived(char* str, uint32_t str_len);
    //void can_deviceStatus(Agv_Device_Status_t agv_device_status);
public slots:
    void processReceivedFrames();
    void sendFrame(const QCanBusFrame frame) const;
    void processErrors(QCanBusDevice::CanBusError) const;
    void connectDevice();
    void disconnectDevice();
    void processFramesWritten(qint64);
    void can_sendMsg(char* msg_data, uint32_t msg_size);
    uint16_t crc_16_CCITT_False(uint8_t* pucFrame, uint32_t offset, uint16_t initial_value, uint16_t usLen);
private:
    QCanBusDevice *m_canDevice = nullptr;

    Can_Msg_t can_msg;

    Gps_Info_t gps_info;

    unsigned char data_buf[20][20];
};

#endif // CANBUS_PROC_H
