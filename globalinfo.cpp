#include "zutility.h"
#include "globalinfo.h"
#include"ztools.h"
#include<QSettings>
#include<QStringList>

GlobalInfo* GlobalInfo::_instance = NULL;
GlobalInfo::GlobalInfo(QObject *parent) :
    QObject(parent)
{
    train_id = -1;
    VServerIP = "";
    callUnitIP = "";
    sysCtlIp = "";
    bedNr = ZUtility::getBedNr();
    roomNr = ZUtility::getRoomNr();
    deviceNr = ZUtility::getDeviceNr();
    carId = ZTools::getCarID();

    chStateMap.insert("Ch"+QString::number(0),"play");
    for( int i = 1 ;i < 9;i++) {
        chStateMap.insert("Ch"+QString::number(i),"stop");
    }

    g_info.addPara("StartStation","--");
    g_info.addPara("EndStation","--");
    g_info.addPara("PreStation","--");
    g_info.addPara("NextStation","--");
    g_info.addPara("CurrentStationEn","--");
    g_info.addPara("StartStation_th","--");
    g_info.addPara("EndStation_th","--");
    g_info.addPara("PreStation_th","--");
    g_info.addPara("NextStation_th","--");
    g_info.addPara("CurrentStationTh","--");

}
GlobalInfo* GlobalInfo::getInstance()
{
    if(_instance == NULL)
        _instance = new GlobalInfo();
    return _instance;
}
