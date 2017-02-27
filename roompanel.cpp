#include "roompanel.h"
#include <CQplayerGUI.h>
#include<iostream>
#include<QString>
#include<stdio.h>
#include<QTimer>
#include"globalinfo.h"
#include"config.h"
#include"CQplayerGUI.h"
#include<QDebug>
#include"ztpmanager.h"
#include"ztools.h"
using namespace std;
static void QextSerialPort_init(QextSerialPort *port,int _io)
{
    port->setDtr();
    port->setBaudRate(BAUD19200);
    port->setFlowControl(FLOW_OFF);
    port->setParity(PAR_NONE);
    port->setDataBits(DATA_8);
    port->setStopBits(STOP_1);
    port->setTimeout(300);

    QString io = QString::number(_io);
    if(!QFile::exists("/sys/class/gpio/gpio"+io))
    {
        QString cmd = "echo "+io+" > /sys/class/gpio/export";
        if(system(cmd.toAscii().data()))
        {
            qDebug()<<"create /sys/class/gpio/gpio" + io+"failed!!";
        }
    }
    if(QFile::exists("/sys/class/gpio/gpio"+io))
    {
        //QFile f("/sys/class/gpio/gpio"+io+"/value");
        QFile f("/sys/class/gpio/gpio"+io+"/direction");
        //f.setFileName("/sys/class/gpio/gpio"+io+"/direction");
        f.open(QFile::ReadWrite);
        f.write("out");
        f.close();

        f.setFileName("/sys/class/gpio/gpio"+io+"/value");
        if(!f.open(QFile::ReadWrite))
            qDebug("open file value failed!!");
        f.write("0");
        f.close();
    }
    else
        qDebug()<<"not exits gpio"+io;



}
static void cntlDeal();
static void tempDeal();
RoomPanel::RoomPanel()
{
    temp = new QextSerialPort("/dev/ttyUSB0",QextSerialPort::EventDriven);
    cntl = new QextSerialPort("/dev/ttyUSB1",QextSerialPort::EventDriven);
    QextSerialPort_init(temp,15);
    QextSerialPort_init(cntl,13);

    tempSoftDog = new SoftWatchdog(temp,tempDeal);
    cntlSoftDog = new SoftWatchdog(cntl,cntlDeal);
    if (temp->open(QIODevice::ReadWrite) == true) {
        cout<<"temp listening for data on "<<temp->portName().toStdString()<<endl;
    }
    else {
        cout<<"temp device failed to open: "<<temp->errorString().toStdString() <<endl;
        return;
    }

    if (cntl->open(QIODevice::ReadWrite) == true) {
        cout<<"cntl listening for data on "<<cntl->portName().toStdString()<<endl;
    }
    else {
        cout<<"cntl device failed to open: "<<cntl->errorString().toStdString() <<endl;
        return;
    }

    connect(temp,SIGNAL(readyRead()),this,SLOT(on_readyRead_temp()),Qt::DirectConnection);
    connect(cntl,SIGNAL(readyRead()),this,SLOT(on_readyRead_cntl()),Qt::DirectConnection);
}
void tempDeal()
{
    if(GlobalInfo::getInstance()->deviceNr == 0) //次设备
        return;
    if( GlobalInfo::getInstance()->VServerIP != "")
    {
        ZTPManager* ztpm = new ZTPManager;
        int _tryCnt = 3;
        while(_tryCnt--)
        {
            ZTPprotocol ztp;
            ztp.addPara("T","Error");
            ztp.addPara("Code","100");
            ztpm->SendOneZtp(ztp,QHostAddress(GlobalInfo::getInstance()->VServerIP),8881);
            ZTPManager::ResultState res =  ztpm->waitOneZtp(ztp);
            if(res == ZTPManager::SUCCESS)
            {
                qDebug("Send Error 100 success!");
                break;
            }
        }
        delete ztpm;
    }
    qDebug("cleanUp with tempDeal");
}
void cntlDeal()
{
    if(GlobalInfo::getInstance()->VServerIP != "")
    {
        ZTPManager* ztpm = new ZTPManager;
        int _tryCnt = 3;
        while(_tryCnt--)
        {
            ZTPprotocol ztp;
            ztp.addPara("T","Error");
            ztp.addPara("Code","101");
            ztpm->SendOneZtp(ztp,QHostAddress(GlobalInfo::getInstance()->VServerIP),8881);
            ZTPManager::ResultState res =  ztpm->waitOneZtp(ztp);
            if(res == ZTPManager::SUCCESS)
            {
                qDebug("Send Error 101 success!");
                break;
            }
        }
        delete ztpm;
    }
    qDebug("cleanUp with cntlDeal");
}
static bool flag3 = false;
static void temp_fun3()
{
    flag3 = false;
}
//接收控制面板按键
void RoomPanel::on_readyRead_cntl()
{
    cntlSoftDog->feed();
    QByteArray bytes = cntl->readAll();
//    for(int i = 0;i<bytes.length();i++)
//        qDebug("%2x ",(quint8)bytes[i]);
    quint8* _bytes = (quint8*)bytes.data();
    int i = 0;
    while(i<bytes.length() && (_bytes[i++] != 0xaa || _bytes[i++] != 0x55 ));
    if(i >= bytes.length())
    {
        qDebug("data invalid \n");
        return;
    }
    int start = i-2;

    if(_bytes[start+2] == 0x01)//按键
    {

        if( ((_bytes[start]+_bytes[start+1]+_bytes[start+2]+_bytes[start+3])&0xff) == _bytes[start+4])
        {
            CQplayerGUI *player = GlobalInfo::getInstance()->playerGui;
            quint8 code = _bytes[start+3];
            if(code == 0x03)//音量+
            {
                 player->ui->sliderVlm->setSliderPosition(player->ui->sliderVlm->sliderPosition() + 5);
            }
            else if(code == 0x04)//音量-
            {
                player->ui->sliderVlm->setSliderPosition(player->ui->sliderVlm->sliderPosition() - 5);
            }
            else if(code == 0x01)//频道+
            {
                if(flag3)
                    return;
                flag3 = true;
                ZTools::singleShot(4000,temp_fun3);
                player->onStopButton_clicked();
                if(player->getCurrentChannel() >= 8)
                    player->play(0);
                else
                    player->play(player->getCurrentChannel() + 1);
            }
            else if(code == 0x02)//频道-
            {
                if(flag3)
                    return;
                flag3 = true;
                ZTools::singleShot(4000,temp_fun3);
                player->onStopButton_clicked();
                if(player->getCurrentChannel() == 0)
                    player->play(8);
                else
                    player->play(player->getCurrentChannel() - 1);
            }
            else if(code == 0x05)//开关机
            {
                if(player->getScreenToggle() == CQplayerGUI::ON)
                    player->screenSwitch(CQplayerGUI::OFF);
                else
                    player->screenSwitch(CQplayerGUI::ON);
            }

        }
        else
        {
            qDebug("check sum error \n");
        }
    }
    else if(_bytes[start+2] == 0x02)//心跳
    {
        if( ((_bytes[start]+_bytes[start+1]+_bytes[start+2])&0xff) == _bytes[start+3])
        {
            //
            //qDebug("temp in the door:%f\n\n",gGlobal.temp_in_door);

        }
        else
        {
            qDebug("check sum error \n");
        }
    }

}

