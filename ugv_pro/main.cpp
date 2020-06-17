#include <QCoreApplication>
#include "agv_ctr.h"
#include <QFile>
#include <QString>
#include <QMutex>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>
#include "agv_def.h"

Work_Dir_t Work_Dir;

void log_outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();
    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Info:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
    case QtInfoMsg:
        text = QString("Infomation:");
        break;
    }
    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");    
    QString message = QString("%1 %2 %3 %4").arg(text).arg(current_date_time).arg(context_info).arg(msg);
    QString current_date;
    if(Work_Dir.Time_is_Valid)
    {
        current_date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    }else
    {
        current_date = "000";
    }
    QFile file(QString("%1%2.log").arg(Work_Dir.Dir).arg(current_date));
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    mutex.unlock();
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "AGV Start.";
    if(argc != 2)
    {
        qDebug() << "Input param error! ";
        return -1;
    }
    QDateTime DateTime;
    QString CurrYear = DateTime.currentDateTime().toString("yyyy");
    if(CurrYear.toInt() >= 2020)
    {
       QString CurrDate = DateTime.currentDateTime().toString("yyyy_MM_dd");
       Work_Dir.Dir = QString("/home/linaro/Documents/AGV_Works/%1/").arg(CurrDate);
       Work_Dir.Time_is_Valid = true;
    }else
    {
        Work_Dir.Time_is_Valid = false;
        Work_Dir.Dir = QString("/home/linaro/Documents/AGV_Works/agvTempWorkFolder/");
    }
    QDir dir;
    if(!dir.exists(Work_Dir.Dir))
    {
        dir.mkpath(Work_Dir.Dir);
    }

    if(1 == atoi(argv[1]))
    {
        qInstallMessageHandler(log_outputMessage);
    }else
    {
        qDebug() << "Debug Model.";
    }
    AGV_Ctr agv;
    agv.agv_sysInit();
    return a.exec();
}
