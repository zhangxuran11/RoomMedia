#include "checkversionthread.h"
#include <QFile>
#include <stdio.h>
#include "globalinfo.h"
 #include <QCryptographicHash>
#include <time.h>
#include <unistd.h>
#include<QDebug>
CheckVersionThread::CheckVersionThread(QObject *parent) :
    QThread(parent)
{
    f_mapIsValid = true;
    f_dbIsValid = true;
    qsrand(time(NULL));
}
void CheckVersionThread::run()
{
    updateTrainLineDbMD5();
    updateGPSMapMD5();
    if(f_mapIsValid == true)
    {
//        emit updateMap();
    }
    ztpManager = new ZTPManager(5221,QHostAddress("224.102.228.40"));
    connect(ztpManager,SIGNAL(readyRead()),this,SLOT(procVersion()),Qt::DirectConnection);
    gpsMapZtpm = new ZTPManager;
    trainLineZtpm = new ZTPManager;
    connect(gpsMapZtpm,SIGNAL(readyRead()),this,SLOT(OnDownComplete()),Qt::DirectConnection);
    connect(trainLineZtpm,SIGNAL(readyRead()),this,SLOT(OnDownComplete()),Qt::DirectConnection);
    exec();
}
void CheckVersionThread::OnDownComplete()
{
    QObject* obj = sender();
    ZTPManager* ztpm = dynamic_cast<ZTPManager*>(obj);
    if(ztpm == NULL)
        return;
    ZTPprotocol ztp;
    ztpm->getOneZtp(ztp);
    ztp.print();
    if(ztp.getPara("T") == "DOWN_COMPLETE")
    {
        if(ztpm == gpsMapZtpm)
        {
            updateGPSMapMD5();
        }
        else if(ztpm == trainLineZtpm)
        {
            updateTrainLineDbMD5();
        }
    }
}
void CheckVersionThread::procVersion()
{
    ZTPprotocol ztp;
    ztpManager->getOneZtp(ztp);
    if(ztp.getPara("T") == "MAP_DB_MD5")
    {
        ztp.print();
        if(!f_mapIsValid)
            updateGPSMapMD5();
        if(!f_dbIsValid)
            updateTrainLineDbMD5();
        if(ztp.getPara("GPSMAP_MD5") != curGPSMapMD5)
        {
            ZTPprotocol ztp;
            ztp.addPara("T","Down");
            ztp.addPara("Resource","GPSMap.png");
            gpsMapZtpm->SendOneZtp(ztp,QHostAddress("224.102.228.40"),8316);
            f_mapIsValid = false;
        }
        if(ztp.getPara("TrainLineDb_MD5") != curTrainLineDbMD5)
        {
            ZTPprotocol ztp;
            ztp.addPara("T","Down");
            ztp.addPara("Resource","TrainLine.db");
            trainLineZtpm->SendOneZtp(ztp,QHostAddress("224.102.228.40"),8316);
            f_dbIsValid = false;
        }
        if(ztp.getPara("TrainLineDb_MD5") == curTrainLineDbMD5  &&f_mapIsValid&& !f_dbIsValid )
        {
            f_dbIsValid = true;
            emit updateMap();
        }
        if(ztp.getPara("GPSMAP_MD5") == curGPSMapMD5  && !f_mapIsValid )
        {
            f_mapIsValid = true;
            emit updateMap();
        }
    }

}
void CheckVersionThread::updateGPSMapMD5()
{
    char buf[40];
    memset(buf,0,40);
    QFile f("/srv/tftp/GPSMap.png");
    bool res = f.open(QFile::ReadOnly);
    QByteArray _curGPSMapMD5;
    if(res)
    {
        _curGPSMapMD5 = QCryptographicHash::hash ( f.readAll(), QCryptographicHash::Md5);
        for(int i = 0;i < _curGPSMapMD5.length();i++)
        {
            sprintf(buf+2*i,"%02x",_curGPSMapMD5.data()[i]);
        }
        curGPSMapMD5 = QString(buf);
        f.close();
    }
    else
    {
        qDebug("open /srv/tftp/GPSMap.png failed !");
        f_mapIsValid = false;
    }
}

void CheckVersionThread::updateTrainLineDbMD5()
{
    char buf[40];
    memset(buf,0,40);
    QFile f("/srv/tftp/TrainLine.db");
    bool res = f.open(QFile::ReadOnly);
    QByteArray _curTrainLineDbMD5;
    if(res)
    {
        _curTrainLineDbMD5 = QCryptographicHash::hash ( f.readAll(), QCryptographicHash::Md5);
        for(int i = 0;i < _curTrainLineDbMD5.length();i++)
        {
            sprintf(buf+2*i,"%02x",_curTrainLineDbMD5.data()[i]);
        }
        curTrainLineDbMD5 = QString(buf);
        f.close();
    }
    else
    {
        qDebug("open /srv/tftp/TrainLine.db failed !");
        f_dbIsValid = false;
    }
}
