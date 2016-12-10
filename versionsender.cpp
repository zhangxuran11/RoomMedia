#include "versionsender.h"
#include "ztpmanager.h"
VersionSender::VersionSender(QString devName,int majorVersion,int minorVersion,int microVersion,int _carNo) :
    DevName(devName),MajorVersion(majorVersion),MinorVersion(minorVersion),MicroVersion(microVersion)
{
    testOffline = false;
    online =true;
    carNo = _carNo;
    timer.setSingleShot(false);
    timer.setInterval(1000);
    timer.start();
    connect(&timer,SIGNAL(timeout()),this,SLOT(OnSendVersion()));
}
void VersionSender::OnSendVersion()
{
    static ZTPManager* ztpm = new ZTPManager;
    ZTPprotocol ztp;
    ztp.addPara("T","VERSION");
    if(testOffline)
        ztp.addPara("STATE","OFFLINE");
    else if(online)
        ztp.addPara("STATE","ONLINE");
    else
        ztp.addPara("STATE","OFFLINE");
    ztp.addPara("CARNO",QString::number(carNo));
    ztp.addPara("DEVICE",DevName);
    ztp.addPara("MajorVersion",QString::number(MajorVersion));
    ztp.addPara("MinorVersion",QString::number(MinorVersion));
    ztp.addPara("MicroVersion",QString::number(MicroVersion));
//    ztp.print();
    ztpm->SendOneZtp(ztp,QHostAddress("224.102.228.40"),8318);
}
