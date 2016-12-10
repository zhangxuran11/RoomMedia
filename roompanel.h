#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include "qextserialport/src/qextserialport.h"
#include"softwatchdog.h"
#include<QObject>
class QTimer;
class RoomPanel : public QObject
{
    Q_OBJECT

        QextSerialPort *temp;//温度
        QextSerialPort *cntl;//控制面板
        SoftWatchdog* tempSoftDog;
        SoftWatchdog* cntlSoftDog;
public:
    RoomPanel();
private slots:
    //接收温度
    void on_readyRead_temp();
    //接收控制面板按键
    void on_readyRead_cntl();
};

#endif // SERIALTHREAD_H
