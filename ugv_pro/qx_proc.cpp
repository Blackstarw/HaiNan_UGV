#include "qx_proc.h"
#include <QDebug>
#include <QTime>

Nosr_Msg_t nosr_msg[5];
qxwz_uint32_t sdk_auth_flag;
qxwz_uint32_t sdk_start_flag;
qxwz_sdk_cap_info_t sdk_cap_info;

Qx_Proc::Qx_Proc(QObject *parent) : QObject(parent)
{

}

int Qx_Proc::qx_sdkInit(qxwz_sdk_config_t *sdk_config)
{
   return qxwz_sdk_init(sdk_config);
}

int Qx_Proc::qx_sdkAuth()
{
    return qxwz_sdk_auth();
}

int Qx_Proc::qx_sdkStart(qxwz_uint32_t cap_id)
{
    return qxwz_sdk_start(cap_id);
}

int Qx_Proc::qx_sdkUploadGGA(const qxwz_char_t *gga, qxwz_uint32_t len)
{
    return qxwz_sdk_upload_gga(gga, len);
}
int Qx_Proc::qx_getCoord(qxwz_sdk_get_coord_sys_callback_t cb)
{
    return qxwz_sdk_get_coord_sys(cb);
}

int Qx_Proc::qx_setCoord(qxwz_uint32_t coord_sys_index, qxwz_sdk_set_coord_sys_callback_t cb)
{
    return qxwz_sdk_set_coord_sys(coord_sys_index, cb);
}

void Qx_Proc::qx_set_coord_cb(qxwz_int32_t status_code)
{
    qDebug() << "qx_set_coord"  << status_code;
}


void Qx_Proc::qx_get_coord_cb(qxwz_int32_t status_code, qxwz_sdk_coord_sys_info_t *coord_sys_info)
{
    qDebug() << "get qx coord" << coord_sys_info->curr_coord_sys_index;
    qDebug() << "coord sys count" << coord_sys_info->coord_sys_count;
    qDebug() << status_code;
}

//void Qx_Proc::qx_show_caps(qxwz_sdk_cap_info_t *cap_info)
//{
//    qxwz_int32_t loop = 0;

//    qDebug("total capabilities: %d\n", cap_info->caps_num);
//    for (loop = 0; loop < cap_info->caps_num; ++loop) {
//        qDebug("idx: %d, cap_id: %u, state: %d, act_method: %d, expire_time: %llu\n",
//            loop + 1,
//            cap_info->caps[loop].cap_id,
//            cap_info->caps[loop].state,
//            cap_info->caps[loop].act_method,
//            cap_info->caps[loop].expire_time);
//    }
//}

void Qx_Proc::qx_on_auth(qxwz_int32_t status_code, qxwz_sdk_cap_info_t *cap_info)
{
    if (status_code == QXWZ_SDK_STAT_AUTH_SUCC) {
        sdk_auth_flag = 1;
        sdk_cap_info = *cap_info;
        //qx_show_caps(cap_info);
    } else {
        qDebug("auth failed, code=%d\n", status_code);
    }
}

void Qx_Proc::qx_on_start(qxwz_int32_t status_code, qxwz_uint32_t cap_id)
{

    qDebug() << "qx_on_start " << "status_code: " << status_code << "cap_id: " << cap_id;
    sdk_start_flag = 1;
}


void Qx_Proc::qx_on_status(int code)
{
    qDebug(" on status code: %d\n", code);
}



void Qx_Proc::qx_on_data(qxwz_sdk_data_type_e type, const qxwz_void_t *data, qxwz_uint32_t len)
{


    //qDebug(" on data: %d, ptr: %p, len: %d\n", type, data, len);
   // QTime Time;
   // QString CurrTime = Time.currentTime().toString("hh:mm:ss:zzz");
   // qDebug() << "CurrTime: " << CurrTime;

    static uint8_t idx = 0;

    switch (type) {
        case QXWZ_SDK_DATA_TYPE_RAW_NOSR:
            nosr_msg[idx].nosr_data = (char*)data;
            nosr_msg[idx].data_len = (uint32_t)len;
            nosr_msg[idx].new_msg = true;
            idx ++;
            if(idx > 4) idx = 0;

            //qDebug("QXWZ_SDK_DATA_TYPE_RAW_NOSR\n");
            break;
        default:
            qDebug("unknown type: %d\n", type);
    }
}

void Qx_Proc::qx_checkNosrData()
{

    static uint8_t idx = 0;

    while(true)
    {
        if(nosr_msg[idx].new_msg)
        {
            nosr_msg[idx].new_msg = false;
            qDebug() <<"got a nosr new msg." << idx << "data len:" << nosr_msg[idx].data_len;
            emit qx_nosrReady(nosr_msg[idx].nosr_data, nosr_msg[idx].data_len);
        }
        idx ++;
        if(idx > 4) idx = 0;
        QThread::msleep(60);
    }

}