//接收温度
void RoomPanel::on_readyRead_temp()
{
    //qDebug("recv data on ttyUSB1...\n");
    tempSoftDog->feed();
    QByteArray bytes = temp->readAll();
    //qDebug()<<bytes;
    quint8* _bytes = (quint8*)bytes.data();
    int i = 0;
    while(i<bytes.length() && (_bytes[i++] != 0xaa || _bytes[i++] != 0x55 || _bytes[i++] != 0x00));
    if(i >= bytes.length())
    {
        qDebug("data invalid \n");
        return;
    }
    int start = i-3;
    if( ((_bytes[start]+_bytes[start+1]+_bytes[start+2]+_bytes[start+3]+_bytes[start+4])&0xff) == _bytes[start+5])
    {
        //static double temp_sample = 0;
        //static int cnt = 0;
        //cnt++;
        qint16 tmp;
        quint8* addr_tmp = (quint8*)&tmp;
        *addr_tmp = _bytes[start+4];
        *(addr_tmp+1) = _bytes[start+3];
        GlobalInfo *glb = GlobalInfo::getInstance();
        double temp_in_door = tmp*0.0625;
        //temp_sample += temp_in_door;
        //if(cnt == 5)
        //{
            ZTPprotocol ztp;
            ztp.addPara("T","Temperature");
            ztp.addPara("CAR_ID",QString::number(ZTools::getCarID()));
            ztp.addPara("ROOM_NR",QString::number(GlobalInfo::getInstance()->roomNr));
            ztp.addPara("value",QString::number(temp_in_door));
            glb->comZtpManager->SendOneZtp(ztp,QHostAddress(BROADCAST_IP),8314);
        //    cnt = 0;
        //    temp_sample = 0;
       // }


    }
    else
    {
        qDebug("check sum error \n");
    }

}
