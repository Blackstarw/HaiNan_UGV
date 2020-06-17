#include "canbus_proc.h"
#include <QDebug>
#include <string.h>
#include <QVariant>
#include <QThread>
Canbus_Proc::Canbus_Proc(QObject *parent) : QObject(parent)
{

}

void Canbus_Proc::connectDevice()
{
    QString errorString;
    m_canDevice = QCanBus::instance()->createDevice(QStringLiteral("socketcan"), QStringLiteral("can0"), &errorString);
    if (!m_canDevice)
    {
        qDebug() << QString("Error creating device 'socketcan', reason: " + errorString);
        return;
    }
    connect(m_canDevice, &QCanBusDevice::errorOccurred, this, &Canbus_Proc::processErrors);
    connect(m_canDevice, &QCanBusDevice::framesReceived, this, &Canbus_Proc::processReceivedFrames);
    connect(m_canDevice, &QCanBusDevice::framesWritten, this, &Canbus_Proc::processFramesWritten);

    if (!m_canDevice->connectDevice())
    {
        qDebug() << QString("Socket can connection error: " + errorString);
        delete m_canDevice;
        m_canDevice = nullptr;
    } else {
        qDebug() << QString("Socket can connect success ");

        agv_device_status.Agv_Start = true;
        //emit can_deviceStatus(agv_device_status);
    }


}
void Canbus_Proc::processReceivedFrames()
{
    if (!m_canDevice)
        return;

    while (m_canDevice->framesAvailable()) {
        const QCanBusFrame frame = m_canDevice->readFrame();
        if (frame.frameType() == QCanBusFrame::ErrorFrame)
        {
            qDebug() << m_canDevice->interpretErrorFrame(frame);
        }else
        {
            //qDebug() << frame.toString();
            switch (frame.frameId()) {
            case Camera_Operation_ID:
                if(frame.payload().data()[0] == Camera_Take_Photo)
                {
                    emit cam_takePhoto(Camera_HD);
                    qDebug() << "can thread id" << QThread::currentThread();
                    qDebug("Camera_HD Camera_Take_Photo");
                }
                if(frame.payload().data()[1] == Camera_Take_Photo)
                {
                    emit cam_takePhoto(Camera_Infrared);
                    qDebug("Camera_Infrared Camera_Take_Photo");
                }
                break;

            case STM_DATA_SUMMARY_ID :
                //qDebug() << frame.payload().toHex();
                can_msg.can_msg_buf.clear();
                memcpy(&can_msg.can_msg_size, frame.payload().data(), 4);
                memcpy(&can_msg.can_msg_crc, frame.payload().data() + 4, 2);
                can_msg.can_msg_buf.clear();
                //qDebug() << "can_msg_size" <<can_msg.can_msg_size;
                //qDebug() << "can_msg_crc" << can_msg.can_msg_crc;
                break;
            case STM_DATA_TX_ID :
                can_msg.can_msg_buf.append(frame.payload());
                break;
            case STM_DATA_END_ID :
                if((uint32_t)can_msg.can_msg_buf.size() == can_msg.can_msg_size)
                {
                    if(can_msg.can_msg_crc == crc_16_CCITT_False((uint8_t*)can_msg.can_msg_buf.data(), 0, 0xffff, can_msg.can_msg_size))
                    {
                        //qDebug() << "crc : OK";
                        emit gps_msgReceived(can_msg.can_msg_buf.data(), can_msg.can_msg_size);
                    }
                }
                break;
            default:
                break;
            }
        }
//        const QString time = QString::fromLatin1("%1.%2  ")
//                .arg(frame.timeStamp().seconds(), 10, 10, QLatin1Char(' '))
//                .arg(frame.timeStamp().microSeconds() / 100, 4, 10, QLatin1Char('0'));
    }
}
void Canbus_Proc::sendFrame(const QCanBusFrame frame) const
{
    m_canDevice->writeFrame(frame);

}
void Canbus_Proc::processErrors(QCanBusDevice::CanBusError) const
{
    qDebug() << "Can bus error ";
}
void Canbus_Proc::disconnectDevice()
{

}
void Canbus_Proc::processFramesWritten(qint64)
{

}

uint16_t Canbus_Proc::crc_16_CCITT_False(uint8_t* pucFrame, uint32_t offset, uint16_t initial_value, uint16_t usLen)
{
    uint32_t start = offset;
    uint32_t end = offset + usLen;
    uint16_t crc = initial_value; // initial value 0xffff
    const uint16_t  polynomial = 0x1021; // poly value

    for (uint32_t index = start; index < end; index++)
    {
        uint8_t b = pucFrame[index];

        for (uint32_t i = 0; i < 8; i++)
        {
            uint8_t bit = ((b >> (7 - i) & 1) == 1);
            uint8_t c15 = ((crc >> 15 & 1) == 1);
            crc <<= 1;
            if (c15 ^ bit) crc ^= polynomial;
        }
    }
    crc &= 0xffff;
    return crc;
}

void Canbus_Proc::can_sendMsg(char* data, uint32_t data_len)
{
    //qDebug()<<"can_sendMsg" << QThread::currentThread();
    uint32_t frame_delay = 5;
    QCanBusFrame TxFrame;
    QByteArray TxFrame_Data;
    char TxData[8];
    uint16_t Data_CRC = crc_16_CCITT_False((uint8_t*)data, 0, 0xffff, data_len);
    memcpy(TxData, &data_len, 4);
    memcpy((TxData + 4), &Data_CRC, 2);
    TxFrame.setFrameId(DATA_SUMMARY_ID);
    TxFrame_Data.setRawData(TxData, 6);
    TxFrame.setPayload(TxFrame_Data);
    sendFrame(TxFrame);
    TxFrame_Data.clear();

    QThread::msleep(frame_delay);
    uint32_t count = data_len / 8;
    uint32_t mod = data_len % 8;
    uint32_t idx;

    for(idx = 0; idx < count; idx ++)
    {
        memcpy(TxData, data + idx * 8, 8);
        TxFrame.setFrameId(DATA_TX_ID);
        TxFrame_Data.setRawData(TxData, 8);
        TxFrame.setPayload(TxFrame_Data);
        sendFrame(TxFrame);
        TxFrame_Data.clear();
        QThread::msleep(frame_delay);
    }
    if(mod)
    {
        memcpy(TxData, data + idx * 8, mod);

        TxFrame.setFrameId(DATA_TX_ID);
        TxFrame_Data.setRawData(TxData, mod);
        TxFrame.setPayload(TxFrame_Data);
        sendFrame(TxFrame);
        TxFrame_Data.clear();
        QThread::msleep(frame_delay);
    }
    TxFrame.setFrameId(DATA_END_ID);
    TxFrame_Data.setRawData(TxData, 1);
    TxFrame.setPayload(TxFrame_Data);
    sendFrame(TxFrame);
    TxFrame_Data.clear();
    QThread::msleep(frame_delay);
}

