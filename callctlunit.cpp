#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<QFileInfo>

#define DEV_PATH "/dev/input/event0"

#include "config.h"
#include "ztpmanager.h"
#include"callctlunit.h"
#include"globalinfo.h"
CallCtlUnit::CallCtlUnit(QObject *parent) :
    QThread(parent)
{
    flag = false;
    ioNr = 14;
}
void CallCtlUnit::run()
{
   char strBuf[50];
   sprintf(strBuf,"/sys/class/gpio/gpio%d",ioNr);
    if(!QFile::exists(strBuf))
    {
        sprintf(strBuf,"echo %d > /sys/class/gpio/export",ioNr);
        if(system(strBuf) == -1)
        {
            qDebug()<<"system err";
        }
        sprintf(strBuf,"echo out > /sys/class/gpio/gpio%d/direction",ioNr);
        if(-1 == system(strBuf))
        {
            qDebug()<<"system err";
        }
        sprintf(strBuf,"echo 0 > /sys/class/gpio/gpio%d/value",ioNr);
        if(-1 == system(strBuf))
        {
            qDebug()<<"system err";
        }
    }
    int keys_fd;
    struct input_event t;
    keys_fd=open(DEV_PATH, O_RDONLY);
    if(keys_fd <= 0)
       {
           qDebug("open /dev/input/event0 device error!\n");
           return;
       }
       while(1)
       {
           if(flag == false)//当前无呼叫状态
           {
               if(read(keys_fd, &t, sizeof(t)) == sizeof(t))
               {
                   if(t.type==EV_KEY &&  t.code == KEY_F)//呼叫按钮
                       if(t.value==1)
                       {
                           flag = true;//变为呼叫状态
                           sprintf(strBuf,"echo 1 > /sys/class/gpio/gpio%d/value",ioNr);//呼叫灯亮
                           if(-1 == system(strBuf))
                           {
                               qDebug()<<"system() err";
                           }
                           qDebug("sendRoomCall(true);");
                           sendRoomCall(flag);//发送包间呼叫消息。
                           sleep(3);
                       }
               }
           }
           else//当前呼叫状态
           {
               if(read(keys_fd, &t, sizeof(t)) == sizeof(t))
               {
                   if(t.type==EV_KEY &&  t.code == KEY_G)//停止呼叫按钮
                       if(t.value==1)
                       {

                               flag = false;//变为无呼叫状态
                                sprintf(strBuf,"echo 0 > /sys/class/gpio/gpio%d/value",ioNr);//熄灭呼叫灯
                               if( -1 == system(strBuf))
                               {
                                   qDebug()<<"system() err";
                               }
                               qDebug("sendRoomCall(false);");
                               sendRoomCall(flag);//发送包间呼叫停止消息。
                       }
               }

           }
       }
}
void CallCtlUnit::sendRoomCall(bool _flag)
{
    if(GlobalInfo::getInstance()->callUnitIP == "")
    {
        qDebug("has not avalid call unit ip");
        return;
    }
    ZTPManager ztpm;
    ZTPprotocol ztp;
    ztp.addPara("T","Call");
    if(_flag)
        ztp.addPara("Value","On");
    else
        ztp.addPara("Value","Off");
    ztp.addPara("RoomNo",QString::number(GlobalInfo::getInstance()->roomNr ));
    ztp.addPara("DeviceNo",QString::number(GlobalInfo::getInstance()->deviceNr ));
    ztpm.SendOneZtp(ztp,QHostAddress(GlobalInfo::getInstance()->callUnitIP),7789);
//    int tryCnt = 3;
//    ztp.clear();
//    ZTPManager::ResultState ztpRes;
//    while(tryCnt--)
//    {
//        ztpRes = ztpm.waitOneZtp(ztp,300);
//        if(ztpRes == ZTPManager::SUCCESS)
//            return;
//    }
    //qDebug()<<"SendOneZtp Call State Res"<<res;
}
