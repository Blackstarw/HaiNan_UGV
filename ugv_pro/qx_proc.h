#ifndef QX_PROC_H
#define QX_PROC_H

#include <QObject>
#include "agv_def.h"
#include "string.h"
#include "qxwz_types.h"
#include "qxwz_sdk.h"
#include <QThread>
#include "camera_proc.h"
#include "canbus_proc.h"
//#include "http_proc.h"
#include "qx_proc.h"



class Qx_Proc : public QObject
{
    Q_OBJECT
public:
    explicit Qx_Proc(QObject *parent = nullptr);
    int qx_getCoord(qxwz_sdk_get_coord_sys_callback_t cb);
    int qx_setCoord(qxwz_uint32_t coord_sys_index, qxwz_sdk_set_coord_sys_callback_t cb);

signals:
    void qx_nosrReady(char *data, uint32_t len);

public slots:

    int qx_sdkInit(qxwz_sdk_config_t *sdk_config);

    int qx_sdkAuth();

    int qx_sdkStart(qxwz_uint32_t cap_id);

    int qx_sdkUploadGGA(const qxwz_char_t *gga, qxwz_uint32_t len);
    static void qx_set_coord_cb(qxwz_int32_t status_code);
    static void qx_get_coord_cb(qxwz_int32_t status_code, qxwz_sdk_coord_sys_info_t *coord_sys_info);
    static void qx_on_status(int code);
    //static void qx_show_caps(qxwz_sdk_cap_info_t *cap_info);
    static void qx_on_auth(qxwz_int32_t status_code, qxwz_sdk_cap_info_t *cap_info);
    static void qx_on_start(qxwz_int32_t status_code, qxwz_uint32_t cap_id);
    static void qx_on_data(qxwz_sdk_data_type_e type, const qxwz_void_t *data, qxwz_uint32_t len);
    void qx_checkNosrData();

private:




};

#endif // QX_PROC_H
